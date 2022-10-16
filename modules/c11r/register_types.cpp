#include "core/engine.h"
#include "register_types.h"
#include "editor/C11REditor.hpp"
C11RScriptLanguage* c11r_script_language = nullptr;

#ifdef TOOLS_ENABLED
static _C11REditor *c11r_editor_singleton = nullptr;
#endif

void register_c11r_types()
{
    c11r_script_language = memnew(C11RScriptLanguage);
    ScriptServer::register_language(c11r_script_language);
	
	ClassDB::register_class<C11RScript>();
	ClassDB::register_virtual_class<Block>();
	ClassDB::register_class<C11RScriptFunctionState>();

	#ifdef TOOLS_ENABLED
    ClassDB::set_current_api(ClassDB::API_EDITOR);
	ClassDB::register_class<_C11REditor>();
	ClassDB::set_current_api(ClassDB::API_CORE);
	c11r_editor_singleton = memnew(_C11REditor);
	Engine::get_singleton()->add_singleton(Engine::Singleton("VisualScriptEditor", _C11REditor::get_singleton()));
	C11REditor::register_editor();
	#endif

// TODO implement an editor to use for this block
/*
    #ifdef TOOLS_ENABLED
    ClassDB::set_current_api(ClassDB::API_EDITOR);
	ClassDB::register_class<_VisualScriptEditor>(); // registers the editor
	ClassDB::set_current_api(ClassDB::API_CORE);
	vs_editor_singleton = memnew(_VisualScriptEditor);
	Engine::get_singleton()->add_singleton(Engine::Singleton("VisualScriptEditor", _VisualScriptEditor::get_singleton()));

	VisualScriptEditor::register_editor(); // makes the ScriptServer aware of the editor and its callbacks
    #endif
*/
}

void unregister_c11r_types()
{
    ScriptServer::unregister_language(c11r_script_language);
	
#ifdef TOOLS_ENABLED
	C11REditor::free_clipboard();
	if(c11r_editor_singleton)
	{
		memdelete(c11r_editor_singleton);
	}
#endif

    if(c11r_script_language)
    {
        memdelete(c11r_script_language);
    }
}


/* From: visual_script/register_types.cpp


VisualScriptLanguage *visual_script_language = nullptr;
#ifdef TOOLS_ENABLED
static _VisualScriptEditor *vs_editor_singleton = nullptr;
#endif

void register_visual_script_types() {
	visual_script_language = memnew(VisualScriptLanguage);
	//script_language_gd->init();
	ScriptServer::register_language(visual_script_language);

	ClassDB::register_class<VisualScript>();
	ClassDB::register_virtual_class<VisualScriptNode>();
	ClassDB::register_class<VisualScriptFunctionState>();
	ClassDB::register_class<VisualScriptFunction>();
	ClassDB::register_virtual_class<VisualScriptLists>();
	ClassDB::register_class<VisualScriptComposeArray>();
	ClassDB::register_class<VisualScriptOperator>();
	ClassDB::register_class<VisualScriptVariableSet>();
	ClassDB::register_class<VisualScriptVariableGet>();
	ClassDB::register_class<VisualScriptConstant>();
	ClassDB::register_class<VisualScriptIndexGet>();
	ClassDB::register_class<VisualScriptIndexSet>();
	ClassDB::register_class<VisualScriptGlobalConstant>();
	ClassDB::register_class<VisualScriptClassConstant>();
	ClassDB::register_class<VisualScriptMathConstant>();
	ClassDB::register_class<VisualScriptBasicTypeConstant>();
	ClassDB::register_class<VisualScriptEngineSingleton>();
	ClassDB::register_class<VisualScriptSceneNode>();
	ClassDB::register_class<VisualScriptSceneTree>();
	ClassDB::register_class<VisualScriptResourcePath>();
	ClassDB::register_class<VisualScriptSelf>();
	ClassDB::register_class<VisualScriptCustomNode>();
	ClassDB::register_class<VisualScriptSubCall>();
	ClassDB::register_class<VisualScriptComment>();
	ClassDB::register_class<VisualScriptConstructor>();
	ClassDB::register_class<VisualScriptLocalVar>();
	ClassDB::register_class<VisualScriptLocalVarSet>();
	ClassDB::register_class<VisualScriptInputAction>();
	ClassDB::register_class<VisualScriptDeconstruct>();
	ClassDB::register_class<VisualScriptPreload>();
	ClassDB::register_class<VisualScriptTypeCast>();

	ClassDB::register_class<VisualScriptFunctionCall>();
	ClassDB::register_class<VisualScriptPropertySet>();
	ClassDB::register_class<VisualScriptPropertyGet>();
	//ClassDB::register_type<VisualScriptScriptCall>();
	ClassDB::register_class<VisualScriptEmitSignal>();

	ClassDB::register_class<VisualScriptReturn>();
	ClassDB::register_class<VisualScriptCondition>();
	ClassDB::register_class<VisualScriptWhile>();
	ClassDB::register_class<VisualScriptIterator>();
	ClassDB::register_class<VisualScriptSequence>();
	//ClassDB::register_class<VisualScriptInputFilter>();
	ClassDB::register_class<VisualScriptSwitch>();
	ClassDB::register_class<VisualScriptSelect>();

	ClassDB::register_class<VisualScriptYield>();
	ClassDB::register_class<VisualScriptYieldSignal>();

	ClassDB::register_class<VisualScriptBuiltinFunc>();

	ClassDB::register_class<VisualScriptExpression>();

	register_visual_script_nodes();
	register_visual_script_func_nodes();
	register_visual_script_builtin_func_node();
	register_visual_script_flow_control_nodes();
	register_visual_script_yield_nodes();
	register_visual_script_expression_node();

#ifdef TOOLS_ENABLED
	ClassDB::set_current_api(ClassDB::API_EDITOR);
	ClassDB::register_class<_VisualScriptEditor>();
	ClassDB::set_current_api(ClassDB::API_CORE);
	vs_editor_singleton = memnew(_VisualScriptEditor);
	Engine::get_singleton()->add_singleton(Engine::Singleton("VisualScriptEditor", _VisualScriptEditor::get_singleton()));

	VisualScriptEditor::register_editor();
#endif
}

void unregister_visual_script_types() {
	unregister_visual_script_nodes();

	ScriptServer::unregister_language(visual_script_language);

#ifdef TOOLS_ENABLED
	VisualScriptEditor::free_clipboard();
	if (vs_editor_singleton) {
		memdelete(vs_editor_singleton);
	}
#endif
	if (visual_script_language) {
		memdelete(visual_script_language);
	}
}



*/