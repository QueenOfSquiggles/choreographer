#include "../c11r_lang.hpp"

#include "core/core_string_names.h"
#include "core/os/os.h"
#include "core/project_settings.h"
#include "scene/main/node.h"
#include "modules/visual_script/visual_script_func_nodes.h"
#include "modules/visual_script/visual_script_nodes.h"



bool C11RScriptInstance::set(const StringName &p_name, const Variant &p_value) {
	Map<StringName, Variant>::Element *E = variables.find(p_name);
	if (!E) {
		return false;
	}

	E->get() = p_value;

	return true;
}

bool C11RScriptInstance::get(const StringName &p_name, Variant &r_ret) const {
	const Map<StringName, Variant>::Element *E = variables.find(p_name);
	if (!E) {
		return false;
	}

	r_ret = E->get();
	return true;
}
void C11RScriptInstance::get_property_list(List<PropertyInfo> *p_properties) const {
	for (const Map<StringName, C11RScript::Variable>::Element *E = script->variables.front(); E; E = E->next()) {
		if (!E->get()._export) {
			continue;
		}
		PropertyInfo p = E->get().info;
		p.name = String(E->key());
		p.usage |= PROPERTY_USAGE_SCRIPT_VARIABLE;
		p_properties->push_back(p);
	}
}
Variant::Type C11RScriptInstance::get_property_type(const StringName &p_name, bool *r_is_valid) const {
	const Map<StringName, C11RScript::Variable>::Element *E = script->variables.find(p_name);
	if (!E) {
		if (r_is_valid) {
			*r_is_valid = false;
		}
		ERR_FAIL_V(Variant::NIL);
	}

	if (r_is_valid) {
		*r_is_valid = true;
	}

	return E->get().info.type;
}

void C11RScriptInstance::get_method_list(List<MethodInfo> *p_list) const {
	for (const Map<StringName, C11RScript::Function>::Element *E = script->functions.front(); E; E = E->next()) {
		if (E->key() == script->get_default_func()) {
			continue;
		}

		MethodInfo mi;
		mi.name = E->key();
		if (E->get().function_id >= 0 && E->get().nodes.has(E->get().function_id)) {
			Ref<VisualScriptFunction> vsf = E->get().nodes[E->get().function_id].node;
			if (vsf.is_valid()) {
				for (int i = 0; i < vsf->get_argument_count(); i++) {
					PropertyInfo arg;
					arg.name = vsf->get_argument_name(i);
					arg.type = vsf->get_argument_type(i);

					mi.arguments.push_back(arg);
				}

				if (!vsf->is_sequenced()) { //assumed constant if not sequenced
					mi.flags |= METHOD_FLAG_CONST;
				}
			}
		}

		p_list->push_back(mi);
	}
}
bool C11RScriptInstance::has_method(const StringName &p_method) const {
	if (p_method == script->get_default_func()) {
		return false;
	}

	return script->functions.has(p_method);
}

//#define VSDEBUG(m_text) print_line(m_text)
#define VSDEBUG(m_text)

void C11RScriptInstance::_dependency_step(BlockInstance *node, int p_pass, int *pass_stack, const Variant **input_args, Variant **output_args, Variant *variant_stack, Variant::CallError &r_error, String &error_str, BlockInstance **r_error_node) {
	ERR_FAIL_COND(node->pass_idx == -1);

	if (pass_stack[node->pass_idx] == p_pass) {
		return;
	}

	pass_stack[node->pass_idx] = p_pass;

	if (!node->dependencies.empty()) {
		int dc = node->dependencies.size();
		BlockInstance **deps = node->dependencies.ptrw();

		for (int i = 0; i < dc; i++) {
			_dependency_step(deps[i], p_pass, pass_stack, input_args, output_args, variant_stack, r_error, error_str, r_error_node);
			if (r_error.error != Variant::CallError::CALL_OK) {
				return;
			}
		}
	}

	for (int i = 0; i < node->input_port_count; i++) {
		int index = node->input_ports[i] & BlockInstance::INPUT_MASK;

		if (node->input_ports[i] & BlockInstance::INPUT_DEFAULT_VALUE_BIT) {
			//is a default value (unassigned input port)
			input_args[i] = &default_values[index];
		} else {
			//regular temporary in stack
			input_args[i] = &variant_stack[index];
		}
	}
	for (int i = 0; i < node->output_port_count; i++) {
		output_args[i] = &variant_stack[node->output_ports[i]];
	}

	Variant *working_mem = node->working_mem_idx >= 0 ? &variant_stack[node->working_mem_idx] : (Variant *)nullptr;

	node->step(input_args, output_args, BlockInstance::START_MODE_BEGIN_SEQUENCE, working_mem, r_error, error_str);
	//ignore return
	if (r_error.error != Variant::CallError::CALL_OK) {
		*r_error_node = node;
	}
}

