
#include "c11r_lang.hpp"

#include "core/core_string_names.h"
#include "core/os/os.h"
#include "core/project_settings.h"
#include "scene/main/node.h"
#include "modules/visual_script/visual_script_func_nodes.h"
#include "modules/visual_script/visual_script_nodes.h"

//used by editor, this is not really saved
void Block::set_breakpoint(bool p_breakpoint) {
	breakpoint = p_breakpoint;
}

bool Block::is_breakpoint() const {
	return breakpoint;
}

void Block::ports_changed_notify() {
	emit_signal("ports_changed");
}

void Block::set_default_input_value(int p_port, const Variant &p_value) {
	ERR_FAIL_INDEX(p_port, default_input_values.size());

	default_input_values[p_port] = p_value;

#ifdef TOOLS_ENABLED
	for (Set<C11RScript *>::Element *E = scripts_used.front(); E; E = E->next()) {
		E->get()->set_edited(true);
	}
#endif
}

Variant Block::get_default_input_value(int p_port) const {
	ERR_FAIL_INDEX_V(p_port, default_input_values.size(), Variant());
	return default_input_values[p_port];
}

void Block::_set_default_input_values(Array p_values) {
	default_input_values = p_values;
}

void Block::validate_input_default_values() {
	default_input_values.resize(MAX(default_input_values.size(), get_input_value_port_count())); //let it grow as big as possible, we don't want to lose values on resize

	//actually validate on save
	for (int i = 0; i < get_input_value_port_count(); i++) {
		Variant::Type expected = get_input_value_port_info(i).type;

		if (expected == Variant::NIL || expected == default_input_values[i].get_type()) {
			continue;
		} else {
			//not the same, reconvert
			Variant::CallError ce;
			Variant existing = default_input_values[i];
			const Variant *existingp = &existing;
			default_input_values[i] = Variant::construct(expected, &existingp, 1, ce, false);
			if (ce.error != Variant::CallError::CALL_OK) {
				//could not convert? force..
				default_input_values[i] = Variant::construct(expected, nullptr, 0, ce, false);
			}
		}
	}
}

Array Block::_get_default_input_values() const {
	//validate on save, since on load there is little info about this
	Array values = default_input_values;
	values.resize(get_input_value_port_count());

	return values;
}

String Block::get_text() const {
	return "";
}

void Block::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_visual_script"), &Block::get_visual_script);
	ClassDB::bind_method(D_METHOD("set_default_input_value", "port_idx", "value"), &Block::set_default_input_value);
	ClassDB::bind_method(D_METHOD("get_default_input_value", "port_idx"), &Block::get_default_input_value);
	ClassDB::bind_method(D_METHOD("ports_changed_notify"), &Block::ports_changed_notify);
	ClassDB::bind_method(D_METHOD("_set_default_input_values", "values"), &Block::_set_default_input_values);
	ClassDB::bind_method(D_METHOD("_get_default_input_values"), &Block::_get_default_input_values);
	ClassDB::bind_method(D_METHOD("_set_block_namespace"), &Block::_set_block_namespace);
	ClassDB::bind_method(D_METHOD("_get_block_namespace"), &Block::_get_block_namespace);
	
	MethodInfo block_call_func;
	block_call_func.name="_call_block";
	block_call_func.arguments.push_back(PropertyInfo(Variant::ARRAY, "arg_stack"));
	block_call_func.return_val = PropertyInfo(Variant::ARRAY, "return_arg_stack");
	ClassDB::add_virtual_method("Block", block_call_func);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "_default_input_values", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR | PROPERTY_USAGE_INTERNAL), "_set_default_input_values", "_get_default_input_values");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "block_namespace", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR),"_set_block_namespace", "_get_block_namespace");
	ADD_SIGNAL(MethodInfo("ports_changed"));

}

void Block::_set_block_namespace(const String &p_namespace)
{
	block_namespace = p_namespace;
}
String Block::_get_block_namespace()
{
	return block_namespace;
}

Block::TypeGuess Block::guess_output_type(TypeGuess *p_inputs, int p_output) const {
	PropertyInfo pinfo = get_output_value_port_info(p_output);

	TypeGuess tg;

	tg.type = pinfo.type;
	if (pinfo.hint == PROPERTY_HINT_RESOURCE_TYPE) {
		tg.gdclass = pinfo.hint_string;
	}

	return tg;
}

Ref<C11RScript> Block::get_visual_script() const {
	if (scripts_used.size()) {
		return Ref<C11RScript>(scripts_used.front()->get());
	}

	return Ref<C11RScript>();
}

