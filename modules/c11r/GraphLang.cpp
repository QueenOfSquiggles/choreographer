#include "GraphLang.hpp"

#pragma region Block

void Block::_bind_methods()
{

}

#pragma endregion

#pragma region C11RScript

void C11RScript::_bind_methods()
{

}

//  - - - - - - - - - - - - - - - -
//  Script: funcs
//  - - - - - - - - - - - - - - - - 

bool C11RScript::can_instance() const {}

Ref<Script> C11RScript::get_base_script() const {} //for script inheritance

bool C11RScript::inherits_script(const Ref<Script> &p_script) const {}

StringName C11RScript::get_instance_base_type() const {} // this may not work in all scripts, will return empty if so

ScriptInstance *C11RScript::instance_create(Object *p_this) {}

bool C11RScript::instance_has(const Object *p_this) const {}

bool C11RScript::has_source_code() const {}

String C11RScript::get_source_code() const {}

void C11RScript::set_source_code(const String &p_code) {}

Error C11RScript::reload(bool p_keep_state = false) {}

bool C11RScript::has_method(const StringName &p_method) const {}

MethodInfo C11RScript::get_method_info(const StringName &p_method) const {}

bool C11RScript::is_tool() const {}

bool C11RScript::is_valid() const {}

ScriptLanguage *C11RScript::get_language() const {}

bool C11RScript::has_script_signal(const StringName &p_signal) const {}

void C11RScript::get_script_signal_list(List<MethodInfo> *r_signals) const {}

bool C11RScript::get_property_default_value(const StringName &p_property, Variant &r_value) const {}

void C11RScript::get_script_method_list(List<MethodInfo> *p_list) const {}

void C11RScript::get_script_property_list(List<PropertyInfo> *p_list) const {}

#pragma endregion

#pragma region C11RScriptLanguage

C11RScriptLanguage::C11RScriptLanguage()
{

}
C11RScriptLanguage::~C11RScriptLanguage()
{

}

/////////////////////////////////////////////////////
//    C11RScriptLanguage : ScriptLanguage overrides
/////////////////////////////////////////////////////
String C11RScriptLanguage::get_name() const
{
    return "Choreographer";
}

/* LANGUAGE FUNCTIONS */
void C11RScriptLanguage::init(){}

String C11RScriptLanguage::get_type() const
{
    return "Choreographer";
}

String C11RScriptLanguage::get_extension() const
{
    return "c11r";
}

Error C11RScriptLanguage::execute_file(const String &p_path)
{
    return OK;
}

void C11RScriptLanguage::finish(){}

void C11RScriptLanguage::get_reserved_words(List<String> *p_words) const{}

bool C11RScriptLanguage::is_control_flow_keyword(String p_string) const 
{
    return false;
}

void C11RScriptLanguage::get_comment_delimiters(List<String> *p_delimiters) const{}

void C11RScriptLanguage::get_string_delimiters(List<String> *p_delimiters) const{}

Ref<Script> C11RScriptLanguage::get_template(const String &p_class_name, const String &p_base_class_name) const
{
    Ref<C11RScript> script;
    script.instance();
    //TODO script->set_instance_base_type(p_base_class_name);
    return script;
}

void C11RScriptLanguage::make_template(const String &p_class_name, const String &p_base_class_name, Ref<Script> &p_script) 
{
    Ref<C11RScript> script = p_script;
    //TODO script->set_instance_base_type(p_base_class_name);

}

bool C11RScriptLanguage::is_using_templates() 
{ 
    return true; 
}

bool C11RScriptLanguage::validate(const String &p_script, int &r_line_error, int &r_col_error, String &r_test_error, const String &p_path = "", List<String> *r_functions = nullptr, List<Warning> *r_warnings = nullptr, Set<int> *r_safe_lines = nullptr) const
{
    return false;
}

String C11RScriptLanguage::validate_path(const String &p_path) const 
{ 
    return ""; // FIXME
}

Script *C11RScriptLanguage::create_script() const
{
    return memnew(C11RScript);
}

bool C11RScriptLanguage::has_named_classes() const
{
    return false;
}

bool C11RScriptLanguage::supports_builtin_mode() const
{
    return false;
}

bool C11RScriptLanguage::can_inherit_from_file() 
{ 
    return false; //FIXME
}

int C11RScriptLanguage::find_function(const String &p_function, const String &p_code) const
{
    return -1;
}

String C11RScriptLanguage::make_function(const String &p_class, const String &p_name, const PoolStringArray &p_args) const
{
    return String();
}

Error C11RScriptLanguage::open_in_external_editor(const Ref<Script> &p_script, int p_line, int p_col) 
{ 
    return ERR_UNAVAILABLE; //FIXME
}

