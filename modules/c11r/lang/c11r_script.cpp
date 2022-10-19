#include "../c11r_lang.hpp"

#include "core/core_string_names.h"
#include "core/os/os.h"
#include "core/project_settings.h"
#include "scene/main/node.h"
#include "modules/visual_script/visual_script_func_nodes.h"
#include "modules/visual_script/visual_script_nodes.h"

void C11RScript::add_function(const StringName &p_name) {
	ERR_FAIL_COND(instances.size());
	ERR_FAIL_COND(!String(p_name).is_valid_identifier());
	ERR_FAIL_COND(functions.has(p_name));

	functions[p_name] = Function();
	functions[p_name].scroll = Vector2(-50, -100);
}

bool C11RScript::has_function(const StringName &p_name) const {
	return functions.has(p_name);
}
void C11RScript::remove_function(const StringName &p_name) {
	ERR_FAIL_COND(instances.size());
	ERR_FAIL_COND(!functions.has(p_name));

	for (Map<int, Function::NodeData>::Element *E = functions[p_name].nodes.front(); E; E = E->next()) {
		E->get().node->disconnect("ports_changed", this, "_node_ports_changed");
		E->get().node->scripts_used.erase(this);
	}

	functions.erase(p_name);
}

void C11RScript::rename_function(const StringName &p_name, const StringName &p_new_name) {
	ERR_FAIL_COND(instances.size());
	ERR_FAIL_COND(!functions.has(p_name));
	if (p_new_name == p_name) {
		return;
	}

	ERR_FAIL_COND(!String(p_new_name).is_valid_identifier());

	ERR_FAIL_COND(functions.has(p_new_name));
	ERR_FAIL_COND(variables.has(p_new_name));
	ERR_FAIL_COND(custom_signals.has(p_new_name));

	functions[p_new_name] = functions[p_name];
	functions.erase(p_name);
}

void C11RScript::set_function_scroll(const StringName &p_name, const Vector2 &p_scroll) {
	ERR_FAIL_COND(!functions.has(p_name));
	functions[p_name].scroll = p_scroll;
}

Vector2 C11RScript::get_function_scroll(const StringName &p_name) const {
	ERR_FAIL_COND_V(!functions.has(p_name), Vector2());
	return functions[p_name].scroll;
}

void C11RScript::get_function_list(List<StringName> *r_functions) const {
	for (const Map<StringName, Function>::Element *E = functions.front(); E; E = E->next()) {
		r_functions->push_back(E->key());
	}

	r_functions->sort_custom<StringName::AlphCompare>();
}

int C11RScript::get_function_node_id(const StringName &p_name) const {
	ERR_FAIL_COND_V(!functions.has(p_name), -1);

	return functions[p_name].function_id;
}

void C11RScript::_node_ports_changed(int p_id) {
	StringName function;

	for (Map<StringName, Function>::Element *E = functions.front(); E; E = E->next()) {
		if (E->get().nodes.has(p_id)) {
			function = E->key();
			break;
		}
	}

	ERR_FAIL_COND(function == StringName());

	Function &func = functions[function];
	Ref<Block> vsn = func.nodes[p_id].node;

	vsn->validate_input_default_values();

	//must revalidate all the functions

	{
		List<SequenceConnection> to_remove;

		for (Set<SequenceConnection>::Element *E = func.sequence_connections.front(); E; E = E->next()) {
			if (E->get().from_node == p_id && E->get().from_output >= vsn->get_output_sequence_port_count()) {
				to_remove.push_back(E->get());
			}
			if (E->get().to_node == p_id && !vsn->has_input_sequence_port()) {
				to_remove.push_back(E->get());
			}
		}

		while (to_remove.size()) {
			func.sequence_connections.erase(to_remove.front()->get());
			to_remove.pop_front();
		}
	}

	{
		List<DataConnection> to_remove;

		for (Set<DataConnection>::Element *E = func.data_connections.front(); E; E = E->next()) {
			if (E->get().from_node == p_id && E->get().from_port >= vsn->get_output_value_port_count()) {
				to_remove.push_back(E->get());
			}
			if (E->get().to_node == p_id && E->get().to_port >= vsn->get_input_value_port_count()) {
				to_remove.push_back(E->get());
			}
		}

		while (to_remove.size()) {
			func.data_connections.erase(to_remove.front()->get());
			to_remove.pop_front();
		}
	}

#ifdef TOOLS_ENABLED
	set_edited(true); //something changed, let's set as edited
	emit_signal("node_ports_changed", function, p_id);
#endif
}

void C11RScript::add_node(const StringName &p_name, int p_id, const Ref<Block> &p_node, const Point2 &p_pos) {
	ERR_FAIL_COND(instances.size());
	ERR_FAIL_COND(p_node.is_null());

	StringName var_name;
	if (Object::cast_to<VisualScriptVariableGet>(*p_node)) {
		Ref<VisualScriptVariableGet> vget = p_node;
		var_name = vget->get_variable();
	} else if (Object::cast_to<VisualScriptVariableSet>(*p_node)) {
		Ref<VisualScriptVariableSet> vset = p_node;
		var_name = vset->get_variable();
	}
	if (variables.has(var_name)) {
		for (Map<StringName, Variable>::Element *E = variables.front(); E; E = E->next()) {
			ERR_FAIL_COND(E->get().nodes.has(p_id)); // Only unique ids can exist in a script, even for different functions
		}

		Variable &var = variables[var_name];
		var.nodes[p_id] = p_node;
	}

	if (functions.has(p_name)) {
		for (Map<StringName, Function>::Element *E = functions.front(); E; E = E->next()) {
			ERR_FAIL_COND(E->get().nodes.has(p_id)); // Only unique id can exist in a script, even for different functions
		}

		Function &func = functions[p_name];
		if (Object::cast_to<VisualScriptFunction>(*p_node)) {
			// The function indeed
			ERR_FAIL_COND_MSG(func.function_id >= 0, "A function node has already been set here.");

			func.function_id = p_id;
		}

		Function::NodeData nd;
		nd.node = p_node;
		nd.pos = p_pos;
		func.nodes[p_id] = nd;
	}

	Ref<Block> vsn = p_node;
	vsn->connect("ports_changed", this, "_node_ports_changed", varray(p_id));
	vsn->scripts_used.insert(this);
	vsn->validate_input_default_values(); // Validate when fully loaded
}