Variant C11RScriptInstance::_call_internal(const StringName &p_method, void *p_stack, int p_stack_size, BlockInstance *p_node, int p_flow_stack_pos, int p_pass, bool p_resuming_yield, Variant::CallError &r_error) {
	Map<StringName, Function>::Element *F = functions.find(p_method);
	ERR_FAIL_COND_V(!F, Variant());
	Function *f = &F->get();

	//this call goes separate, so it can e yielded and suspended
	Variant *variant_stack = (Variant *)p_stack;
	bool *sequence_bits = (bool *)(variant_stack + f->max_stack);
	const Variant **input_args = (const Variant **)(sequence_bits + f->node_count);
	Variant **output_args = (Variant **)(input_args + max_input_args);
	int flow_max = f->flow_stack_size;
	int *flow_stack = flow_max ? (int *)(output_args + max_output_args) : (int *)nullptr;
	int *pass_stack = flow_stack ? (int *)(flow_stack + flow_max) : (int *)nullptr;

	String error_str;

	BlockInstance *node = p_node;
	bool error = false;
	int current_node_id = f->node;
	Variant return_value;
	Variant *working_mem = nullptr;

	int flow_stack_pos = p_flow_stack_pos;

#ifdef DEBUG_ENABLED
	if (ScriptDebugger::get_singleton()) {
		C11RScriptLanguage::singleton->enter_function(this, &p_method, variant_stack, &working_mem, &current_node_id);
	}
#endif

	while (true) {
		p_pass++; //increment pass
		current_node_id = node->get_id();

		VSDEBUG("==========AT NODE: " + itos(current_node_id) + " base: " + node->get_base_node()->get_class_name());
		VSDEBUG("AT STACK POS: " + itos(flow_stack_pos));

		//setup working mem
		working_mem = node->working_mem_idx >= 0 ? &variant_stack[node->working_mem_idx] : (Variant *)nullptr;

		VSDEBUG("WORKING MEM: " + itos(node->working_mem_idx));

		if (current_node_id == f->node) {
			//if function node, set up function arguments from beginning of stack

			for (int i = 0; i < f->argument_count; i++) {
				input_args[i] = &variant_stack[i];
			}
		} else {
			//run dependencies first

			if (!node->dependencies.empty()) {
				int dc = node->dependencies.size();
				BlockInstance **deps = node->dependencies.ptrw();

				for (int i = 0; i < dc; i++) {
					_dependency_step(deps[i], p_pass, pass_stack, input_args, output_args, variant_stack, r_error, error_str, &node);
					if (r_error.error != Variant::CallError::CALL_OK) {
						error = true;
						current_node_id = node->id;
						break;
					}
				}
			}

			if (!error) {
				//setup input pointers normally
				VSDEBUG("INPUT PORTS: " + itos(node->input_port_count));

				for (int i = 0; i < node->input_port_count; i++) {
					int index = node->input_ports[i] & BlockInstance::INPUT_MASK;

					if (node->input_ports[i] & BlockInstance::INPUT_DEFAULT_VALUE_BIT) {
						//is a default value (unassigned input port)
						input_args[i] = &default_values[index];
						VSDEBUG("\tPORT " + itos(i) + " DEFAULT VAL");
					} else {
						//regular temporary in stack
						input_args[i] = &variant_stack[index];
						VSDEBUG("PORT " + itos(i) + " AT STACK " + itos(index));
					}
				}
			}
		}

		if (error) {
			break;
		}

		//setup output pointers

		VSDEBUG("OUTPUT PORTS: " + itos(node->output_port_count));
		for (int i = 0; i < node->output_port_count; i++) {
			output_args[i] = &variant_stack[node->output_ports[i]];
			VSDEBUG("PORT " + itos(i) + " AT STACK " + itos(node->output_ports[i]));
		}

		//do step

		BlockInstance::StartMode start_mode;
		{
			if (p_resuming_yield) {
				start_mode = BlockInstance::START_MODE_RESUME_YIELD;
				p_resuming_yield = false; // should resume only the first time
			} else if (flow_stack && (flow_stack[flow_stack_pos] & BlockInstance::FLOW_STACK_PUSHED_BIT)) {
				//if there is a push bit, it means we are continuing a sequence
				start_mode = BlockInstance::START_MODE_CONTINUE_SEQUENCE;
			} else {
				start_mode = BlockInstance::START_MODE_BEGIN_SEQUENCE;
			}
		}

		VSDEBUG("STEP - STARTSEQ: " + itos(start_mode));

		int ret = node->step(input_args, output_args, start_mode, working_mem, r_error, error_str);

		if (r_error.error != Variant::CallError::CALL_OK) {
			//use error from step
			error = true;
			break;
		}

		if (ret & BlockInstance::STEP_YIELD_BIT) {
			//yielded!
			if (node->get_working_memory_size() == 0) {
				r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
				error_str = RTR("A node yielded without working memory, please read the docs on how to yield properly!");
				error = true;
				break;

			} else {
				Ref<C11RScriptFunctionState> state = *working_mem;
				if (!state.is_valid()) {
					r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
					error_str = RTR("Node yielded, but did not return a function state in the first working memory.");
					error = true;
					break;
				}

				//step 1, capture all state
				state->instance_id = get_owner_ptr()->get_instance_id();
				state->script_id = get_script()->get_instance_id();
				state->instance = this;
				state->function = p_method;
				state->working_mem_index = node->working_mem_idx;
				state->variant_stack_size = f->max_stack;
				state->node = node;
				state->flow_stack_pos = flow_stack_pos;
				state->stack.resize(p_stack_size);
				state->pass = p_pass;
				memcpy(state->stack.ptrw(), p_stack, p_stack_size);
				//step 2, run away, return directly
				r_error.error = Variant::CallError::CALL_OK;

#ifdef DEBUG_ENABLED
				//will re-enter later, so exiting
				if (ScriptDebugger::get_singleton()) {
					C11RScriptLanguage::singleton->exit_function();
				}
#endif

				return state;
			}
		}

#ifdef DEBUG_ENABLED
		if (ScriptDebugger::get_singleton()) {
			// line
			bool do_break = false;

			if (ScriptDebugger::get_singleton()->get_lines_left() > 0) {
				if (ScriptDebugger::get_singleton()->get_depth() <= 0) {
					ScriptDebugger::get_singleton()->set_lines_left(ScriptDebugger::get_singleton()->get_lines_left() - 1);
				}
				if (ScriptDebugger::get_singleton()->get_lines_left() <= 0) {
					do_break = true;
				}
			}

			if (ScriptDebugger::get_singleton()->is_breakpoint(current_node_id, source)) {
				do_break = true;
			}

			if (do_break) {
				C11RScriptLanguage::singleton->debug_break("Breakpoint", true);
			}

			ScriptDebugger::get_singleton()->line_poll();
		}
#endif
		int output = ret & BlockInstance::STEP_MASK;

		VSDEBUG("STEP RETURN: " + itos(ret));

		if (ret & BlockInstance::STEP_EXIT_FUNCTION_BIT) {
			if (node->get_working_memory_size() == 0) {
				r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
				error_str = RTR("Return value must be assigned to first element of node working memory! Fix your node please.");
				error = true;
			} else {
				//assign from working memory, first element
				return_value = *working_mem;
			}

			VSDEBUG("EXITING FUNCTION - VALUE " + String(return_value));
			break; //exit function requested, bye
		}

		BlockInstance *next = nullptr; //next node

		if ((ret == output || ret & BlockInstance::STEP_FLAG_PUSH_STACK_BIT) && node->sequence_output_count) {
			//if no exit bit was set, and has sequence outputs, guess next node
			if (output >= node->sequence_output_count) {
				r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
				error_str = RTR("Node returned an invalid sequence output:") + " " + itos(output);
				error = true;
				break;
			}

			next = node->sequence_outputs[output];
			if (next) {
				VSDEBUG("GOT NEXT NODE - " + itos(next->get_id()));
			} else {
				VSDEBUG("GOT NEXT NODE - NULL");
			}
		}

		if (flow_stack) {
			//update flow stack pos (may have changed)
			flow_stack[flow_stack_pos] = current_node_id;

			//add stack push bit if requested
			if (ret & BlockInstance::STEP_FLAG_PUSH_STACK_BIT) {
				flow_stack[flow_stack_pos] |= BlockInstance::FLOW_STACK_PUSHED_BIT;
				sequence_bits[node->sequence_index] = true; //remember sequence bit
				VSDEBUG("NEXT SEQ - FLAG BIT");
			} else {
				sequence_bits[node->sequence_index] = false; //forget sequence bit
				VSDEBUG("NEXT SEQ - NORMAL");
			}

			if (ret & BlockInstance::STEP_FLAG_GO_BACK_BIT) {
				//go back request

				if (flow_stack_pos > 0) {
					flow_stack_pos--;
					node = instances[flow_stack[flow_stack_pos] & BlockInstance::FLOW_STACK_MASK];
					VSDEBUG("NEXT IS GO BACK");
				} else {
					VSDEBUG("NEXT IS GO BACK, BUT NO NEXT SO EXIT");
					break; //simply exit without value or error
				}
			} else if (next) {
				if (sequence_bits[next->sequence_index]) {
					// what happened here is that we are entering a node that is in the middle of doing a sequence (pushed stack) from the front
					// because each node has a working memory, we can't really do a sub-sequence
					// as a result, the sequence will be restarted and the stack will roll back to find where this node
					// started the sequence

					bool found = false;

					for (int i = flow_stack_pos; i >= 0; i--) {
						if ((flow_stack[i] & BlockInstance::FLOW_STACK_MASK) == next->get_id()) {
							flow_stack_pos = i; //roll back and remove bit
							flow_stack[i] = next->get_id();
							sequence_bits[next->sequence_index] = false;
							found = true;
						}
					}

					if (!found) {
						r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
						error_str = RTR("Found sequence bit but not the node in the stack, report bug!");
						error = true;
						break;
					}

					node = next;
					VSDEBUG("RE-ENTERED A LOOP, RETURNED STACK POS TO - " + itos(flow_stack_pos));

				} else {
					// check for stack overflow
					if (flow_stack_pos + 1 >= flow_max) {
						r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
						error_str = RTR("Stack overflow with stack depth:") + " " + itos(output);
						error = true;
						break;
					}

					node = next;

					flow_stack_pos++;
					flow_stack[flow_stack_pos] = node->get_id();

					VSDEBUG("INCREASE FLOW STACK");
				}

			} else {
				//no next node, try to go back in stack to pushed bit

				bool found = false;

				for (int i = flow_stack_pos; i >= 0; i--) {
					VSDEBUG("FS " + itos(i) + " - " + itos(flow_stack[i]));
					if (flow_stack[i] & BlockInstance::FLOW_STACK_PUSHED_BIT) {
						node = instances[flow_stack[i] & BlockInstance::FLOW_STACK_MASK];
						flow_stack_pos = i;
						found = true;
						break;
					}
				}

				if (!found) {
					VSDEBUG("NO NEXT NODE, NO GO BACK, EXITING");
					break; //done, couldn't find a push stack bit
				}

				VSDEBUG("NO NEXT NODE, GO BACK TO: " + itos(flow_stack_pos));
			}
		} else {
			node = next; //stackless mode, simply assign next node
		}
	}

	if (error) {
		//error
		// function, file, line, error, explanation
		String err_file = script->get_path();
		String err_func = p_method;
		int err_line = current_node_id; //not a line but it works as one

		if (node && (r_error.error != Variant::CallError::CALL_ERROR_INVALID_METHOD || error_str == String())) {
			if (error_str != String()) {
				error_str += " ";
			}

			if (r_error.error == Variant::CallError::CALL_ERROR_INVALID_ARGUMENT) {
				int errorarg = r_error.argument;
				error_str += "Cannot convert argument " + itos(errorarg + 1) + " to " + Variant::get_type_name(r_error.expected) + ".";
			} else if (r_error.error == Variant::CallError::CALL_ERROR_TOO_MANY_ARGUMENTS) {
				error_str += "Expected " + itos(r_error.argument) + " arguments.";
			} else if (r_error.error == Variant::CallError::CALL_ERROR_TOO_FEW_ARGUMENTS) {
				error_str += "Expected " + itos(r_error.argument) + " arguments.";
			} else if (r_error.error == Variant::CallError::CALL_ERROR_INVALID_METHOD) {
				error_str += "Invalid Call.";
			} else if (r_error.error == Variant::CallError::CALL_ERROR_INSTANCE_IS_NULL) {
				error_str += "Base Instance is null";
			}
		}

		//if (!GDScriptLanguage::get_singleton()->debug_break(err_text,false)) {
		// debugger break did not happen

		if (!C11RScriptLanguage::singleton->debug_break(error_str, false)) {
			_err_print_error(err_func.utf8().get_data(), err_file.utf8().get_data(), err_line, error_str.utf8().get_data(), ERR_HANDLER_SCRIPT);
		}

		//}
	} else {
		//return_value=
	}

#ifdef DEBUG_ENABLED
	if (ScriptDebugger::get_singleton()) {
		C11RScriptLanguage::singleton->exit_function();
	}
#endif

	//clean up variant stack
	for (int i = 0; i < f->max_stack; i++) {
		variant_stack[i].~Variant();
	}

	return return_value;
}

