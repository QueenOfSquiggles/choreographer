#include "../c11r_lang.hpp"

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