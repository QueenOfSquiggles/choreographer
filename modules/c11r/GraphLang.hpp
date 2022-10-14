#pragma once

#include <core/class_db.h>
#include <scene/main/node.h>
#include <core/script_language.h>



class Block : public Resource
{
    GDCLASS(Block, Resource);

protected:
    static void _bind_methods();
    Block(){}
    ~Block(){}

};

class C11RScript : public Script
{
    GDCLASS(C11RScript, Script);

    RES_BASE_EXTENSION("c11r");
protected:
    static void _bind_methods();

public:

    String test_property;

    void test_method();

    C11RScript(){}
    ~C11RScript(){}
    // TODO implement virtual functions

    //  - - - - - - - - - - - - - - - -
    //  Script: virtual funcs
    //  - - - - - - - - - - - - - - - - 

    virtual bool can_instance() const;

	virtual Ref<Script> get_base_script() const; //for script inheritance

	virtual bool inherits_script(const Ref<Script> &p_script) const;

	virtual StringName get_instance_base_type() const; // this may not work in all scripts, will return empty if so
	
    virtual ScriptInstance *instance_create(Object *p_this);
    
    virtual bool instance_has(const Object *p_this) const;

	virtual bool has_source_code() const;
	
    virtual String get_source_code() const;
	
    virtual void set_source_code(const String &p_code);
	
    virtual Error reload(bool p_keep_state = false);

	virtual bool has_method(const StringName &p_method) const;
	
    virtual MethodInfo get_method_info(const StringName &p_method) const;

	virtual bool is_tool() const;
	
    virtual bool is_valid() const;

	virtual ScriptLanguage *get_language() const;

	virtual bool has_script_signal(const StringName &p_signal) const;
	
    virtual void get_script_signal_list(List<MethodInfo> *r_signals) const;

	virtual bool get_property_default_value(const StringName &p_property, Variant &r_value) const;
    
    virtual void get_script_method_list(List<MethodInfo> *p_list) const;
	
    virtual void get_script_property_list(List<PropertyInfo> *p_list) const;
};

class C11RScriptLanguage : public ScriptLanguage
{
public:

    static C11RScriptLanguage* singleton;
    
    C11RScriptLanguage();
    ~C11RScriptLanguage();

    //  - - - - - - - - - - - - - - - -
    //  ScriptLanguage: virtual funcs
    //  - - - - - - - - - - - - - - - - 
    virtual String get_name() const;

	/* LANGUAGE FUNCTIONS */
	virtual void init();
	virtual String get_type() const;
	virtual String get_extension() const;
	virtual Error execute_file(const String &p_path);
	virtual void finish();
    virtual void get_reserved_words(List<String> *p_words) const;
	virtual bool is_control_flow_keyword(String p_string) const;
	virtual void get_comment_delimiters(List<String> *p_delimiters) const;
	virtual void get_string_delimiters(List<String> *p_delimiters) const;
	virtual Ref<Script> get_template(const String &p_class_name, const String &p_base_class_name) const;
	virtual void make_template(const String &p_class_name, const String &p_base_class_name, Ref<Script> &p_script) {}
	virtual bool is_using_templates() { return false; }
	virtual bool validate(const String &p_script, int &r_line_error, int &r_col_error, String &r_test_error, const String &p_path = "", List<String> *r_functions = nullptr, List<Warning> *r_warnings = nullptr, Set<int> *r_safe_lines = nullptr) const;
	virtual String validate_path(const String &p_path) const { return ""; }
	virtual Script *create_script() const;
	virtual bool has_named_classes() const;
	virtual bool supports_builtin_mode() const;
	virtual bool can_inherit_from_file() { return false; }
	virtual int find_function(const String &p_function, const String &p_code) const;
	virtual String make_function(const String &p_class, const String &p_name, const PoolStringArray &p_args) const;
	virtual Error open_in_external_editor(const Ref<Script> &p_script, int p_line, int p_col) { return ERR_UNAVAILABLE; }
	virtual bool overrides_external_editor() { return false; }

	virtual Error complete_code(const String &p_code, const String &p_path, Object *p_owner, List<ScriptCodeCompletionOption> *r_options, bool &r_force, String &r_call_hint) { return ERR_UNAVAILABLE; }
    virtual Error lookup_code(const String &p_code, const String &p_symbol, const String &p_path, Object *p_owner, LookupResult &r_result) { return ERR_UNAVAILABLE; }

	virtual void auto_indent_code(String &p_code, int p_from_line, int p_to_line) const;
	virtual void add_global_constant(const StringName &p_variable, const Variant &p_value);
	virtual void add_named_global_constant(const StringName &p_name, const Variant &p_value) {}
	virtual void remove_named_global_constant(const StringName &p_name) {}

	/* MULTITHREAD FUNCTIONS */

	//some VMs need to be notified of thread creation/exiting to allocate a stack
	virtual void thread_enter() {}
	virtual void thread_exit() {}

	/* DEBUGGER FUNCTIONS */

	virtual String debug_get_error() const;
	virtual int debug_get_stack_level_count() const;
	virtual int debug_get_stack_level_line(int p_level) const;
	virtual String debug_get_stack_level_function(int p_level) const;
	virtual String debug_get_stack_level_source(int p_level) const;
	virtual void debug_get_stack_level_locals(int p_level, List<String> *p_locals, List<Variant> *p_values, int p_max_subitems = -1, int p_max_depth = -1);
	virtual void debug_get_stack_level_members(int p_level, List<String> *p_members, List<Variant> *p_values, int p_max_subitems = -1, int p_max_depth = -1);
	virtual ScriptInstance *debug_get_stack_level_instance(int p_level) { return nullptr; }
	virtual void debug_get_globals(List<String> *p_globals, List<Variant> *p_values, int p_max_subitems = -1, int p_max_depth = -1);
	virtual String debug_parse_stack_level_expression(int p_level, const String &p_expression, int p_max_subitems = -1, int p_max_depth = -1);
    virtual Vector<StackInfo> debug_get_current_stack_info() { return Vector<StackInfo>(); }

	virtual void reload_all_scripts();
	virtual void reload_tool_script(const Ref<Script> &p_script, bool p_soft_reload);
	/* LOADER FUNCTIONS */

	virtual void get_recognized_extensions(List<String> *p_extensions) const;
	virtual void get_public_functions(List<MethodInfo> *p_functions) const;
	virtual void get_public_constants(List<Pair<String, Variant>> *p_constants) const;

	struct ProfilingInfo {
		StringName signature;
		uint64_t call_count;
		uint64_t total_time;
		uint64_t self_time;
	};

	virtual void profiling_start();
	virtual void profiling_stop();

	virtual int profiling_get_accumulated_data(ScriptLanguage::ProfilingInfo *p_info_arr, int p_info_max) override;
	virtual int profiling_get_frame_data(ScriptLanguage::ProfilingInfo *p_info_arr, int p_info_max) override;

	virtual void *alloc_instance_binding_data(Object *p_object) { return nullptr; } //optional, not used by all languages
	virtual void free_instance_binding_data(void *p_data) {} //optional, not used by all languages
	virtual void refcount_incremented_instance_binding(Object *p_object) {} //optional, not used by all languages
	virtual bool refcount_decremented_instance_binding(Object *p_object) { return true; } //return true if it can die //optional, not used by all languages

	virtual void frame();

	virtual bool handles_global_class_type(const String &p_type) const { return false; }
	virtual String get_global_class_name(const String &p_path, String *r_base_type = nullptr, String *r_icon_path = nullptr) const { return String(); }
};