Variant C11RScriptInstance::call(const StringName &p_method, const Variant **p_args, int p_argcount, Variant::CallError &r_error) {
	r_error.error = Variant::CallError::CALL_OK; //ok by default

	Map<StringName, Function>::Element *F = functions.find(p_method);
	if (!F) {
		r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
		return Variant();
	}

	VSDEBUG("CALLING: " + String(p_method));

	Function *f = &F->get();

	int total_stack_size = 0;

	total_stack_size += f->max_stack * sizeof(Variant); //variants
	total_stack_size += f->node_count * sizeof(bool);
	total_stack_size += (max_input_args + max_output_args) * sizeof(Variant *); //arguments
	total_stack_size += f->flow_stack_size * sizeof(int); //flow
	total_stack_size += f->pass_stack_size * sizeof(int);

	VSDEBUG("STACK SIZE: " + itos(total_stack_size));
	VSDEBUG("STACK VARIANTS: : " + itos(f->max_stack));
	VSDEBUG("SEQBITS: : " + itos(f->node_count));
	VSDEBUG("MAX INPUT: " + itos(max_input_args));
	VSDEBUG("MAX OUTPUT: " + itos(max_output_args));
	VSDEBUG("FLOW STACK SIZE: " + itos(f->flow_stack_size));
	VSDEBUG("PASS STACK SIZE: " + itos(f->pass_stack_size));

	void *stack = alloca(total_stack_size);

	Variant *variant_stack = (Variant *)stack;
	bool *sequence_bits = (bool *)(variant_stack + f->max_stack);
	const Variant **input_args = (const Variant **)(sequence_bits + f->node_count);
	Variant **output_args = (Variant **)(input_args + max_input_args);
	int flow_max = f->flow_stack_size;
	int *flow_stack = flow_max ? (int *)(output_args + max_output_args) : (int *)nullptr;
	int *pass_stack = flow_stack ? (int *)(flow_stack + flow_max) : (int *)nullptr;

	for (int i = 0; i < f->node_count; i++) {
		sequence_bits[i] = false; //all starts as false
	}

	memset(pass_stack, 0, f->pass_stack_size * sizeof(int));

	Map<int, BlockInstance *>::Element *E = instances.find(f->node);
	if (!E) {
		r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;

		ERR_FAIL_V_MSG(Variant(), "No C11RScriptFunction node in function.");
	}

	BlockInstance *node = E->get();

	if (flow_stack) {
		flow_stack[0] = node->get_id();
	}

	VSDEBUG("ARGUMENTS: " + itos(f->argument_count) = " RECEIVED: " + itos(p_argcount));

	if (p_argcount < f->argument_count) {
		r_error.error = Variant::CallError::CALL_ERROR_TOO_FEW_ARGUMENTS;
		r_error.argument = node->get_input_port_count();

		return Variant();
	}

	if (p_argcount > f->argument_count) {
		r_error.error = Variant::CallError::CALL_ERROR_TOO_MANY_ARGUMENTS;
		r_error.argument = node->get_input_port_count();

		return Variant();
	}

	//allocate variant stack
	for (int i = 0; i < f->max_stack; i++) {
		memnew_placement(&variant_stack[i], Variant);
	}

	//allocate function arguments (must be copied for yield to work properly)
	for (int i = 0; i < p_argcount; i++) {
		variant_stack[i] = *p_args[i];
	}

	return _call_internal(p_method, stack, total_stack_size, node, 0, 0, false, r_error);
}

