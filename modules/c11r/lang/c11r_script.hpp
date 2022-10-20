#ifndef C11R_SCRIPT_H
#define C11R_SCRIPT_H

#include "../c11r_lang.hpp"
#include "block.hpp"
#include "c11r_script_instance.hpp"
#include "c11r_script_language.hpp"

class C11RScript : public Script {
	GDCLASS(C11RScript, Script);

	RES_BASE_EXTENSION("c11r");

public:
	struct SequenceConnection {
		union {
			struct {
				uint64_t from_node : 24;
				uint64_t from_output : 16;
				uint64_t to_node : 24;
			};
			uint64_t id;
		};

		bool operator<(const SequenceConnection &p_connection) const {
			return id < p_connection.id;
		}
	};

	struct DataConnection {
		union {
			struct {
				uint64_t from_node : 24;
				uint64_t from_port : 8;
				uint64_t to_node : 24;
				uint64_t to_port : 8;
			};
			uint64_t id;
		};

		bool operator<(const DataConnection &p_connection) const {
			return id < p_connection.id;
		}
	};

private:
	friend class C11RScriptInstance;

	StringName base_type;
	struct Argument {
		String name;
		Variant::Type type;
	};

	struct Function {
		struct NodeData {
			Point2 pos;
			Ref<Block> node;
		};

		Map<int, NodeData> nodes;

		Set<SequenceConnection> sequence_connections;

		Set<DataConnection> data_connections;

		int function_id;

		Vector2 scroll;

		Function() { function_id = -1; }
	};

	struct Variable {
		Map<int, Ref<Block>> nodes;
		PropertyInfo info;
		Variant default_value;
		bool _export;
		// add getter & setter options here
	};

	Map<StringName, Function> functions;
	Map<StringName, Variable> variables;
	Map<StringName, Vector<Argument>> custom_signals;

	Map<Object *, C11RScriptInstance *> instances;

	bool is_tool_script;

#ifdef TOOLS_ENABLED
	Set<PlaceHolderScriptInstance *> placeholders;
	//void _update_placeholder(PlaceHolderScriptInstance *p_placeholder);
	virtual void _placeholder_erased(PlaceHolderScriptInstance *p_placeholder);
	void _update_placeholders();
#endif

	void _set_variable_info(const StringName &p_name, const Dictionary &p_info);
	Dictionary _get_variable_info(const StringName &p_name) const;

	void _set_data(const Dictionary &p_data);
	Dictionary _get_data() const;

protected:
	void _node_ports_changed(int p_id);
	static void _bind_methods();

public:
	bool is_sub_graph = false;

	bool inherits_script(const Ref<Script> &p_script) const;

	// TODO: Remove it in future when breaking changes are acceptable
	StringName get_default_func() const;
	void add_function(const StringName &p_name);
	bool has_function(const StringName &p_name) const;
	void remove_function(const StringName &p_name);
	void rename_function(const StringName &p_name, const StringName &p_new_name);
	void set_function_scroll(const StringName &p_name, const Vector2 &p_scroll);
	Vector2 get_function_scroll(const StringName &p_name) const;
	void get_function_list(List<StringName> *r_functions) const;
	int get_function_node_id(const StringName &p_name) const;
	void set_tool_enabled(bool p_enabled);

	void add_node(const StringName &p_func, int p_id, const Ref<Block> &p_node, const Point2 &p_pos = Point2());
	void remove_node(const StringName &p_func, int p_id);
	bool has_node(const StringName &p_func, int p_id) const;
	Ref<Block> get_node(const StringName &p_func, int p_id) const;
	void set_node_position(const StringName &p_func, int p_id, const Point2 &p_pos);
	Point2 get_node_position(const StringName &p_func, int p_id) const;
	void get_node_list(const StringName &p_func, List<int> *r_nodes) const;

	void sequence_connect(const StringName &p_func, int p_from_node, int p_from_output, int p_to_node);
	void sequence_disconnect(const StringName &p_func, int p_from_node, int p_from_output, int p_to_node);
	bool has_sequence_connection(const StringName &p_func, int p_from_node, int p_from_output, int p_to_node) const;
	void get_sequence_connection_list(const StringName &p_func, List<SequenceConnection> *r_connection) const;

