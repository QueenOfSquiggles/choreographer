#include "C11REditor.hpp"

#include "core/object.h"
#include "core/os/input.h"
#include "core/os/keyboard.h"
#include "core/script_language.h"
#include "core/variant.h"
#include "editor/editor_node.h"
#include "editor/editor_resource_preview.h"
#include "editor/editor_scale.h"
#include "scene/main/viewport.h"

#ifdef TOOLS_ENABLED


C11REditor::C11REditor()
{
    if(!clipboard)
    {
        clipboard = memnew(Clipboard);
    }

    // TODO initialize graphics of editor
    // Note: this is a node in the editor's scene tree, we can add Control child nodes

    undo_redo = EditorNode::get_singleton()->get_undo_redo();
    set_process_input(true);
    set_process_unhandled_input(true);
}

C11REditor::~C11REditor()
{
    undo_redo->clear_history();
}


static ScriptEditorBase *create_editor(const RES &p_resource) {
	if (Object::cast_to<C11RScript>(*p_resource)) {
		return memnew(C11REditor);
	}

	return nullptr;
}

C11REditor::Clipboard *C11REditor::clipboard = nullptr;

void C11REditor::_bind_methods(){}
void C11REditor::add_syntax_highlighter(SyntaxHighlighter *p_highlighter){}
void C11REditor::set_syntax_highlighter(SyntaxHighlighter *p_highlighter){}

void C11REditor::apply_code(){}
RES C11REditor::get_edited_resource() const
{
    return RES(); // FIXME
}
void C11REditor::set_edited_resource(const RES &p_res){}
void C11REditor::enable_editor(){}
Vector<String> C11REditor::get_functions(){
    return Vector<String>(); // FIXME
}
void C11REditor::reload_text(){}
String C11REditor::get_name(){
    return String(); // FIXME
}
Ref<Texture> C11REditor::get_icon(){
    return nullptr;// FIXME
}
bool C11REditor::is_unsaved()
{
    return false;
}
Variant C11REditor::get_edit_state(){
    return Variant();
}
void C11REditor::set_edit_state(const Variant &p_state){}
void C11REditor::goto_line(int p_line, bool p_with_error){}
void C11REditor::set_executing_line(int p_line){}
void C11REditor::clear_executing_line(){}
void C11REditor::trim_trailing_whitespace(){}
void C11REditor::insert_final_newline(){}
void C11REditor::convert_indent_to_spaces(){}
void C11REditor::convert_indent_to_tabs(){}
void C11REditor::ensure_focus(){}
void C11REditor::tag_saved_version(){}
void C11REditor::reload(bool p_soft){}
void C11REditor::get_breakpoints(List<int> *p_breakpoints){}
void C11REditor::add_callback(const String &p_function, PoolStringArray p_args) {}
void C11REditor::update_settings(){}
bool C11REditor::show_members_overview()
{
    return false;
}
void C11REditor::set_debugger_active(bool p_active){}
void C11REditor::set_tooltip_request_func(String p_method, Object *p_obj){}
Control *C11REditor::get_edit_menu(){
    return nullptr;
}
void C11REditor::clear_edit_menu(){}

void C11REditor::validate(){}


static void register_editor_callback()
{
    ScriptEditor::register_create_script_editor_function(create_editor);

    ED_SHORTCUT("choreographer/delete_selected", TTR("Delete Selected"), KEY_DELETE);
	ED_SHORTCUT("choreographer/toggle_breakpoint", TTR("Toggle Breakpoint"), KEY_F9);
	ED_SHORTCUT("choreographer/find_node_type", TTR("Find Node Type"), KEY_MASK_CMD + KEY_F);
	ED_SHORTCUT("choreographer/copy_nodes", TTR("Copy Nodes"), KEY_MASK_CMD + KEY_C);
	ED_SHORTCUT("choreographer/cut_nodes", TTR("Cut Nodes"), KEY_MASK_CMD + KEY_X);
	ED_SHORTCUT("choreographer/paste_nodes", TTR("Paste Nodes"), KEY_MASK_CMD + KEY_V);
	ED_SHORTCUT("choreographer/create_function", TTR("Make Function"), KEY_MASK_CMD + KEY_G);
	ED_SHORTCUT("choreographer/refresh_nodes", TTR("Refresh Graph"), KEY_MASK_CMD + KEY_R);
	ED_SHORTCUT("choreographer/edit_member", TTR("Edit Member"), KEY_MASK_CMD + KEY_E);
}

void C11REditor::register_editor(){

    EditorNode::add_plugin_init_callback(register_editor_callback);
}

void C11REditor::free_clipboard(){
    if(clipboard)
    {
        memdelete(clipboard);
    }
}

_C11REditor *_C11REditor::singleton = nullptr;

_C11REditor::_C11REditor()
{
    singleton = this;
}
_C11REditor::~_C11REditor()
{
}

void _C11REditor::_bind_methods()
{
}
#endif