Block::Block() {
	breakpoint = false;
}

////////////////

/////////////////////

BlockInstance::BlockInstance() {
	sequence_outputs = nullptr;
	input_ports = nullptr;
}

BlockInstance::~BlockInstance() {
	if (sequence_outputs) {
		memdelete_arr(sequence_outputs);GDScript 
	}

	if (input_ports) {
		memdelete_arr(input_ports);
	}

	if (output_ports) {
		memdelete_arr(output_ports);
	}
}
C11RScript::C11RScript() {
	base_type = "Object";
	is_tool_script = false;
}

C11RScript::~C11RScript() {
}

void C11RScript::load(Ref<ConfigFile> p_configuration)
{
	// TODO load information
	print_line("Loading C11RScript from custom loader (ConfigFile)");
}

Ref<ConfigFile> C11RScript::save() const
{
	Ref<ConfigFile> cfg;
	cfg.instance();
	cfg->set_value("header", "number", 0.5);
	cfg->set_value("header", "name", "QueenOfSquiggles");
	cfg->set_value("metadata", "garbage", rand());
	cfg->set_value("metadata", "trash", rand());
	cfg->set_value("metadata", "rubbish", rand());
	return cfg;
}

////////////////////////////////////////////

bool C11RScriptInstance::set(const StringName &p_name, const Variant &p_value) {
}

bool C11RScriptInstance::get(const StringName &p_name, Variant &r_ret) const {
}
void C11RScriptInstance::get_property_list(List<PropertyInfo> *p_properties) const {
}
Variant::Type C11RScriptInstance::get_property_type(const StringName &p_name, bool *r_is_valid) const {
}

void C11RScriptInstance::get_method_list(List<MethodInfo> *p_list) const {
}
bool C11RScriptInstance::has_method(const StringName &p_method) const {
	return false;
}

Variant C11RScriptInstance::call(const StringName &p_method, const Variant **p_args, int p_argcount, Variant::CallError &r_error) {
	return Variant();
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
	return nullptr;
}

MultiplayerAPI::RPCMode C11RScriptInstance::get_rpc_mode(const StringName &p_method) const {
	return MultiplayerAPI::RPC_MODE_DISABLED;
}

MultiplayerAPI::RPCMode C11RScriptInstance::get_rset_mode(const StringName &p_variable) const {
	return MultiplayerAPI::RPC_MODE_DISABLED;
}
ScriptLanguage *C11RScriptInstance::get_language() {
	return C11RScriptLanguage::singleton;
}

C11RScriptInstance::C11RScriptInstance() {
}

C11RScriptInstance::~C11RScriptInstance() {
}

/////////////////////////////////////////////

/////////////////////

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

///////////////////////////////////////////////

String C11RScriptLanguage::get_name() const {
	return "Choreographer";
}

/* LANGUAGE FUNCTIONS */
void C11RScriptLanguage::init() {
}
String C11RScriptLanguage::get_type() const {
	return "Choregrapher";
}
String C11RScriptLanguage::get_extension() const {
	return C11R_LANG_FILE_EXT;
}
Error C11RScriptLanguage::execute_file(const String &p_path) {
	return OK;
}
void C11RScriptLanguage::finish() {
}

/* EDITOR FUNCTIONS */
void C11RScriptLanguage::get_reserved_words(List<String> *p_words) const {
}
bool C11RScriptLanguage::is_control_flow_keyword(String p_keyword) const {
	return false;
}
void C11RScriptLanguage::get_comment_delimiters(List<String> *p_delimiters) const {
}
void C11RScriptLanguage::get_string_delimiters(List<String> *p_delimiters) const {
}
Ref<Script> C11RScriptLanguage::get_template(const String &p_class_name, const String &p_base_class_name) const {
	Ref<C11RScript> script;
	script.instance();
	script->set_instance_base_type(p_base_class_name);
	return script;
}

bool C11RScriptLanguage::is_using_templates() {
	return true;
}

void C11RScriptLanguage::make_template(const String &p_class_name, const String &p_base_class_name, Ref<Script> &p_script) {
	Ref<C11RScript> script = p_script;
	script->set_instance_base_type(p_base_class_name);
}

bool C11RScriptLanguage::validate(const String &p_script, int &r_line_error, int &r_col_error, String &r_test_error, const String &p_path, List<String> *r_functions, List<ScriptLanguage::Warning> *r_warnings, Set<int> *r_safe_lines) const {
	return false;
}
Script *C11RScriptLanguage::create_script() const {
	return memnew(C11RScript);
}
bool C11RScriptLanguage::has_named_classes() const {
	return false;
}
bool C11RScriptLanguage::supports_builtin_mode() const {
	return true;
}
int C11RScriptLanguage::find_function(const String &p_function, const String &p_code) const {
	return -1;
}
String C11RScriptLanguage::make_function(const String &p_class, const String &p_name, const PoolStringArray &p_args) const {
	return String();
}