void C11RScript::remove_node(const StringName &p_name, int p_id) {
	ERR_FAIL_COND(instances.size());

	if (functions.has(p_name)) {
		Function &func = functions[p_name];
		if (func.nodes.has(p_id)) {
			{
				List<SequenceConnection> to_remove;

				for (Set<SequenceConnection>::Element *E = func.sequence_connections.front(); E; E = E->next()) {
					if (E->get().from_node == p_id || E->get().to_node == p_id) {
						to_remove.push_back(E->get());
					}
				}

				while (to_remove.size()) {
					func.sequence_connections.erase(to_remove.front()->get());
					to_remove.pop_front();
				}
			}

			{
				List<DataConnection> to_remove;

				for (Set<DataConnection>::Element *E = func.data_connections.front(); E; E = E->next()) {
					if (E->get().from_node == p_id || E->get().to_node == p_id) {
						to_remove.push_back(E->get());
					}
				}

				while (to_remove.size()) {
					func.data_connections.erase(to_remove.front()->get());
					to_remove.pop_front();
				}
			}

			if (Object::cast_to<VisualScriptFunction>(func.nodes[p_id].node.ptr())) {
				func.function_id = -1; // Revert to invalid
			}

			{
				StringName var_name;
				if (Object::cast_to<VisualScriptVariableGet>(*func.nodes[p_id].node)) {
					Ref<VisualScriptVariableGet> vget = func.nodes[p_id].node;
					var_name = vget->get_variable();
				} else if (Object::cast_to<VisualScriptVariableSet>(*func.nodes[p_id].node)) {
					Ref<VisualScriptVariableSet> vset = func.nodes[p_id].node;
					var_name = vset->get_variable();
				}

				if (variables.has(var_name)) {
					variables[var_name].nodes.erase(p_id);
				}
			}

			func.nodes[p_id].node->disconnect("ports_changed", this, "_node_ports_changed");
			func.nodes[p_id].node->scripts_used.erase(this);

			func.nodes.erase(p_id);
		}
	}
}

bool C11RScript::has_node(const StringName &p_func, int p_id) const {
	ERR_FAIL_COND_V(!functions.has(p_func), false);
	const Function &func = functions[p_func];

	return func.nodes.has(p_id);
}

Ref<Block> C11RScript::get_node(const StringName &p_name, int p_id) const {
	if (functions.has(p_name)) {
		const Function &func = functions[p_name];
		if (func.nodes.has(p_id)) {
			return func.nodes[p_id].node;
		}
	}

	if (variables.has(p_name)) {
		const Variable &var = variables[p_name];
		if (var.nodes.has(p_id)) {
			return var.nodes[p_id];
		}
	}

	return Ref<Block>();
}

void C11RScript::set_node_position(const StringName &p_func, int p_id, const Point2 &p_pos) {
	ERR_FAIL_COND(instances.size());
	ERR_FAIL_COND(!functions.has(p_func));
	Function &func = functions[p_func];

	ERR_FAIL_COND(!func.nodes.has(p_id));
	func.nodes[p_id].pos = p_pos;
}

Point2 C11RScript::get_node_position(const StringName &p_func, int p_id) const {
	ERR_FAIL_COND_V(!functions.has(p_func), Point2());
	const Function &func = functions[p_func];

	ERR_FAIL_COND_V(!func.nodes.has(p_id), Point2());
	return func.nodes[p_id].pos;
}

void C11RScript::get_node_list(const StringName &p_func, List<int> *r_nodes) const {
	if (functions.has(p_func)) {
		const Function &func = functions[p_func];
		for (const Map<int, Function::NodeData>::Element *E = func.nodes.front(); E; E = E->next()) {
			r_nodes->push_back(E->key());
		}
	}

	if (variables.has(p_func)) {
		const Variable &var = variables[p_func];
		for (const Map<int, Ref<Block>>::Element *E = var.nodes.front(); E; E = E->next()) {
			r_nodes->push_back(E->key());
		}
	}
}

void C11RScript::sequence_connect(const StringName &p_func, int p_from_node, int p_from_output, int p_to_node) {
	ERR_FAIL_COND(instances.size());
	ERR_FAIL_COND(!functions.has(p_func));
	Function &func = functions[p_func];

	SequenceConnection sc;
	sc.from_node = p_from_node;
	sc.from_output = p_from_output;
	sc.to_node = p_to_node;
	ERR_FAIL_COND(func.sequence_connections.has(sc));

	func.sequence_connections.insert(sc);
}

