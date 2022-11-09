#include "c11r_lang.hpp"

#include "core/core_string_names.h"
#include "core/os/os.h"
#include "core/project_settings.h"
#include "scene/main/node.h"

#include "packs/block_pack.hpp"


void Block::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_set_block_namespace"), &Block::_set_block_namespace);
	ClassDB::bind_method(D_METHOD("_get_block_namespace"), &Block::_get_block_namespace);
	
	MethodInfo block_call_func;
	block_call_func.name="_call_block";
	block_call_func.arguments.push_back(PropertyInfo(Variant::ARRAY, "arg_stack"));
	block_call_func.return_val = PropertyInfo(Variant::ARRAY, "return_arg_stack");
	ClassDB::add_virtual_method("Block", block_call_func);

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

Block::Block() {
}

////////////////


BlockInstance::BlockInstance() {
}

BlockInstance::~BlockInstance() {
}

/////////////////////

C11RScript::C11RScript() {
}

C11RScript::~C11RScript() {
}

void C11RScript::_bind_methods()
{
	print_line("Binding class DB functions for c11r");
	ClassDB::bind_method(D_METHOD("custom_func"), &C11RScript::internal_ready);
}

void C11RScript::load(Ref<ConfigFile> p_config)
{
	print_line("Loading C11RScript from custom loader (ConfigFile)");


	String cur_section = "";
	{ // block pack section
		cur_section = "block_packs";
		List<String> block_pack_keys;
		p_config->get_section_keys(cur_section, &block_pack_keys);
		for(int i = 0; i < block_pack_keys.size(); i++)
		{
			String key = block_pack_keys[i];
			String path = p_config->get_value(cur_section, key, "null");
			if (path == "null")
			{
				print_line("Failed to get proper path for config block pack dep");
				continue;
			}
			Ref<BlockPack> pack = ResourceLoader::load(path, "BlockPack");
			if (pack.is_null())
			{
				print_line("Failed to load block pack");
				continue;
			}
			print_line(vformat("Registering block pack into script dependencies: %s", pack->namespace_prefix));
			block_pack_dependencies.push_back(pack);
		}
	}
}

Ref<ConfigFile> C11RScript::save() const
{
	Ref<ConfigFile> cfg;
	cfg.instance();

	// block pack references
	for(int i = 0; i < block_pack_dependencies.size(); i++)
	{
		Ref<BlockPack> pack = block_pack_dependencies[i];
		if (pack.is_null()) continue;
		cfg->set_value("block packs", pack->namespace_prefix, pack->get_path());
	}


	return cfg;
}

void C11RScript::internal_ready()
{
	print_line("C11RScript - Ready");
}

// overrides
bool C11RScript::can_instance() const {
	return true;
}

Ref<Script> C11RScript::get_base_script() const {
	return base;
}

bool C11RScript::inherits_script(const Ref<Script> &p_script) const {
	return false; // TODO recursive lookup
}

StringName C11RScript::get_instance_base_type() const 
{
	if(base.is_valid())
	{
		return base->get_instance_base_type();
	}
	return StringName();
}

ScriptInstance *C11RScript::instance_create(Object *p_this) 
{
	if(!Object::cast_to<C11RScript>(p_this))
	{
		return nullptr;
	}

	C11RScriptInstance *instance = memnew(C11RScriptInstance);
	instance->script = *(Object::cast_to<Ref<C11RScript>>(p_this));
	// TODO add in any instance dependencies
	return instance;
}

bool C11RScript::instance_has(const Object *p_this) const 
{
	return false;
}

bool C11RScript::has_source_code() const 
{
	return false;
}

String C11RScript::get_source_code() const 
{
	return "";
}

void C11RScript::set_source_code(const String &p_code){}

Error C11RScript::reload(bool p_keep_state)
{
	return OK;
}