bool C11RScriptLanguage::overrides_external_editor() { 
    return false; //FIXME
}

Error C11RScriptLanguage::complete_code(const String &p_code, const String &p_path, Object *p_owner, List<ScriptCodeCompletionOption> *r_options, bool &r_force, String &r_call_hint) 
{ 
    return ERR_UNAVAILABLE;  //FIXME
}

Error C11RScriptLanguage::lookup_code(const String &p_code, const String &p_symbol, const String &p_path, Object *p_owner, LookupResult &r_result) 
{ 
    return ERR_UNAVAILABLE; // FIXME
}

void C11RScriptLanguage::auto_indent_code(String &p_code, int p_from_line, int p_to_line) const {}

void C11RScriptLanguage::add_global_constant(const StringName &p_variable, const Variant &p_value) {}

void C11RScriptLanguage::add_named_global_constant(const StringName &p_name, const Variant &p_value) {} // FIXME

void C11RScriptLanguage::remove_named_global_constant(const StringName &p_name) {} // FIXME

/* MULTITHREAD FUNCTIONS */

//some VMs need to be notified of thread creation/exiting to allocate a stack
void C11RScriptLanguage::thread_enter() {} // FIXME remove
void C11RScriptLanguage::thread_exit() {} // FIXME remove

/* DEBUGGER FUNCTIONS */

String C11RScriptLanguage::debug_get_error() const
{
    return String();
} // FIXME

int C11RScriptLanguage::debug_get_stack_level_count() const
{
    return 0;
} // FIXME

int C11RScriptLanguage::debug_get_stack_level_line(int p_level) const
{
    return 0;
}//FIXME

String C11RScriptLanguage::debug_get_stack_level_function(int p_level) const
{
    return String();
} //FIXME

String C11RScriptLanguage::debug_get_stack_level_source(int p_level) const 
{
    return String();
} // FIXME

void C11RScriptLanguage::debug_get_stack_level_locals(int p_level, List<String> *p_locals, List<Variant> *p_values, int p_max_subitems = -1, int p_max_depth = -1) {} // FIXME

void C11RScriptLanguage::debug_get_stack_level_members(int p_level, List<String> *p_members, List<Variant> *p_values, int p_max_subitems = -1, int p_max_depth = -1) {} // FIXME

ScriptInstance C11RScriptLanguage::*debug_get_stack_level_instance(int p_level) 
{ 
    return nullptr; 
}

void C11RScriptLanguage::debug_get_globals(List<String> *p_globals, List<Variant> *p_values, int p_max_subitems = -1, int p_max_depth = -1){}

String C11RScriptLanguage::debug_parse_stack_level_expression(int p_level, const String &p_expression, int p_max_subitems = -1, int p_max_depth = -1)
{
    return String();
}

Vector<ScriptLanguage::StackInfo> C11RScriptLanguage::debug_get_current_stack_info() 
{ 
    return Vector<StackInfo>(); 
}

void C11RScriptLanguage::reload_all_scripts(){}

void C11RScriptLanguage::reload_tool_script(const Ref<Script> &p_script, bool p_soft_reload){}


/* LOADER FUNCTIONS */

void C11RScriptLanguage::get_recognized_extensions(List<String> *p_extensions) const{}

void C11RScriptLanguage::get_public_functions(List<MethodInfo> *p_functions) const{}

void C11RScriptLanguage::get_public_constants(List<Pair<String, Variant>> *p_constants) const {}

void C11RScriptLanguage::profiling_start(){}

void C11RScriptLanguage::profiling_stop(){}

int C11RScriptLanguage::profiling_get_accumulated_data(ScriptLanguage::ProfilingInfo *p_info_arr, int p_info_max)
{
    return 0;
}

int C11RScriptLanguage::profiling_get_frame_data(ScriptLanguage::ProfilingInfo *p_info_arr, int p_info_max){
    return 0;
}

void *C11RScriptLanguage::alloc_instance_binding_data(Object *p_object) { return nullptr; } //optional, not used by all languages

void C11RScriptLanguage::free_instance_binding_data(void *p_data) {} //optional, not used by all languages

void C11RScriptLanguage::refcount_incremented_instance_binding(Object *p_object) {} //optional, not used by all languages

bool C11RScriptLanguage::refcount_decremented_instance_binding(Object *p_object) 
{ 
    //return true if it can die //optional, not used by all languages
    return true;
} 

void C11RScriptLanguage::frame(){}

bool C11RScriptLanguage::handles_global_class_type(const String &p_type) const 
{ 
    return false; 
}

String C11RScriptLanguage::get_global_class_name(const String &p_path, String *r_base_type = nullptr, String *r_icon_path = nullptr) const 
{ 
    return String(); 
}

#pragma endregion