void C11RScript::sequence_disconnect(const StringName &p_func, int p_from_node, int p_from_output, int p_to_node) {
	ERR_FAIL_COND(!functions.has(p_func));
	Function &func = functions[p_func];

	SequenceConnection sc;
	sc.from_node = p_from_node;
	sc.from_output = p_from_output;
	sc.to_node = p_to_node;
	ERR_FAIL_COND(!func.sequence_connections.has(sc));

	func.sequence_connections.erase(sc);
}

bool C11RScript::has_sequence_connection(const StringName &p_func, int p_from_node, int p_from_output, int p_to_node) const {
	ERR_FAIL_COND_V(!functions.has(p_func), false);
	const Function &func = functions[p_func];

	SequenceConnection sc;
	sc.from_node = p_from_node;
	sc.from_output = p_from_output;
	sc.to_node = p_to_node;

	return func.sequence_connections.has(sc);
}

void C11RScript::get_sequence_connection_list(const StringName &p_func, List<SequenceConnection> *r_connection) const {
	ERR_FAIL_COND(!functions.has(p_func));
	const Function &func = functions[p_func];

	for (const Set<SequenceConnection>::Element *E = func.sequence_connections.front(); E; E = E->next()) {
		r_connection->push_back(E->get());
	}
}

void C11RScript::data_connect(const StringName &p_func, int p_from_node, int p_from_port, int p_to_node, int p_to_port) {
	ERR_FAIL_COND(instances.size());
	ERR_FAIL_COND(!functions.has(p_func));
	Function &func = functions[p_func];

	DataConnection dc;
	dc.from_node = p_from_node;
	dc.from_port = p_from_port;
	dc.to_node = p_to_node;
	dc.to_port = p_to_port;

	ERR_FAIL_COND(func.data_connections.has(dc));

	func.data_connections.insert(dc);
}

void C11RScript::data_disconnect(const StringName &p_func, int p_from_node, int p_from_port, int p_to_node, int p_to_port) {
	ERR_FAIL_COND(!functions.has(p_func));
	Function &func = functions[p_func];

	DataConnection dc;
	dc.from_node = p_from_node;
	dc.from_port = p_from_port;
	dc.to_node = p_to_node;
	dc.to_port = p_to_port;

	ERR_FAIL_COND(!func.data_connections.has(dc));

	func.data_connections.erase(dc);
}

bool C11RScript::has_data_connection(const StringName &p_func, int p_from_node, int p_from_port, int p_to_node, int p_to_port) const {
	ERR_FAIL_COND_V(!functions.has(p_func), false);
	const Function &func = functions[p_func];

	DataConnection dc;
	dc.from_node = p_from_node;
	dc.from_port = p_from_port;
	dc.to_node = p_to_node;
	dc.to_port = p_to_port;

	return func.data_connections.has(dc);
}

bool C11RScript::is_input_value_port_connected(const StringName &p_func, int p_node, int p_port) const {
	ERR_FAIL_COND_V(!functions.has(p_func), false);
	const Function &func = functions[p_func];

	for (const Set<DataConnection>::Element *E = func.data_connections.front(); E; E = E->next()) {
		if (E->get().to_node == p_node && E->get().to_port == p_port) {
			return true;
		}
	}

	return false;
}

bool C11RScript::get_input_value_port_connection_source(const StringName &p_func, int p_node, int p_port, int *r_node, int *r_port) const {
	ERR_FAIL_COND_V(!functions.has(p_func), false);
	const Function &func = functions[p_func];

	for (const Set<DataConnection>::Element *E = func.data_connections.front(); E; E = E->next()) {
		if (E->get().to_node == p_node && E->get().to_port == p_port) {
			*r_node = E->get().from_node;
			*r_port = E->get().from_port;
			return true;
		}
	}

	return false;
}

void C11RScript::get_data_connection_list(const StringName &p_func, List<DataConnection> *r_connection) const {
	ERR_FAIL_COND(!functions.has(p_func));
	const Function &func = functions[p_func];

	for (const Set<DataConnection>::Element *E = func.data_connections.front(); E; E = E->next()) {
		r_connection->push_back(E->get());
	}
}

void C11RScript::set_tool_enabled(bool p_enabled) {
	is_tool_script = p_enabled;
}

void C11RScript::add_variable(const StringName &p_name, const Variant &p_default_value, bool p_export) {
	ERR_FAIL_COND(instances.size());
	ERR_FAIL_COND(!String(p_name).is_valid_identifier());
	ERR_FAIL_COND(variables.has(p_name));

	Variable v;
	v.default_value = p_default_value;
	v.info.type = p_default_value.get_type();
	v.info.name = p_name;
	v.info.hint = PROPERTY_HINT_NONE;
	v._export = p_export;

	variables[p_name] = v;

#ifdef TOOLS_ENABLED
	_update_placeholders();
#endif
}

bool C11RScript::has_variable(const StringName &p_name) const {
	return variables.has(p_name);
}

void C11RScript::remove_variable(const StringName &p_name) {
	ERR_FAIL_COND(!variables.has(p_name));
	variables.erase(p_name);

#ifdef TOOLS_ENABLED
	_update_placeholders();
#endif
}

void C11RScript::set_variable_default_value(const StringName &p_name, const Variant &p_value) {
	ERR_FAIL_COND(!variables.has(p_name));

	variables[p_name].default_value = p_value;

#ifdef TOOLS_ENABLED
	_update_placeholders();
#endif
}
Variant C11RScript::get_variable_default_value(const StringName &p_name) const {
	ERR_FAIL_COND_V(!variables.has(p_name), Variant());
	return variables[p_name].default_value;
}
void C11RScript::set_variable_info(const StringName &p_name, const PropertyInfo &p_info) {
	ERR_FAIL_COND(instances.size());
	ERR_FAIL_COND(!variables.has(p_name));
	variables[p_name].info = p_info;
	variables[p_name].info.name = p_name;

#ifdef TOOLS_ENABLED
	_update_placeholders();
#endif
}
PropertyInfo C11RScript::get_variable_info(const StringName &p_name) const {
	ERR_FAIL_COND_V(!variables.has(p_name), PropertyInfo());
	return variables[p_name].info;
}