bool C11RScript::has_method(const StringName &p_method) const 
{
	for(int i = 0; i < functions.size(); i++)
	{
		BlockFunction func = functions[i];
		if (func.method_info.name == p_method) return true;
	}
	for(int i = 0; i < composited_blocks.size(); i++)
	{
		Composition comp = composited_blocks[i];
		// we want to ignore virtual compositions because they are not considered implementation without explicit specification of function calls
		// >> virtual compositions can provide optional function calls, but do not provide any default functions. Must be explicitly called by the compositing script.
		if(!comp.is_virtual && comp.script->has_method(p_method)) return true;
	}
	return false;
}

MethodInfo C11RScript::get_method_info(const StringName &p_method) const 
{
	for(int i = 0; i < functions.size(); i++)
	{
		BlockFunction func = functions[i];
		if (func.method_info.name == p_method) return func.method_info;
	}
	for(int i = 0; i < composited_blocks.size(); i++)
	{
		Composition comp = composited_blocks[i];
		// we want to ignore virtual compositions because they are not considered implementation without explicit specification of function calls
		// >> virtual compositions can provide optional function calls, but do not provide any default functions. Must be explicitly called by the compositing script.
		if(!comp.is_virtual && comp.script->has_method(p_method))
		{
			List<MethodInfo> block_funcs;
			comp.script->get_method_list(&block_funcs);
			for(int j = 0; j < block_funcs.size(); j++)
			{
				// BUG this will generally produce unpredictable behaviour. I'm not sure if this function is being called internally very often, or expects consistent results.
				MethodInfo info = block_funcs[i];
				if(info.name == p_method) return info;				
			}
		}
	}
	return MethodInfo();
}

bool C11RScript::is_tool() const 
{
	return false; // TODO do we want to allow tool scripts for this language? It would require some extra effort to maintain stability
}

bool C11RScript::is_valid() const 
{
	return true;
}

ScriptLanguage *C11RScript::get_language() const 
{
	return C11RScriptLanguage::singleton;
}

bool C11RScript::has_script_signal(const StringName &p_signal) const 
{
	return false;
}


Variant C11RScript::call(const StringName &p_method, const Variant **p_args, int p_argcount, Variant::CallError &r_error) {

	// TODO call functions as Block Entries

	return Script::call(p_method, p_args, p_argcount, r_error);
}

void C11RScript::get_script_signal_list(List<MethodInfo> *r_signals) const 
{}

bool C11RScript::get_property_default_value(const StringName &p_property, Variant &r_value) const 
{
	return false;
}

void C11RScript::get_script_method_list(List<MethodInfo> *p_list) const 
{
	ClassDB::get_method_list("C11RScript", p_list);

	for(int i = 0; i < functions.size(); i++)
	{
		BlockFunction func = functions[i];
		p_list->push_back(func.method_info);
	}
	for(int i = 0; i < composited_blocks.size(); i++)
	{
		Composition comp = composited_blocks[i];
		if(!p_list->find(comp.function.method_info))
		{ // only add to method list when not present in compositing script
			p_list->push_back(comp.function.method_info);
		}
	}
	
	if(base.is_valid())
	{
		List<MethodInfo> base_funcs;
		base->get_script_method_list(&base_funcs);
		for(int i = 0; i < base_funcs.size(); i++)
		{
			MethodInfo info = base_funcs[i];
			if(!p_list->find(info))
			{ // only add to method list when not present in child script
				p_list->push_back(info);
			}
		}
	}
}

void C11RScript::get_script_property_list(List<PropertyInfo> *p_list) const 
{
	for(int i = 0; i < properties.size(); i++)
	{
		C11RProperty prop = properties[i];
		p_list->push_back(prop.property);
	}
	if (base.is_valid()) 
	{
		base->get_script_property_list(p_list);
	}
	// intentionally, we do not add properties from composited blocks. Just from the base because that's important for adding the script to nodes
}

int C11RScript::get_member_line(const StringName &p_member) const 
{
	return 0;
}

void C11RScript::get_constants(Map<StringName, Variant> *p_constants){}
void C11RScript::get_members(Set<StringName> *p_constants){}