void C11RScriptLanguage::auto_indent_code(String &p_code, int p_from_line, int p_to_line) const {
}
void C11RScriptLanguage::add_global_constant(const StringName &p_variable, const Variant &p_value) {
}

/* DEBUGGER FUNCTIONS */

bool C11RScriptLanguage::debug_break_parse(const String &p_file, int p_node, const String &p_error) {
	//break because of parse error

	if (ScriptDebugger::get_singleton() && Thread::get_caller_id() == Thread::get_main_id()) {
		_debug_parse_err_node = p_node;
		_debug_parse_err_file = p_file;
		_debug_error = p_error;
		ScriptDebugger::get_singleton()->debug(this, false, true);
		return true;
	} else {
		return false;
	}
}

bool C11RScriptLanguage::debug_break(const String &p_error, bool p_allow_continue) {
	if (ScriptDebugger::get_singleton() && Thread::get_caller_id() == Thread::get_main_id()) {
		_debug_parse_err_node = -1;
		_debug_parse_err_file = "";
		_debug_error = p_error;
		ScriptDebugger::get_singleton()->debug(this, p_allow_continue, true);
		return true;
	} else {
		return false;
	}
}

String C11RScriptLanguage::debug_get_error() const {
	return _debug_error;
}

int C11RScriptLanguage::debug_get_stack_level_count() const {
	if (_debug_parse_err_node >= 0) {
		return 1;
	}

	return _debug_call_stack_pos;
}
int C11RScriptLanguage::debug_get_stack_level_line(int p_level) const {
	if (_debug_parse_err_node >= 0) {
		return _debug_parse_err_node;
	}

	ERR_FAIL_INDEX_V(p_level, _debug_call_stack_pos, -1);

	int l = _debug_call_stack_pos - p_level - 1;

	return *(_call_stack[l].current_id);
}
String C11RScriptLanguage::debug_get_stack_level_function(int p_level) const {
	if (_debug_parse_err_node >= 0) {
		return "";
	}

	ERR_FAIL_INDEX_V(p_level, _debug_call_stack_pos, "");
	int l = _debug_call_stack_pos - p_level - 1;
	return *_call_stack[l].function;
}
String C11RScriptLanguage::debug_get_stack_level_source(int p_level) const {
	if (_debug_parse_err_node >= 0) {
		return _debug_parse_err_file;
	}

	ERR_FAIL_INDEX_V(p_level, _debug_call_stack_pos, "");
	int l = _debug_call_stack_pos - p_level - 1;
	return _call_stack[l].instance->get_script_ptr()->get_path();
}
void C11RScriptLanguage::debug_get_stack_level_locals(int p_level, List<String> *p_locals, List<Variant> *p_values, int p_max_subitems, int p_max_depth) {
	if (_debug_parse_err_node >= 0) {
		return;
	}

	ERR_FAIL_INDEX(p_level, _debug_call_stack_pos);

	int l = _debug_call_stack_pos - p_level - 1;
	const StringName *f = _call_stack[l].function;

	ERR_FAIL_COND(!_call_stack[l].instance->functions.has(*f));

	BlockInstance *node = _call_stack[l].instance->instances[*_call_stack[l].current_id];
	ERR_FAIL_COND(!node);

	p_locals->push_back("node_name");
	p_values->push_back(node->get_base_node()->get_text());

	for (int i = 0; i < node->input_port_count; i++) {
		String name = node->get_base_node()->get_input_value_port_info(i).name;
		if (name == String()) {
			name = "in_" + itos(i);
		}

		p_locals->push_back("input/" + name);

		//value is trickier

		int in_from = node->input_ports[i];
		int in_value = in_from & BlockInstance::INPUT_MASK;

		if (in_from & BlockInstance::INPUT_DEFAULT_VALUE_BIT) {
			p_values->push_back(_call_stack[l].instance->default_values[in_value]);
		} else {
			p_values->push_back(_call_stack[l].stack[in_value]);
		}
	}

	for (int i = 0; i < node->output_port_count; i++) {
		String name = node->get_base_node()->get_output_value_port_info(i).name;
		if (name == String()) {
			name = "out_" + itos(i);
		}

		p_locals->push_back("output/" + name);

		//value is trickier

		int in_from = node->output_ports[i];
		p_values->push_back(_call_stack[l].stack[in_from]);
	}

	for (int i = 0; i < node->get_working_memory_size(); i++) {
		p_locals->push_back("working_mem/mem_" + itos(i));
		p_values->push_back((*_call_stack[l].work_mem)[i]);
	}
}