void C11RScript::set_variable_export(const StringName &p_name, bool p_export) {
	ERR_FAIL_COND(!variables.has(p_name));

	variables[p_name]._export = p_export;
#ifdef TOOLS_ENABLED
	_update_placeholders();
#endif
}

bool C11RScript::get_variable_export(const StringName &p_name) const {
	ERR_FAIL_COND_V(!variables.has(p_name), false);
	return variables[p_name]._export;
}

void C11RScript::_set_variable_info(const StringName &p_name, const Dictionary &p_info) {
	PropertyInfo pinfo;
	if (p_info.has("type")) {
		pinfo.type = Variant::Type(int(p_info["type"]));
	}
	if (p_info.has("name")) {
		pinfo.name = p_info["name"];
	}
	if (p_info.has("hint")) {
		pinfo.hint = PropertyHint(int(p_info["hint"]));
	}
	if (p_info.has("hint_string")) {
		pinfo.hint_string = p_info["hint_string"];
	}
	if (p_info.has("usage")) {
		pinfo.usage = p_info["usage"];
	}

	set_variable_info(p_name, pinfo);
}

Dictionary C11RScript::_get_variable_info(const StringName &p_name) const {
	PropertyInfo pinfo = get_variable_info(p_name);
	Dictionary d;
	d["type"] = pinfo.type;
	d["name"] = pinfo.name;
	d["hint"] = pinfo.hint;
	d["hint_string"] = pinfo.hint_string;
	d["usage"] = pinfo.usage;

	return d;
}

void C11RScript::get_variable_list(List<StringName> *r_variables) const {
	for (Map<StringName, Variable>::Element *E = variables.front(); E; E = E->next()) {
		r_variables->push_back(E->key());
	}

	r_variables->sort_custom<StringName::AlphCompare>();
}

void C11RScript::set_instance_base_type(const StringName &p_type) {
	ERR_FAIL_COND(instances.size());
	base_type = p_type;
}

void C11RScript::rename_variable(const StringName &p_name, const StringName &p_new_name) {
	ERR_FAIL_COND(instances.size());
	ERR_FAIL_COND(!variables.has(p_name));
	if (p_new_name == p_name) {
		return;
	}

	ERR_FAIL_COND(!String(p_new_name).is_valid_identifier());

	ERR_FAIL_COND(functions.has(p_new_name));
	ERR_FAIL_COND(variables.has(p_new_name));
	ERR_FAIL_COND(custom_signals.has(p_new_name));

	variables[p_new_name] = variables[p_name];
	variables.erase(p_name);
}

void C11RScript::add_custom_signal(const StringName &p_name) {
	ERR_FAIL_COND(instances.size());
	ERR_FAIL_COND(!String(p_name).is_valid_identifier());
	ERR_FAIL_COND(custom_signals.has(p_name));

	custom_signals[p_name] = Vector<Argument>();
}

bool C11RScript::has_custom_signal(const StringName &p_name) const {
	return custom_signals.has(p_name);
}
void C11RScript::custom_signal_add_argument(const StringName &p_func, Variant::Type p_type, const String &p_name, int p_index) {
	ERR_FAIL_COND(instances.size());
	ERR_FAIL_COND(!custom_signals.has(p_func));
	Argument arg;
	arg.type = p_type;
	arg.name = p_name;
	if (p_index < 0) {
		custom_signals[p_func].push_back(arg);
	} else {
		custom_signals[p_func].insert(0, arg);
	}
}
void C11RScript::custom_signal_set_argument_type(const StringName &p_func, int p_argidx, Variant::Type p_type) {
	ERR_FAIL_COND(instances.size());
	ERR_FAIL_COND(!custom_signals.has(p_func));
	ERR_FAIL_INDEX(p_argidx, custom_signals[p_func].size());
	custom_signals[p_func].write[p_argidx].type = p_type;
}
Variant::Type C11RScript::custom_signal_get_argument_type(const StringName &p_func, int p_argidx) const {
	ERR_FAIL_COND_V(!custom_signals.has(p_func), Variant::NIL);
	ERR_FAIL_INDEX_V(p_argidx, custom_signals[p_func].size(), Variant::NIL);
	return custom_signals[p_func][p_argidx].type;
}
void C11RScript::custom_signal_set_argument_name(const StringName &p_func, int p_argidx, const String &p_name) {
	ERR_FAIL_COND(instances.size());
	ERR_FAIL_COND(!custom_signals.has(p_func));
	ERR_FAIL_INDEX(p_argidx, custom_signals[p_func].size());
	custom_signals[p_func].write[p_argidx].name = p_name;
}
String C11RScript::custom_signal_get_argument_name(const StringName &p_func, int p_argidx) const {
	ERR_FAIL_COND_V(!custom_signals.has(p_func), String());
	ERR_FAIL_INDEX_V(p_argidx, custom_signals[p_func].size(), String());
	return custom_signals[p_func][p_argidx].name;
}
void C11RScript::custom_signal_remove_argument(const StringName &p_func, int p_argidx) {
	ERR_FAIL_COND(instances.size());
	ERR_FAIL_COND(!custom_signals.has(p_func));
	ERR_FAIL_INDEX(p_argidx, custom_signals[p_func].size());
	custom_signals[p_func].remove(p_argidx);
}