////////////////////////////////////////////

bool C11RScriptInstance::set(const StringName &p_name, const Variant &p_value) {
	bool r_valid;
	script->set(p_name, p_value, &r_valid);
	return r_valid;
}

bool C11RScriptInstance::get(const StringName &p_name, Variant &r_ret) const {
	bool r_valid;
	Variant value = script->get(p_name, &r_valid);
	if(!r_valid) return false;

	r_ret = value;
	return true;
}
void C11RScriptInstance::get_property_list(List<PropertyInfo> *p_properties) const {
	script->get_property_list(p_properties);
}

Variant::Type C11RScriptInstance::get_property_type(const StringName &p_name, bool *r_is_valid) const {
	return Variant::NIL; // FIXME
}

void C11RScriptInstance::get_method_list(List<MethodInfo> *p_list) const {
	script->get_method_list(p_list);
}
bool C11RScriptInstance::has_method(const StringName &p_method) const {
	return script->has_method(p_method);
}

Variant C11RScriptInstance::call(const StringName &p_method, const Variant **p_args, int p_argcount, Variant::CallError &r_error) {
	if(script->has_method(p_method))
	{
		return script->call(p_method, p_args, p_argcount, r_error);
	}
	return Variant();
}

void C11RScriptInstance::notification(int p_notification) {
	print_line(vformat("c11r script instance received notification ID %s", itos(p_notification)));
	//notification is not virtual, it gets called at ALL levels just like in C.
	// Variant value = p_notification;
	// const Variant *args[1] = { &value };

	// Script *sptr = script.ptr();
	// while (sptr) {
	// 	if (sptr->has_method("notification")) { // I don't like hardcoding this. Is there a real alternative?
	// 		Variant::CallError err;
	// 		sptr->call("notification", args, 1, err);
	// 		// E->get()->call(this, args, 1, err);
	// 		if (err.error != Variant::CallError::CALL_OK) {
	// 			//print error about notification call
	// 		}
	// 	}
	// 	sptr = sptr->get_base_script().ptr();
	// }
	// 
}

String C11RScriptInstance::to_string(bool *r_valid) {
	if (has_method(CoreStringNames::get_singleton()->_to_string)) {
		Variant::CallError ce;
		Variant ret = call(CoreStringNames::get_singleton()->_to_string, nullptr, 0, ce);
		if (ce.error == Variant::CallError::CALL_OK) {
			if (ret.get_type() != Variant::STRING) {
				if (r_valid) {
					*r_valid = false;
				}
				ERR_FAIL_V_MSG(String(), "Wrong type for " + CoreStringNames::get_singleton()->_to_string + ", must be a String.");
			}
			if (r_valid) {
				*r_valid = true;
			}
			return ret.operator String();
		}
	}
	if (r_valid) {
		*r_valid = false;
	}
	return String();
}

Ref<Script> C11RScriptInstance::get_script() const {
	return script;
}

MultiplayerAPI::RPCMode C11RScriptInstance::get_rpc_mode(const StringName &p_method) const {
	// TODO implement RPCMode assignments for functions
	return MultiplayerAPI::RPC_MODE_DISABLED;
}

MultiplayerAPI::RPCMode C11RScriptInstance::get_rset_mode(const StringName &p_variable) const {
	// TODO implement RSET assignments for properties/variables
	return MultiplayerAPI::RPC_MODE_DISABLED;
}
ScriptLanguage *C11RScriptInstance::get_language() {
	return C11RScriptLanguage::singleton;
}

C11RScriptInstance::C11RScriptInstance() {
}

C11RScriptInstance::~C11RScriptInstance() {
}

///////////////////////////////////////////////

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
	return C11R_LANG_FILE_EXT;
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
	return script;
}

bool C11RScriptLanguage::is_using_templates() {
	return false;
}

void C11RScriptLanguage::make_template(const String &p_class_name, const String &p_base_class_name, Ref<Script> &p_script) {
}

