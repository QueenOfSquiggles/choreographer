#ifndef C11R_SCRIPT_LANGUAGE_H
#define C11R_SCRIPT_LANGUAGE_H


#include "../c11r_lang.hpp"
#include "c11r_script.hpp"
#include "block_instance.hpp"

class C11RScriptLanguage : public ScriptLanguage {
	Map<String, BlockRegisterFunc> register_funcs;

	struct CallLevel {
		Variant *stack;
		Variant **work_mem;
		const StringName *function;
		C11RScriptInstance *instance;
		int *current_id;
	};

	int _debug_parse_err_node;
	String _debug_parse_err_file;
	String _debug_error;
	int _debug_call_stack_pos;
	int _debug_max_call_stack;
	CallLevel *_call_stack;

public:
	StringName notification;
	StringName _get_output_port_unsequenced;
	StringName _step;
	StringName _subcall;

	static C11RScriptLanguage *singleton;

	Mutex lock;

	bool debug_break(const String &p_error, bool p_allow_continue = true);
	bool debug_break_parse(const String &p_file, int p_node, const String &p_error);

	_FORCE_INLINE_ void enter_function(C11RScriptInstance *p_instance, const StringName *p_function, Variant *p_stack, Variant **p_work_mem, int *current_id) {
		if (Thread::get_main_id() != Thread::get_caller_id()) {
			return; //no support for other threads than main for now
		}

		if (ScriptDebugger::get_singleton()->get_lines_left() > 0 && ScriptDebugger::get_singleton()->get_depth() >= 0) {
			ScriptDebugger::get_singleton()->set_depth(ScriptDebugger::get_singleton()->get_depth() + 1);
		}

		if (_debug_call_stack_pos >= _debug_max_call_stack) {
			//stack overflow
			_debug_error = "Stack Overflow (Stack Size: " + itos(_debug_max_call_stack) + ")";
			ScriptDebugger::get_singleton()->debug(this);
			return;
		}

		_call_stack[_debug_call_stack_pos].stack = p_stack;
		_call_stack[_debug_call_stack_pos].instance = p_instance;
		_call_stack[_debug_call_stack_pos].function = p_function;
		_call_stack[_debug_call_stack_pos].work_mem = p_work_mem;
		_call_stack[_debug_call_stack_pos].current_id = current_id;
		_debug_call_stack_pos++;
	}

	_FORCE_INLINE_ void exit_function() {
		if (Thread::get_main_id() != Thread::get_caller_id()) {
			return; //no support for other threads than main for now
		}

		if (ScriptDebugger::get_singleton()->get_lines_left() > 0 && ScriptDebugger::get_singleton()->get_depth() >= 0) {
			ScriptDebugger::get_singleton()->set_depth(ScriptDebugger::get_singleton()->get_depth() - 1);
		}

		if (_debug_call_stack_pos == 0) {
			_debug_error = "Stack Underflow (Engine Bug)";
			ScriptDebugger::get_singleton()->debug(this);
			return;
		}

		_debug_call_stack_pos--;
	}

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

#endif // C11R_SCRIPT_LANGUAGE_H