int C11RScript::custom_signal_get_argument_count(const StringName &p_func) const {
	ERR_FAIL_COND_V(!custom_signals.has(p_func), 0);
	return custom_signals[p_func].size();
}
void C11RScript::custom_signal_swap_argument(const StringName &p_func, int p_argidx, int p_with_argidx) {
	ERR_FAIL_COND(instances.size());
	ERR_FAIL_COND(!custom_signals.has(p_func));
	ERR_FAIL_INDEX(p_argidx, custom_signals[p_func].size());
	ERR_FAIL_INDEX(p_with_argidx, custom_signals[p_func].size());

	SWAP(custom_signals[p_func].write[p_argidx], custom_signals[p_func].write[p_with_argidx]);
}
void C11RScript::remove_custom_signal(const StringName &p_name) {
	ERR_FAIL_COND(instances.size());
	ERR_FAIL_COND(!custom_signals.has(p_name));
	custom_signals.erase(p_name);
}

void C11RScript::rename_custom_signal(const StringName &p_name, const StringName &p_new_name) {
	ERR_FAIL_COND(instances.size());
	ERR_FAIL_COND(!custom_signals.has(p_name));
	if (p_new_name == p_name) {
		return;
	}

	ERR_FAIL_COND(!String(p_new_name).is_valid_identifier());

	ERR_FAIL_COND(functions.has(p_new_name));
	ERR_FAIL_COND(variables.has(p_new_name));
	ERR_FAIL_COND(custom_signals.has(p_new_name));

	custom_signals[p_new_name] = custom_signals[p_name];
	custom_signals.erase(p_name);
}

void C11RScript::get_custom_signal_list(List<StringName> *r_custom_signals) const {
	for (const Map<StringName, Vector<Argument>>::Element *E = custom_signals.front(); E; E = E->next()) {
		r_custom_signals->push_back(E->key());
	}

	r_custom_signals->sort_custom<StringName::AlphCompare>();
}

int C11RScript::get_available_id() const {
	int max_id = 0;
	for (Map<StringName, Function>::Element *E = functions.front(); E; E = E->next()) {
		if (E->get().nodes.empty()) {
			continue;
		}

		int last_id = E->get().nodes.back()->key();
		max_id = MAX(max_id, last_id + 1);
	}

	return max_id;
}

/////////////////////////////////

bool C11RScript::can_instance() const {
	return true; //ScriptServer::is_scripting_enabled();
}

StringName C11RScript::get_instance_base_type() const {
	return base_type;
}

Ref<Script> C11RScript::get_base_script() const {
	return Ref<Script>(); // no inheritance in visual script
}

#ifdef TOOLS_ENABLED
void C11RScript::_placeholder_erased(PlaceHolderScriptInstance *p_placeholder) {
	placeholders.erase(p_placeholder);
}

void C11RScript::_update_placeholders() {
	if (placeholders.size() == 0) {
		return; //no bother if no placeholders
	}
	List<PropertyInfo> pinfo;
	Map<StringName, Variant> values;

	for (Map<StringName, Variable>::Element *E = variables.front(); E; E = E->next()) {
		if (!E->get()._export) {
			continue;
		}

		PropertyInfo p = E->get().info;
		p.name = String(E->key());
		pinfo.push_back(p);
		values[p.name] = E->get().default_value;
	}

	for (Set<PlaceHolderScriptInstance *>::Element *E = placeholders.front(); E; E = E->next()) {
		E->get()->update(pinfo, values);
	}
}

#endif

ScriptInstance *C11RScript::instance_create(Object *p_this) {
#ifdef TOOLS_ENABLED

	if (!ScriptServer::is_scripting_enabled() && !is_tool_script) {
		PlaceHolderScriptInstance *sins = memnew(PlaceHolderScriptInstance(C11RScriptLanguage::singleton, Ref<Script>((Script *)this), p_this));
		placeholders.insert(sins);

		List<PropertyInfo> pinfo;
		Map<StringName, Variant> values;

		for (Map<StringName, Variable>::Element *E = variables.front(); E; E = E->next()) {
			if (!E->get()._export) {
				continue;
			}

			PropertyInfo p = E->get().info;
			p.name = String(E->key());
			pinfo.push_back(p);
			values[p.name] = E->get().default_value;
		}

		sins->update(pinfo, values);

		return sins;
	}
#endif

	C11RScriptInstance *instance = memnew(C11RScriptInstance);
	instance->create(Ref<C11RScript>(this), p_this);

	C11RScriptLanguage::singleton->lock.lock();
	instances[p_this] = instance;
	C11RScriptLanguage::singleton->lock.unlock();

	return instance;
}

bool C11RScript::instance_has(const Object *p_this) const {
	return instances.has((Object *)p_this);
}

bool C11RScript::has_source_code() const {
	return false;
}

String C11RScript::get_source_code() const {
	return String();
}

void C11RScript::set_source_code(const String &p_code) {
}

Error C11RScript::reload(bool p_keep_state) {
	return OK;
}

bool C11RScript::is_tool() const {
	return is_tool_script;
}

bool C11RScript::is_valid() const {
	return true; //always valid
}

ScriptLanguage *C11RScript::get_language() const {
	return C11RScriptLanguage::singleton;
}

bool C11RScript::has_script_signal(const StringName &p_signal) const {
	return custom_signals.has(p_signal);
}

