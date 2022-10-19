#include "../c11r_lang.hpp"

#include "core/core_string_names.h"
#include "core/os/os.h"
#include "core/project_settings.h"
#include "scene/main/node.h"
#include "modules/visual_script/visual_script_func_nodes.h"
#include "modules/visual_script/visual_script_nodes.h"


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
	return "c11r";
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
	return true;
}
bool C11RScriptLanguage::supports_builtin_mode() const {
	return true; // TODO is this possible? Or even desireable?
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
	return ""; // TODO provide debugging information
}

void C11RScriptLanguage::reload_all_scripts() {
}
void C11RScriptLanguage::reload_tool_script(const Ref<Script> &p_script, bool p_soft_reload) {
}
/* LOADER FUNCTIONS */

void C11RScriptLanguage::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("c11r");
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
	return 0; // TODO provide debugging data
}

int C11RScriptLanguage::profiling_get_frame_data(ProfilingInfo *p_info_arr, int p_info_max) {
	return 0; // TODO provide debugging data
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
