#ifndef BLOCK_H
#define BLOCK_H


#include "../c11r_lang.hpp"

class Block : public Resource {
	GDCLASS(Block, Resource);

	friend class C11RScript;

	Set<C11RScript *> scripts_used;

	Array default_input_values;
	bool breakpoint;

	void _set_default_input_values(Array p_values);
	Array _get_default_input_values() const;

	void validate_input_default_values();

	void _set_block_namespace(const String &p_namespace);
	String _get_block_namespace();

protected:
	void ports_changed_notify();
	static void _bind_methods();

public:
	String block_namespace = "core";
	
	Ref<C11RScript> get_visual_script() const;

	virtual int get_output_sequence_port_count() const = 0;
	virtual bool has_input_sequence_port() const = 0;

	virtual String get_output_sequence_port_text(int p_port) const = 0;

	virtual bool has_mixed_input_and_sequence_ports() const { return false; }

	virtual int get_input_value_port_count() const = 0;
	virtual int get_output_value_port_count() const = 0;

	virtual PropertyInfo get_input_value_port_info(int p_idx) const = 0;
	virtual PropertyInfo get_output_value_port_info(int p_idx) const = 0;

	void set_default_input_value(int p_port, const Variant &p_value);
	Variant get_default_input_value(int p_port) const;

	virtual String get_caption() const = 0;
	virtual String get_text() const;
	virtual String get_category() const = 0;

	//used by editor, this is not really saved
	void set_breakpoint(bool p_breakpoint);
	bool is_breakpoint() const;

	virtual BlockInstance *instance(C11RScriptInstance *p_instance) = 0;

	struct TypeGuess {
		Variant::Type type;
		StringName gdclass;
		Ref<Script> script;

		TypeGuess() {
			type = Variant::NIL;
		}
	};

	virtual TypeGuess guess_output_type(TypeGuess *p_inputs, int p_output) const;

	Block();
};

#endif // BLOCK_H