void C11RScript::get_script_signal_list(List<MethodInfo> *r_signals) const {
	for (const Map<StringName, Vector<Argument>>::Element *E = custom_signals.front(); E; E = E->next()) {
		MethodInfo mi;
		mi.name = E->key();
		for (int i = 0; i < E->get().size(); i++) {
			PropertyInfo arg;
			arg.type = E->get()[i].type;
			arg.name = E->get()[i].name;
			mi.arguments.push_back(arg);
		}

		r_signals->push_back(mi);
	}
}

bool C11RScript::get_property_default_value(const StringName &p_property, Variant &r_value) const {
	if (!variables.has(p_property)) {
		return false;
	}

	r_value = variables[p_property].default_value;
	return true;
}
void C11RScript::get_script_method_list(List<MethodInfo> *p_list) const {
	for (Map<StringName, Function>::Element *E = functions.front(); E; E = E->next()) {
		MethodInfo mi;
		mi.name = E->key();
		if (E->get().function_id >= 0) {
			Ref<VisualScriptFunction> func = E->get().nodes[E->get().function_id].node;
			if (func.is_valid()) {
				for (int i = 0; i < func->get_argument_count(); i++) {
					PropertyInfo arg;
					arg.name = func->get_argument_name(i);
					arg.type = func->get_argument_type(i);
					mi.arguments.push_back(arg);
				}

				p_list->push_back(mi);
			}
		}
	}
}

bool C11RScript::has_method(const StringName &p_method) const {
	return functions.has(p_method);
}
MethodInfo C11RScript::get_method_info(const StringName &p_method) const {
	const Map<StringName, Function>::Element *E = functions.find(p_method);
	if (!E) {
		return MethodInfo();
	}

	MethodInfo mi;
	mi.name = E->key();
	if (E->get().function_id >= 0) {
		Ref<VisualScriptFunction> func = E->get().nodes[E->get().function_id].node;
		if (func.is_valid()) {
			for (int i = 0; i < func->get_argument_count(); i++) {
				PropertyInfo arg;
				arg.name = func->get_argument_name(i);
				arg.type = func->get_argument_type(i);
				mi.arguments.push_back(arg);
			}

			if (!func->is_sequenced()) {
				mi.flags |= METHOD_FLAG_CONST;
			}
		}
	}

	return mi;
}

void C11RScript::get_script_property_list(List<PropertyInfo> *p_list) const {
	List<StringName> vars;
	get_variable_list(&vars);

	for (List<StringName>::Element *E = vars.front(); E; E = E->next()) {
		//if (!variables[E->get()]._export)
		//	continue;
		PropertyInfo pi = variables[E->get()].info;
		pi.usage |= PROPERTY_USAGE_SCRIPT_VARIABLE;
		p_list->push_back(pi);
	}
}

int C11RScript::get_member_line(const StringName &p_member) const {
#ifdef TOOLS_ENABLED
	if (has_function(p_member)) {
		for (Map<int, Function::NodeData>::Element *E = functions[p_member].nodes.front(); E; E = E->next()) {
			if (Object::cast_to<VisualScriptFunction>(E->get().node.ptr())) {
				return E->key();
			}
		}
	}
#endif
	return -1;
}

#ifdef TOOLS_ENABLED
bool C11RScript::are_subnodes_edited() const {
	for (const Map<StringName, Function>::Element *E = functions.front(); E; E = E->next()) {
		for (const Map<int, Function::NodeData>::Element *F = E->get().nodes.front(); F; F = F->next()) {
			if (F->get().node->is_edited()) {
				return true;
			}
		}
	}

	return false;
}
#endif

void C11RScript::_set_data(const Dictionary &p_data) {
	Dictionary d = p_data;
	if (d.has("base_type")) {
		base_type = d["base_type"];
	}

	variables.clear();
	Array vars = d["variables"];
	for (int i = 0; i < vars.size(); i++) {
		Dictionary v = vars[i];
		StringName name = v["name"];
		add_variable(name);
		_set_variable_info(name, v);
		set_variable_default_value(name, v["default_value"]);
		set_variable_export(name, v.has("export") && bool(v["export"]));
	}

	custom_signals.clear();
	Array sigs = d["signals"];
	for (int i = 0; i < sigs.size(); i++) {
		Dictionary cs = sigs[i];
		add_custom_signal(cs["name"]);

		Array args = cs["arguments"];
		for (int j = 0; j < args.size(); j += 2) {
			custom_signal_add_argument(cs["name"], Variant::Type(int(args[j + 1])), args[j]);
		}
	}

	Array funcs = d["functions"];
	functions.clear();

	Vector2 last_pos = Vector2(-100 * funcs.size(), -100 * funcs.size()); // this is the center of the last fn box
	Vector2 last_size = Vector2(0.0, 0.0);

	for (int i = 0; i < funcs.size(); i++) {
		Dictionary func = funcs[i];

		StringName name = func["name"];
		//int id=func["function_id"];
		add_function(name);

		set_function_scroll(name, func["scroll"]);

		Array nodes = func["nodes"];

		if (!d.has("vs_unify") && nodes.size() > 0) {
			Vector2 top_left = nodes[1];
			Vector2 bottom_right = nodes[1];

			for (int j = 0; j < nodes.size(); j += 3) {
				Point2 pos = nodes[j + 1];
				if (pos.y > top_left.y) {
					top_left.y = pos.y;
				}
				if (pos.y < bottom_right.y) {
					bottom_right.y = pos.y;
				}
				if (pos.x > bottom_right.x) {
					bottom_right.x = pos.x;
				}
				if (pos.x < top_left.x) {
					top_left.x = pos.x;
				}
			}

			Vector2 size = Vector2(bottom_right.x - top_left.x, top_left.y - bottom_right.y);

			Vector2 offset = last_pos + (last_size / 2.0) + (size / 2.0); // dunno I might just keep it in one axis but diagonal feels better....

			last_pos = offset;
			last_size = size;

			for (int j = 0; j < nodes.size(); j += 3) {
				add_node(name, nodes[j], nodes[j + 2], offset + nodes[j + 1]); // also add an additional buffer if you want to
			}

		} else {
			for (int j = 0; j < nodes.size(); j += 3) {
				add_node(name, nodes[j], nodes[j + 2], nodes[j + 1]);
			}
		}
		Array sequence_connections = func["sequence_connections"];

		for (int j = 0; j < sequence_connections.size(); j += 3) {
			sequence_connect(name, sequence_connections[j + 0], sequence_connections[j + 1], sequence_connections[j + 2]);
		}

		Array data_connections = func["data_connections"];

		for (int j = 0; j < data_connections.size(); j += 4) {
			data_connect(name, data_connections[j + 0], data_connections[j + 1], data_connections[j + 2], data_connections[j + 3]);
		}
	}

	if (d.has("is_tool_script")) {
		is_tool_script = d["is_tool_script"];
	} else {
		is_tool_script = false;
	}
}

