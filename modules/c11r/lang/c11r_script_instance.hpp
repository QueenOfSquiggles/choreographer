#ifndef C11R_SCRIPT_INSTANCE_H
#define C11R_SCRIPT_INSTANCE_H

#include "../c11r_lang.hpp"
#include "c11r_script.hpp"
#include "block_instance.hpp"
#include "c11r_script_function_state.hpp"

class C11RScriptInstance : public ScriptInstance {
	Object *owner;
	Ref<C11RScript> script;

	Map<StringName, Variant> variables; //using variable path, not script
	Map<int, BlockInstance *> instances;

	struct Function {
		int node;
		int max_stack;
		int trash_pos;
		int flow_stack_size;
		int pass_stack_size;
		int node_count;
		int argument_count;
	};

	Map<StringName, Function> functions;

	Vector<Variant> default_values;
	int max_input_args, max_output_args;

	StringName source;

	void _dependency_step(BlockInstance *node, int p_pass, int *pass_stack, const Variant **input_args, Variant **output_args, Variant *variant_stack, Variant::CallError &r_error, String &error_str, BlockInstance **r_error_node);
	Variant _call_internal(const StringName &p_method, void *p_stack, int p_stack_size, BlockInstance *p_node, int p_flow_stack_pos, int p_pass, bool p_resuming_yield, Variant::CallError &r_error);

	//Map<StringName,Function> functions;
	friend class C11RScriptFunctionState; //for yield
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

	bool set_variable(const StringName &p_variable, const Variant &p_value) {
		Map<StringName, Variant>::Element *E = variables.find(p_variable);
		if (!E) {
			return false;
		}

		E->get() = p_value;
		return true;
	}

	bool get_variable(const StringName &p_variable, Variant *r_variable) const {
		const Map<StringName, Variant>::Element *E = variables.find(p_variable);
		if (!E) {
			return false;
		}

		*r_variable = E->get();
		return true;
	}

	virtual Ref<Script> get_script() const;

	_FORCE_INLINE_ C11RScript *get_script_ptr() { return script.ptr(); }
	_FORCE_INLINE_ Object *get_owner_ptr() { return owner; }

	void create(const Ref<C11RScript> &p_script, Object *p_owner);

	virtual ScriptLanguage *get_language();

	virtual MultiplayerAPI::RPCMode get_rpc_mode(const StringName &p_method) const;
	virtual MultiplayerAPI::RPCMode get_rset_mode(const StringName &p_variable) const;

	C11RScriptInstance();
	~C11RScriptInstance();
};

#endif // C11R_SCRIPT_INSTANCE_H