void C11RScriptLanguage::debug_get_stack_level_members(int p_level, List<String> *p_members, List<Variant> *p_values, int p_max_subitems, int p_max_depth) {
	if (_debug_parse_err_node >= 0) {
		return;
	}

	ERR_FAIL_INDEX(p_level, _debug_call_stack_pos);
	int l = _debug_call_stack_pos - p_level - 1;

	Ref<C11RScript> vs = _call_stack[l].instance->get_script();
	if (vs.is_null()) {
		return;
	}

	List<StringName> vars;
	vs->get_variable_list(&vars);
	for (List<StringName>::Element *E = vars.front(); E; E = E->next()) {
		Variant v;
		if (_call_stack[l].instance->get_variable(E->get(), &v)) {
			p_members->push_back("variables/" + E->get());
			p_values->push_back(v);
		}
	}
}

void C11RScriptLanguage::debug_get_globals(List<String> *p_locals, List<Variant> *p_values, int p_max_subitems, int p_max_depth) {
	//no globals are really reachable in gdscript
}
String C11RScriptLanguage::debug_parse_stack_level_expression(int p_level, const String &p_expression, int p_max_subitems, int p_max_depth) {
	return "";
}

void C11RScriptLanguage::reload_all_scripts() {
}
void C11RScriptLanguage::reload_tool_script(const Ref<Script> &p_script, bool p_soft_reload) {
}
/* LOADER FUNCTIONS */

void C11RScriptLanguage::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back(C11R_LANG_FILE_EXT);
}
void C11RScriptLanguage::get_public_functions(List<MethodInfo> *p_functions) const {
}
void C11RScriptLanguage::get_public_constants(List<Pair<String, Variant>> *p_constants) const {
}

void C11RScriptLanguage::profiling_start() {
}
void C11RScriptLanguage::profiling_stop() {
}

int C11RScriptLanguage::profiling_get_accumulated_data(ProfilingInfo *p_info_arr, int p_info_max) {
	return 0;
}

int C11RScriptLanguage::profiling_get_frame_data(ProfilingInfo *p_info_arr, int p_info_max) {
	return 0;
}

C11RScriptLanguage *C11RScriptLanguage::singleton = nullptr;

void C11RScriptLanguage::add_register_func(const String &p_name, BlockRegisterFunc p_func) {
	ERR_FAIL_COND(register_funcs.has(p_name));
	register_funcs[p_name] = p_func;
}

void C11RScriptLanguage::remove_register_func(const String &p_name) {
	ERR_FAIL_COND(!register_funcs.has(p_name));
	register_funcs.erase(p_name);
}

Ref<Block> C11RScriptLanguage::create_node_from_name(const String &p_name) {
	ERR_FAIL_COND_V(!register_funcs.has(p_name), Ref<Block>());

	return register_funcs[p_name](p_name);
}

void C11RScriptLanguage::get_registered_node_names(List<String> *r_names) {
	for (Map<String, BlockRegisterFunc>::Element *E = register_funcs.front(); E; E = E->next()) {
		r_names->push_back(E->key());
	}
}

C11RScriptLanguage::C11RScriptLanguage() {
	notification = "_notification";
	_step = "_step";
	_subcall = "_subcall";
	singleton = this;

	_debug_parse_err_node = -1;
	_debug_parse_err_file = "";
	_debug_call_stack_pos = 0;
	int dmcs = GLOBAL_DEF("debug/settings/visual_script/max_call_stack", 1024);
	ProjectSettings::get_singleton()->set_custom_property_info("debug/settings/visual_script/max_call_stack", PropertyInfo(Variant::INT, "debug/settings/visual_script/max_call_stack", PROPERTY_HINT_RANGE, "1024,4096,1,or_greater")); //minimum is 1024

	if (ScriptDebugger::get_singleton()) {
		//debugging enabled!
		_debug_max_call_stack = dmcs;
		_call_stack = memnew_arr(CallLevel, _debug_max_call_stack + 1);

	} else {
		_debug_max_call_stack = 0;
		_call_stack = nullptr;
	}
}

C11RScriptLanguage::~C11RScriptLanguage() {
	if (_call_stack) {
		memdelete_arr(_call_stack);
	}
	singleton = nullptr;
}