void C11RScriptInstance::notification(int p_notification) {
	//do nothing as this is called using virtual

	Variant what = p_notification;
	const Variant *whatp = &what;
	Variant::CallError ce;
	call(C11RScriptLanguage::singleton->notification, &whatp, 1, ce); //do as call
}

String C11RScriptInstance::to_string(bool *r_valid) {
	if (has_method(CoreStringNames::get_singleton()->_to_string)) {
		Variant::CallError ce;
		Variant ret = call(CoreStringNames::get_singleton()->_to_string, nullptr, 0, ce);
		if (ce.error == Variant::CallError::CALL_OK) {
			if (ret.get_type() != Variant::STRING) {
				if (r_valid) {
					*r_valid = false;
				}
				ERR_FAIL_V_MSG(String(), "Wrong type for " + CoreStringNames::get_singleton()->_to_string + ", must be a String.");
			}
			if (r_valid) {
				*r_valid = true;
			}
			return ret.operator String();
		}
	}
	if (r_valid) {
		*r_valid = false;
	}
	return String();
}

Ref<Script> C11RScriptInstance::get_script() const {
	return script;
}

MultiplayerAPI::RPCMode C11RScriptInstance::get_rpc_mode(const StringName &p_method) const {
	if (p_method == script->get_default_func()) {
		return MultiplayerAPI::RPC_MODE_DISABLED;
	}

	const Map<StringName, C11RScript::Function>::Element *E = script->functions.find(p_method);
	if (!E) {
		return MultiplayerAPI::RPC_MODE_DISABLED;
	}

	if (E->get().function_id >= 0 && E->get().nodes.has(E->get().function_id)) {
		Ref<VisualScriptFunction> vsf = E->get().nodes[E->get().function_id].node;
		if (vsf.is_valid()) {
			return vsf->get_rpc_mode();
		}
	}

	return MultiplayerAPI::RPC_MODE_DISABLED;
}