bool C11RScriptLanguage::validate(const String &p_script, int &r_line_error, int &r_col_error, String &r_test_error, const String &p_path, List<String> *r_functions, List<ScriptLanguage::Warning> *r_warnings, Set<int> *r_safe_lines) const {
	return false;
}
Script *C11RScriptLanguage::create_script() const {
	return memnew(C11RScript);
}
bool C11RScriptLanguage::has_named_classes() const {
	return false;
}
bool C11RScriptLanguage::supports_builtin_mode() const {
	return true;
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
	return false;
}

bool C11RScriptLanguage::debug_break(const String &p_error, bool p_allow_continue) {
	return false;
}

String C11RScriptLanguage::debug_get_error() const {
	return "";
}

int C11RScriptLanguage::debug_get_stack_level_count() const {
	return 1;
}
int C11RScriptLanguage::debug_get_stack_level_line(int p_level) const {
	return 0;
}
String C11RScriptLanguage::debug_get_stack_level_function(int p_level) const {
	return "";
}
String C11RScriptLanguage::debug_get_stack_level_source(int p_level) const {
	return "";
}
void C11RScriptLanguage::debug_get_stack_level_locals(int p_level, List<String> *p_locals, List<Variant> *p_values, int p_max_subitems, int p_max_depth) {
}

void C11RScriptLanguage::debug_get_stack_level_members(int p_level, List<String> *p_members, List<Variant> *p_values, int p_max_subitems, int p_max_depth) {
}

void C11RScriptLanguage::debug_get_globals(List<String> *p_locals, List<Variant> *p_values, int p_max_subitems, int p_max_depth) {
	//no globals are really reachable in gdscript
}
String C11RScriptLanguage::debug_parse_stack_level_expression(int p_level, const String &p_expression, int p_max_subitems, int p_max_depth) {
	return "";
}

void C11RScriptLanguage::reload_all_scripts() {
}
void C11RScriptLanguage::reload_tool_script(const Ref<Script> &p_script, bool p_soft_reload) {
}
/* LOADER FUNCTIONS */

void C11RScriptLanguage::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back(C11R_LANG_FILE_EXT);
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
	return 0;
}

int C11RScriptLanguage::profiling_get_frame_data(ProfilingInfo *p_info_arr, int p_info_max) {
	return 0;
}

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

Ref<Block> C11RScriptLanguage::get_block(const String &p_name) 
{
	if (registered_blocks.has(p_name))
	{
		return registered_blocks.find(p_name)->get();
	}
	print_error(vformat("Cannot find registered block '%s'", p_name));
	return Ref<Block>();
}


C11RScriptLanguage *C11RScriptLanguage::singleton = nullptr;

C11RScriptLanguage::C11RScriptLanguage() {
	// use GLOBALDEF to define some settings
	C11RScriptLanguage::singleton = this;
	Array default_packs = GLOBAL_DEF("choreographer/block_packs/default_packs", Array());
	// TODO relocate this to where it will be called when a project is opened (or reloaded)
	// for(int i = 0; i < default_packs.size(); i++)
	// {	// load default packs
	// 	Variant pack = default_packs[i];
	// 	if(pack.get_type() == Variant::STRING)
	// 	{
	// 		// path
	// 		RES res = ResourceLoader::load(pack);
	// 		if(res.is_valid()){
	// 			Ref<BlockPack> block_pack = res;
	// 			if (block_pack.is_valid()) {
	// 				// we have a valid block pack!
	// 				print_line(vformat("Loading block pack with namespace prefix `%s`", block_pack->namespace_prefix));
	// 			}
	// 		} else {
	// 			print_line(vformat("Failed to load block pack from path: %s", pack));
	// 		}


	// 	} else if (pack.get_type() == Variant::OBJECT) {
	// 		// resource reference?
	// 		// TODO handle resource reference
	// 	} else {
	// 		print_error(vformat("Invalid pack entry: %s", pack));
	// 	}

	// }

}

C11RScriptLanguage::~C11RScriptLanguage() {
	singleton = nullptr;
}