	void data_connect(const StringName &p_func, int p_from_node, int p_from_port, int p_to_node, int p_to_port);
	void data_disconnect(const StringName &p_func, int p_from_node, int p_from_port, int p_to_node, int p_to_port);
	bool has_data_connection(const StringName &p_func, int p_from_node, int p_from_port, int p_to_node, int p_to_port) const;
	void get_data_connection_list(const StringName &p_func, List<DataConnection> *r_connection) const;
	bool is_input_value_port_connected(const StringName &p_func, int p_node, int p_port) const;
	bool get_input_value_port_connection_source(const StringName &p_func, int p_node, int p_port, int *r_node, int *r_port) const;

	void add_variable(const StringName &p_name, const Variant &p_default_value = Variant(), bool p_export = false);
	bool has_variable(const StringName &p_name) const;
	void remove_variable(const StringName &p_name);
	void set_variable_default_value(const StringName &p_name, const Variant &p_value);
	Variant get_variable_default_value(const StringName &p_name) const;
	void set_variable_info(const StringName &p_name, const PropertyInfo &p_info);
	PropertyInfo get_variable_info(const StringName &p_name) const;
	void set_variable_export(const StringName &p_name, bool p_export);
	bool get_variable_export(const StringName &p_name) const;
	void get_variable_list(List<StringName> *r_variables) const;
	void rename_variable(const StringName &p_name, const StringName &p_new_name);

	void add_custom_signal(const StringName &p_name);
	bool has_custom_signal(const StringName &p_name) const;
	void custom_signal_add_argument(const StringName &p_func, Variant::Type p_type, const String &p_name, int p_index = -1);
	void custom_signal_set_argument_type(const StringName &p_func, int p_argidx, Variant::Type p_type);
	Variant::Type custom_signal_get_argument_type(const StringName &p_func, int p_argidx) const;
	void custom_signal_set_argument_name(const StringName &p_func, int p_argidx, const String &p_name);
	String custom_signal_get_argument_name(const StringName &p_func, int p_argidx) const;
	void custom_signal_remove_argument(const StringName &p_func, int p_argidx);
	int custom_signal_get_argument_count(const StringName &p_func) const;
	void custom_signal_swap_argument(const StringName &p_func, int p_argidx, int p_with_argidx);
	void remove_custom_signal(const StringName &p_name);
	void rename_custom_signal(const StringName &p_name, const StringName &p_new_name);
	Set<int> get_output_sequence_ports_connected(const String &edited_func, int from_node);

	void get_custom_signal_list(List<StringName> *r_custom_signals) const;

	int get_available_id() const;

	void set_instance_base_type(const StringName &p_type);

	virtual bool can_instance() const;

	virtual Ref<Script> get_base_script() const;
	virtual StringName get_instance_base_type() const;
	virtual ScriptInstance *instance_create(Object *p_this);
	virtual bool instance_has(const Object *p_this) const;

	virtual bool has_source_code() const;
	virtual String get_source_code() const;
	virtual void set_source_code(const String &p_code);
	virtual Error reload(bool p_keep_state = false);

	virtual bool is_tool() const;
	virtual bool is_valid() const;

	virtual ScriptLanguage *get_language() const;

	virtual bool has_script_signal(const StringName &p_signal) const;
	virtual void get_script_signal_list(List<MethodInfo> *r_signals) const;

	virtual bool get_property_default_value(const StringName &p_property, Variant &r_value) const;
	virtual void get_script_method_list(List<MethodInfo> *p_list) const;

	virtual bool has_method(const StringName &p_method) const;
	virtual MethodInfo get_method_info(const StringName &p_method) const;

	virtual void get_script_property_list(List<PropertyInfo> *p_list) const;

	virtual int get_member_line(const StringName &p_member) const;

#ifdef TOOLS_ENABLED
	virtual bool are_subnodes_edited() const;
#endif

	C11RScript();
	~C11RScript();
};

#endif // C11R_SCRIPT_H