MultiplayerAPI::RPCMode C11RScriptInstance::get_rset_mode(const StringName &p_variable) const {
	return MultiplayerAPI::RPC_MODE_DISABLED;
}

void C11RScriptInstance::create(const Ref<C11RScript> &p_script, Object *p_owner) {
	script = p_script;
	owner = p_owner;
	source = p_script->get_path();

	max_input_args = 0;
	max_output_args = 0;

	if (Object::cast_to<Node>(p_owner)) {
		//turn on these if they exist and base is a node
		Node *node = Object::cast_to<Node>(p_owner);
		if (p_script->functions.has("_process")) {
			node->set_process(true);
		}
		if (p_script->functions.has("_physics_process")) {
			node->set_physics_process(true);
		}
		if (p_script->functions.has("_input")) {
			node->set_process_input(true);
		}
		if (p_script->functions.has("_unhandled_input")) {
			node->set_process_unhandled_input(true);
		}
		if (p_script->functions.has("_unhandled_key_input")) {
			node->set_process_unhandled_key_input(true);
		}
	}

	for (const Map<StringName, C11RScript::Variable>::Element *E = script->variables.front(); E; E = E->next()) {
		variables[E->key()] = E->get().default_value;
	}

	for (const Map<StringName, C11RScript::Function>::Element *E = script->functions.front(); E; E = E->next()) {
		if (E->key() == script->get_default_func()) {
			continue;
		}

		Function function;
		function.node = E->get().function_id;
		function.max_stack = 0;
		function.flow_stack_size = 0;
		function.pass_stack_size = 0;
		function.node_count = 0;

		Map<StringName, int> local_var_indices;

		if (function.node < 0) {
			C11RScriptLanguage::singleton->debug_break_parse(get_script()->get_path(), 0, "No start node in function: " + String(E->key()));

			ERR_CONTINUE(function.node < 0);
		}

		{
			Ref<VisualScriptFunction> func_node = script->get_node(E->key(), E->get().function_id);

			if (func_node.is_null()) {
				C11RScriptLanguage::singleton->debug_break_parse(get_script()->get_path(), 0, "No C11RScriptFunction typed start node in function: " + String(E->key()));
			}

			ERR_CONTINUE(!func_node.is_valid());

			function.argument_count = func_node->get_argument_count();
			function.max_stack += function.argument_count;
			function.flow_stack_size = func_node->is_stack_less() ? 0 : func_node->get_stack_size();
			max_input_args = MAX(max_input_args, function.argument_count);
		}

		//multiple passes are required to set up this complex thing..

		//first create the nodes
		for (const Map<int, C11RScript::Function::NodeData>::Element *F = E->get().nodes.front(); F; F = F->next()) {
			Ref<Block> node = F->get().node;

			BlockInstance *instance = node->instance(this); //create instance
			ERR_FAIL_COND(!instance);

			instance->base = node.ptr();

			instance->id = F->key();
			instance->input_port_count = node->get_input_value_port_count();
			instance->input_ports = nullptr;
			instance->output_port_count = node->get_output_value_port_count();
			instance->output_ports = nullptr;
			instance->sequence_output_count = node->get_output_sequence_port_count();
			instance->sequence_index = function.node_count++;
			instance->sequence_outputs = nullptr;
			instance->pass_idx = -1;

			if (instance->input_port_count) {
				instance->input_ports = memnew_arr(int, instance->input_port_count);
				for (int i = 0; i < instance->input_port_count; i++) {
					instance->input_ports[i] = -1; //if not assigned, will become default value
				}
			}

			if (instance->output_port_count) {
				instance->output_ports = memnew_arr(int, instance->output_port_count);
				for (int i = 0; i < instance->output_port_count; i++) {
					instance->output_ports[i] = -1; //if not assigned, will output to trash
				}
			}

			if (instance->sequence_output_count) {
				instance->sequence_outputs = memnew_arr(BlockInstance *, instance->sequence_output_count);
				for (int i = 0; i < instance->sequence_output_count; i++) {
					instance->sequence_outputs[i] = nullptr; //if it remains null, flow ends here
				}
			}

			if (Object::cast_to<VisualScriptLocalVar>(node.ptr()) || Object::cast_to<VisualScriptLocalVarSet>(*node)) {
				//working memory is shared only for this node, for the same variables
				Ref<VisualScriptLocalVar> vslv = node;

				StringName var_name;

				if (Object::cast_to<VisualScriptLocalVar>(*node)) {
					var_name = String(Object::cast_to<VisualScriptLocalVar>(*node)->get_var_name()).strip_edges();
				} else {
					var_name = String(Object::cast_to<VisualScriptLocalVarSet>(*node)->get_var_name()).strip_edges();
				}

				if (!local_var_indices.has(var_name)) {
					local_var_indices[var_name] = function.max_stack;
					function.max_stack++;
				}

				instance->working_mem_idx = local_var_indices[var_name];

			} else if (instance->get_working_memory_size()) {
				instance->working_mem_idx = function.max_stack;
				function.max_stack += instance->get_working_memory_size();
			} else {
				instance->working_mem_idx = -1; //no working mem
			}

			max_input_args = MAX(max_input_args, instance->input_port_count);
			max_output_args = MAX(max_output_args, instance->output_port_count);

			instances[F->key()] = instance;
		}

		function.trash_pos = function.max_stack++; //create pos for trash

		//second pass, do data connections

		for (const Set<C11RScript::DataConnection>::Element *F = E->get().data_connections.front(); F; F = F->next()) {
			C11RScript::DataConnection dc = F->get();
			ERR_CONTINUE(!instances.has(dc.from_node));
			BlockInstance *from = instances[dc.from_node];
			ERR_CONTINUE(!instances.has(dc.to_node));
			BlockInstance *to = instances[dc.to_node];
			ERR_CONTINUE(dc.from_port >= from->output_port_count);
			ERR_CONTINUE(dc.to_port >= to->input_port_count);

			if (from->output_ports[dc.from_port] == -1) {
				int stack_pos = function.max_stack++;
				from->output_ports[dc.from_port] = stack_pos;
			}

			if (from->get_sequence_output_count() == 0 && to->dependencies.find(from) == -1) {
				//if the node we are reading from has no output sequence, we must call step() before reading from it.
				if (from->pass_idx == -1) {
					from->pass_idx = function.pass_stack_size;
					function.pass_stack_size++;
				}
				to->dependencies.push_back(from);
			}

			to->input_ports[dc.to_port] = from->output_ports[dc.from_port]; //read from wherever the stack is
		}

		//third pass, do sequence connections

		for (const Set<C11RScript::SequenceConnection>::Element *F = E->get().sequence_connections.front(); F; F = F->next()) {
			C11RScript::SequenceConnection sc = F->get();
			ERR_CONTINUE(!instances.has(sc.from_node));
			BlockInstance *from = instances[sc.from_node];
			ERR_CONTINUE(!instances.has(sc.to_node));
			BlockInstance *to = instances[sc.to_node];
			ERR_CONTINUE(sc.from_output >= from->sequence_output_count);

			from->sequence_outputs[sc.from_output] = to;
		}

		//fourth pass:
		// 1) unassigned input ports to default values
		// 2) connect unassigned output ports to trash

		for (const Map<int, C11RScript::Function::NodeData>::Element *F = E->get().nodes.front(); F; F = F->next()) {
			ERR_CONTINUE(!instances.has(F->key()));

			Ref<Block> node = F->get().node;
			BlockInstance *instance = instances[F->key()];

			// connect to default values
			for (int i = 0; i < instance->input_port_count; i++) {
				if (instance->input_ports[i] == -1) {
					//unassigned, connect to default val
					instance->input_ports[i] = default_values.size() | BlockInstance::INPUT_DEFAULT_VALUE_BIT;
					default_values.push_back(node->get_default_input_value(i));
				}
			}

			// connect to trash
			for (int i = 0; i < instance->output_port_count; i++) {
				if (instance->output_ports[i] == -1) {
					instance->output_ports[i] = function.trash_pos; //trash is same for all
				}
			}
		}

		functions[E->key()] = function;
	}
}

ScriptLanguage *C11RScriptInstance::get_language() {
	return C11RScriptLanguage::singleton;
}

C11RScriptInstance::C11RScriptInstance() {
}

C11RScriptInstance::~C11RScriptInstance() {
	C11RScriptLanguage::singleton->lock.lock();
	script->instances.erase(owner);
	C11RScriptLanguage::singleton->lock.unlock();

	for (Map<int, BlockInstance *>::Element *E = instances.front(); E; E = E->next()) {
		memdelete(E->get());
	}
}
