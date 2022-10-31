#ifndef C11R_LANG_H
#define C11R_LANG_H

#include "core/os/thread.h"
#include "core/script_language.h"
#include "core/io/config_file.h"

#define C11R_LANG_FILE_EXT "c11r"

class C11RScriptInstance;
class C11RScript;
class BlockInstance;

class Block : public Resource {
	GDCLASS(Block, Resource);

	friend class C11RScript;

	void _set_block_namespace(const String &p_namespace);
	String _get_block_namespace();

protected:
	static void _bind_methods();

public:
	String block_namespace = "core";

	Block();
};

class BlockInstance {
	friend class C11RScriptInstance;
	friend class C11RScriptLanguage; //for debugger


public:
	BlockInstance();
	~BlockInstance();
};

class C11RScript : public Script {
	GDCLASS(C11RScript, Script);

	RES_BASE_EXTENSION(C11R_LANG_FILE_EXT);

private:
	friend class C11RScriptInstance;

public:
	bool is_sub_graph = false;

	// resource format loading
	void load(Ref<ConfigFile> p_configuration);
	Ref<ConfigFile> save() const;

	// overrides
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

	virtual void update_exports() {} //editor tool
	virtual void get_script_method_list(List<MethodInfo> *p_list) const;
	virtual void get_script_property_list(List<PropertyInfo> *p_list) const;

	virtual int get_member_line(const StringName &p_member) const;

	virtual void get_constants(Map<StringName, Variant> *p_constants);
	virtual void get_members(Set<StringName> *p_constants);

	C11RScript();
	~C11RScript();
};

class C11RScriptInstance : public ScriptInstance {
	friend class C11RScriptLanguage; //for debugger
public:
	virtual bool set(const StringName &p_name, const Variant &p_value);
	virtual bool get(const StringName &p_name, Variant &r_ret) const;
	virtual void get_property_list(List<PropertyInfo> *p_properties) const;
	virtual Variant::Type get_property_type(const StringName &p_name, bool *r_is_valid = nullptr) const;

	virtual void get_method_list(List<MethodInfo> *p_list) const;
	virtual bool has_method(const StringName &p_method) const;
	virtual Variant call(const StringName &p_method, const Variant **p_args, int p_argcount, Variant::CallError &r_error);
	virtual void notification(int p_notification);
	String to_string(bool *r_valid);

	virtual Ref<Script> get_script() const;

	virtual ScriptLanguage *get_language();

	virtual MultiplayerAPI::RPCMode get_rpc_mode(const StringName &p_method) const;
	virtual MultiplayerAPI::RPCMode get_rset_mode(const StringName &p_variable) const;

	C11RScriptInstance();
	~C11RScriptInstance();
};

typedef Ref<Block> (*BlockRegisterFunc)(const String &p_type);

class C11RScriptLanguage : public ScriptLanguage {
	Map<String, BlockRegisterFunc> register_funcs;

public:
	static C11RScriptLanguage *singleton;
	Mutex lock;

	bool debug_break(const String &p_error, bool p_allow_continue = true);
	bool debug_break_parse(const String &p_file, int p_node, const String &p_error);

	void enter_function(C11RScriptInstance *p_instance, const StringName *p_function, Variant *p_stack, Variant **p_work_mem, int *current_id);
	void exit_function();

	//////////////////////////////////////

	virtual String get_name() const;

	/* LANGUAGE FUNCTIONS */
	virtual void init();
	virtual String get_type() const;
	virtual String get_extension() const;
	virtual Error execute_file(const String &p_path);
	virtual void finish();

	/* EDITOR FUNCTIONS */
	virtual void get_reserved_words(List<String> *p_words) const;
	virtual bool is_control_flow_keyword(String p_keyword) const;
	virtual void get_comment_delimiters(List<String> *p_delimiters) const;
	virtual void get_string_delimiters(List<String> *p_delimiters) const;
	virtual Ref<Script> get_template(const String &p_class_name, const String &p_base_class_name) const;
	virtual bool is_using_templates();
	virtual void make_template(const String &p_class_name, const String &p_base_class_name, Ref<Script> &p_script);
	virtual bool validate(const String &p_script, int &r_line_error, int &r_col_error, String &r_test_error, const String &p_path = "", List<String> *r_functions = nullptr, List<ScriptLanguage::Warning> *r_warnings = nullptr, Set<int> *r_safe_lines = nullptr) const;
	virtual Script *create_script() const;
	virtual bool has_named_classes() const;
	virtual bool supports_builtin_mode() const;
	virtual int find_function(const String &p_function, const String &p_code) const;
	virtual String make_function(const String &p_class, const String &p_name, const PoolStringArray &p_args) const;
	virtual void auto_indent_code(String &p_code, int p_from_line, int p_to_line) const;
	virtual void add_global_constant(const StringName &p_variable, const Variant &p_value);

	/* DEBUGGER FUNCTIONS */

	virtual String debug_get_error() const;
	virtual int debug_get_stack_level_count() const;
	virtual int debug_get_stack_level_line(int p_level) const;
	virtual String debug_get_stack_level_function(int p_level) const;
	virtual String debug_get_stack_level_source(int p_level) const;
	virtual void debug_get_stack_level_locals(int p_level, List<String> *p_locals, List<Variant> *p_values, int p_max_subitems = -1, int p_max_depth = -1);
	virtual void debug_get_stack_level_members(int p_level, List<String> *p_members, List<Variant> *p_values, int p_max_subitems = -1, int p_max_depth = -1);
	virtual void debug_get_globals(List<String> *p_locals, List<Variant> *p_values, int p_max_subitems = -1, int p_max_depth = -1);
	virtual String debug_parse_stack_level_expression(int p_level, const String &p_expression, int p_max_subitems = -1, int p_max_depth = -1);

	virtual void reload_all_scripts();
	virtual void reload_tool_script(const Ref<Script> &p_script, bool p_soft_reload);
	/* LOADER FUNCTIONS */

	virtual void get_recognized_extensions(List<String> *p_extensions) const;
	virtual void get_public_functions(List<MethodInfo> *p_functions) const;
	virtual void get_public_constants(List<Pair<String, Variant>> *p_constants) const;

	virtual void profiling_start();
	virtual void profiling_stop();

	virtual int profiling_get_accumulated_data(ProfilingInfo *p_info_arr, int p_info_max);
	virtual int profiling_get_frame_data(ProfilingInfo *p_info_arr, int p_info_max);

	void add_register_func(const String &p_name, BlockRegisterFunc p_func);
	void remove_register_func(const String &p_name);
	Ref<Block> create_node_from_name(const String &p_name);
	void get_registered_node_names(List<String> *r_names);

	C11RScriptLanguage();
	~C11RScriptLanguage();
};

//aid for registering
template <class T>
static Ref<Block> create_node_generic(const String &p_name) {
	Ref<T> node;
	node.instance();
	return node;
}

#endif // C11R_LANG_H