Dictionary C11RScript::_get_data() const {
	Dictionary d;
	d["base_type"] = base_type;
	Array vars;
	for (const Map<StringName, Variable>::Element *E = variables.front(); E; E = E->next()) {
		Dictionary var = _get_variable_info(E->key());
		var["name"] = E->key(); //make sure it's the right one
		var["default_value"] = E->get().default_value;
		var["export"] = E->get()._export;
		vars.push_back(var);
	}
	d["variables"] = vars;

	Array sigs;
	for (const Map<StringName, Vector<Argument>>::Element *E = custom_signals.front(); E; E = E->next()) {
		Dictionary cs;
		cs["name"] = E->key();
		Array args;
		for (int i = 0; i < E->get().size(); i++) {
			args.push_back(E->get()[i].name);
			args.push_back(E->get()[i].type);
		}
		cs["arguments"] = args;

		sigs.push_back(cs);
	}

	d["signals"] = sigs;

	Array funcs;

	for (const Map<StringName, Function>::Element *E = functions.front(); E; E = E->next()) {
		Dictionary func;
		func["name"] = E->key();
		func["function_id"] = E->get().function_id;
		func["scroll"] = E->get().scroll;

		Array nodes;

		for (const Map<int, Function::NodeData>::Element *F = E->get().nodes.front(); F; F = F->next()) {
			nodes.push_back(F->key());
			nodes.push_back(F->get().pos);
			nodes.push_back(F->get().node);
		}

		func["nodes"] = nodes;

		Array sequence_connections;

		for (const Set<SequenceConnection>::Element *F = E->get().sequence_connections.front(); F; F = F->next()) {
			sequence_connections.push_back(F->get().from_node);
			sequence_connections.push_back(F->get().from_output);
			sequence_connections.push_back(F->get().to_node);
		}

		func["sequence_connections"] = sequence_connections;

		Array data_connections;

		for (const Set<DataConnection>::Element *F = E->get().data_connections.front(); F; F = F->next()) {
			data_connections.push_back(F->get().from_node);
			data_connections.push_back(F->get().from_port);
			data_connections.push_back(F->get().to_node);
			data_connections.push_back(F->get().to_port);
		}

		func["data_connections"] = data_connections;

		funcs.push_back(func);
	}

	d["functions"] = funcs;
	d["is_tool_script"] = is_tool_script;
	d["vs_unify"] = true;

	return d;
}

