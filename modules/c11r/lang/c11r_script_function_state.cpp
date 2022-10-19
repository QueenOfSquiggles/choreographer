#include "../c11r_lang.hpp"

#include "core/core_string_names.h"
#include "core/os/os.h"
#include "core/project_settings.h"
#include "scene/main/node.h"
#include "modules/visual_script/visual_script_func_nodes.h"
#include "modules/visual_script/visual_script_nodes.h"



Variant C11RScriptFunctionState::_signal_callback(const Variant **p_args, int p_argcount, Variant::CallError &r_error) {
	ERR_FAIL_COND_V(function == StringName(), Variant());

#ifdef DEBUG_ENABLED

	ERR_FAIL_COND_V_MSG(instance_id && !ObjectDB::get_instance(instance_id), Variant(), "Resumed after yield, but class instance is gone.");
	ERR_FAIL_COND_V_MSG(script_id && !ObjectDB::get_instance(script_id), Variant(), "Resumed after yield, but script is gone.");

#endif

	r_error.error = Variant::CallError::CALL_OK;

	Array args;

	if (p_argcount == 0) {
		r_error.error = Variant::CallError::CALL_ERROR_TOO_FEW_ARGUMENTS;
		r_error.argument = 1;
		return Variant();
	} else if (p_argcount == 1) {
		//noooneee, reserved for me, me and only me.
	} else {
		for (int i = 0; i < p_argcount - 1; i++) {
			args.push_back(*p_args[i]);
		}
	}

	Ref<C11RScriptFunctionState> self = *p_args[p_argcount - 1]; //hi, I'm myself, needed this to remain alive.

	if (self.is_null()) {
		r_error.error = Variant::CallError::CALL_ERROR_INVALID_ARGUMENT;
		r_error.argument = p_argcount - 1;
		r_error.expected = Variant::OBJECT;
		return Variant();
	}

	r_error.error = Variant::CallError::CALL_OK;

	Variant *working_mem = ((Variant *)stack.ptr()) + working_mem_index;

	*working_mem = args; //arguments go to working mem.

	Variant ret = instance->_call_internal(function, stack.ptrw(), stack.size(), node, flow_stack_pos, pass, true, r_error);
	function = StringName(); //invalidate
	return ret;
}

void C11RScriptFunctionState::connect_to_signal(Object *p_obj, const String &p_signal, Array p_binds) {
	ERR_FAIL_NULL(p_obj);
	Vector<Variant> binds;
	for (int i = 0; i < p_binds.size(); i++) {
		binds.push_back(p_binds[i]);
	}
	binds.push_back(Ref<C11RScriptFunctionState>(this)); //add myself on the back to avoid dying from unreferencing
	p_obj->connect(p_signal, this, "_signal_callback", binds, CONNECT_ONESHOT);
}

bool C11RScriptFunctionState::is_valid() const {
	return function != StringName();
}

Variant C11RScriptFunctionState::resume(Array p_args) {
	ERR_FAIL_COND_V(function == StringName(), Variant());
#ifdef DEBUG_ENABLED

	ERR_FAIL_COND_V_MSG(instance_id && !ObjectDB::get_instance(instance_id), Variant(), "Resumed after yield, but class instance is gone.");
	ERR_FAIL_COND_V_MSG(script_id && !ObjectDB::get_instance(script_id), Variant(), "Resumed after yield, but script is gone.");

#endif

	Variant::CallError r_error;
	r_error.error = Variant::CallError::CALL_OK;

	Variant *working_mem = ((Variant *)stack.ptr()) + working_mem_index;

	*working_mem = p_args; //arguments go to working mem.

	Variant ret = instance->_call_internal(function, stack.ptrw(), stack.size(), node, flow_stack_pos, pass, true, r_error);
	function = StringName(); //invalidate
	return ret;
}

void C11RScriptFunctionState::_bind_methods() {
	ClassDB::bind_method(D_METHOD("connect_to_signal", "obj", "signals", "args"), &C11RScriptFunctionState::connect_to_signal);
	ClassDB::bind_method(D_METHOD("resume", "args"), &C11RScriptFunctionState::resume, DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("is_valid"), &C11RScriptFunctionState::is_valid);
	ClassDB::bind_vararg_method(METHOD_FLAGS_DEFAULT, "_signal_callback", &C11RScriptFunctionState::_signal_callback, MethodInfo("_signal_callback"));
}

C11RScriptFunctionState::C11RScriptFunctionState() {
}

C11RScriptFunctionState::~C11RScriptFunctionState() {
	if (function != StringName()) {
		Variant *s = ((Variant *)stack.ptr());
		for (int i = 0; i < variant_stack_size; i++) {
			s[i].~Variant();
		}
	}
}