void C11RScript::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_node_ports_changed"), &C11RScript::_node_ports_changed);

	ClassDB::bind_method(D_METHOD("add_function", "name"), &C11RScript::add_function);
	ClassDB::bind_method(D_METHOD("has_function", "name"), &C11RScript::has_function);
	ClassDB::bind_method(D_METHOD("remove_function", "name"), &C11RScript::remove_function);
	ClassDB::bind_method(D_METHOD("rename_function", "name", "new_name"), &C11RScript::rename_function);
	ClassDB::bind_method(D_METHOD("set_function_scroll", "name", "ofs"), &C11RScript::set_function_scroll);
	ClassDB::bind_method(D_METHOD("get_function_scroll", "name"), &C11RScript::get_function_scroll);

	ClassDB::bind_method(D_METHOD("add_node", "func", "id", "node", "position"), &C11RScript::add_node, DEFVAL(Point2()));
	ClassDB::bind_method(D_METHOD("remove_node", "func", "id"), &C11RScript::remove_node);
	ClassDB::bind_method(D_METHOD("get_function_node_id", "name"), &C11RScript::get_function_node_id);

	ClassDB::bind_method(D_METHOD("get_node", "func", "id"), &C11RScript::get_node);
	ClassDB::bind_method(D_METHOD("has_node", "func", "id"), &C11RScript::has_node);
	ClassDB::bind_method(D_METHOD("set_node_position", "func", "id", "position"), &C11RScript::set_node_position);
	ClassDB::bind_method(D_METHOD("get_node_position", "func", "id"), &C11RScript::get_node_position);

	ClassDB::bind_method(D_METHOD("sequence_connect", "func", "from_node", "from_output", "to_node"), &C11RScript::sequence_connect);
	ClassDB::bind_method(D_METHOD("sequence_disconnect", "func", "from_node", "from_output", "to_node"), &C11RScript::sequence_disconnect);
	ClassDB::bind_method(D_METHOD("has_sequence_connection", "func", "from_node", "from_output", "to_node"), &C11RScript::has_sequence_connection);

	ClassDB::bind_method(D_METHOD("data_connect", "func", "from_node", "from_port", "to_node", "to_port"), &C11RScript::data_connect);
	ClassDB::bind_method(D_METHOD("data_disconnect", "func", "from_node", "from_port", "to_node", "to_port"), &C11RScript::data_disconnect);
	ClassDB::bind_method(D_METHOD("has_data_connection", "func", "from_node", "from_port", "to_node", "to_port"), &C11RScript::has_data_connection);

	ClassDB::bind_method(D_METHOD("add_variable", "name", "default_value", "export"), &C11RScript::add_variable, DEFVAL(Variant()), DEFVAL(false));
	ClassDB::bind_method(D_METHOD("has_variable", "name"), &C11RScript::has_variable);
	ClassDB::bind_method(D_METHOD("remove_variable", "name"), &C11RScript::remove_variable);
	ClassDB::bind_method(D_METHOD("set_variable_default_value", "name", "value"), &C11RScript::set_variable_default_value);
	ClassDB::bind_method(D_METHOD("get_variable_default_value", "name"), &C11RScript::get_variable_default_value);
	ClassDB::bind_method(D_METHOD("set_variable_info", "name", "value"), &C11RScript::_set_variable_info);
	ClassDB::bind_method(D_METHOD("get_variable_info", "name"), &C11RScript::_get_variable_info);
	ClassDB::bind_method(D_METHOD("set_variable_export", "name", "enable"), &C11RScript::set_variable_export);
	ClassDB::bind_method(D_METHOD("get_variable_export", "name"), &C11RScript::get_variable_export);
	ClassDB::bind_method(D_METHOD("rename_variable", "name", "new_name"), &C11RScript::rename_variable);

	ClassDB::bind_method(D_METHOD("add_custom_signal", "name"), &C11RScript::add_custom_signal);
	ClassDB::bind_method(D_METHOD("has_custom_signal", "name"), &C11RScript::has_custom_signal);
	ClassDB::bind_method(D_METHOD("custom_signal_add_argument", "name", "type", "argname", "index"), &C11RScript::custom_signal_add_argument, DEFVAL(-1));
	ClassDB::bind_method(D_METHOD("custom_signal_set_argument_type", "name", "argidx", "type"), &C11RScript::custom_signal_set_argument_type);
	ClassDB::bind_method(D_METHOD("custom_signal_get_argument_type", "name", "argidx"), &C11RScript::custom_signal_get_argument_type);
	ClassDB::bind_method(D_METHOD("custom_signal_set_argument_name", "name", "argidx", "argname"), &C11RScript::custom_signal_set_argument_name);
	ClassDB::bind_method(D_METHOD("custom_signal_get_argument_name", "name", "argidx"), &C11RScript::custom_signal_get_argument_name);
	ClassDB::bind_method(D_METHOD("custom_signal_remove_argument", "name", "argidx"), &C11RScript::custom_signal_remove_argument);
	ClassDB::bind_method(D_METHOD("custom_signal_get_argument_count", "name"), &C11RScript::custom_signal_get_argument_count);
	ClassDB::bind_method(D_METHOD("custom_signal_swap_argument", "name", "argidx", "withidx"), &C11RScript::custom_signal_swap_argument);
	ClassDB::bind_method(D_METHOD("remove_custom_signal", "name"), &C11RScript::remove_custom_signal);
	ClassDB::bind_method(D_METHOD("rename_custom_signal", "name", "new_name"), &C11RScript::rename_custom_signal);

	//ClassDB::bind_method(D_METHOD("set_variable_info","name","info"),&VScript::set_variable_info);
	//ClassDB::bind_method(D_METHOD("get_variable_info","name"),&VScript::set_variable_info);

	ClassDB::bind_method(D_METHOD("set_instance_base_type", "type"), &C11RScript::set_instance_base_type);

	ClassDB::bind_method(D_METHOD("_set_data", "data"), &C11RScript::_set_data);
	ClassDB::bind_method(D_METHOD("_get_data"), &C11RScript::_get_data);

	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "data", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR | PROPERTY_USAGE_INTERNAL), "_set_data", "_get_data");

	ADD_SIGNAL(MethodInfo("node_ports_changed", PropertyInfo(Variant::STRING, "function"), PropertyInfo(Variant::INT, "id")));
}

C11RScript::C11RScript() {
	base_type = "Object";
	is_tool_script = false;
}

bool C11RScript::inherits_script(const Ref<Script> &p_script) const {
	return this == p_script.ptr(); //there is no inheritance in visual scripts, so this is enough
}

StringName C11RScript::get_default_func() const {
	return StringName("f_312843592");
}

Set<int> C11RScript::get_output_sequence_ports_connected(const String &edited_func, int from_node) {
	List<C11RScript::SequenceConnection> *sc = memnew(List<C11RScript::SequenceConnection>);
	get_sequence_connection_list(edited_func, sc);
	Set<int> connected;
	for (List<C11RScript::SequenceConnection>::Element *E = sc->front(); E; E = E->next()) {
		if (E->get().from_node == from_node) {
			connected.insert(E->get().from_output);
		}
	}
	memdelete(sc);
	return connected;
}

C11RScript::~C11RScript() {
	while (!functions.empty()) {
		remove_function(functions.front()->key());
	}
}
