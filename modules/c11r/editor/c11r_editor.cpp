#include "c11r_editor.hpp"

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



// Colours in a map so we can load different colour sets as desired.
// Maybe at some point this could even be a project/editor setting? Like the GDScript colour options?
// Likely we could have a third map of "colour_overrides" which gets checked first, then the light/dark check happens if no override exists. This would let users assign specific colour overrides if they desire, without destroying the defaults. Which is generally desired in Godot. 
static bool _is_col_init = false;
static Map<Variant::Type, Color> colours_light;
static Map<Variant::Type, Color> colours_dark;

static void _load_colours()
{
    if (_is_col_init) return;
    colours_dark.insert(Variant::NIL, Color(0.41, 0.93, 0.74));
    colours_dark.insert(Variant::BOOL, Color(0.55, 0.65, 0.94));
    colours_dark.insert(Variant::INT, Color(0.49, 0.78, 0.94));
    colours_dark.insert(Variant::REAL, Color(0.38, 0.85, 0.96));
    colours_dark.insert(Variant::STRING,  Color(0.42, 0.65, 0.93));
    colours_dark.insert(Variant::VECTOR2, Color(0.74, 0.57, 0.95));
    colours_dark.insert(Variant::RECT2, Color(0.95, 0.57, 0.65));
    colours_dark.insert(Variant::VECTOR3, Color(0.84, 0.49, 0.93));
    colours_dark.insert(Variant::TRANSFORM2D, Color(0.77, 0.93, 0.41));
    colours_dark.insert(Variant::PLANE, Color(0.97, 0.44, 0.44));
    colours_dark.insert(Variant::QUAT, Color(0.93, 0.41, 0.64));
    colours_dark.insert(Variant::AABB, Color(0.93, 0.47, 0.57));
    colours_dark.insert(Variant::BASIS, Color(0.89, 0.93, 0.41));
    colours_dark.insert(Variant::TRANSFORM, Color(0.96, 0.66, 0.43));
    colours_dark.insert(Variant::COLOR, Color(0.62, 1.0, 0.44));
    colours_dark.insert(Variant::NODE_PATH, Color(0.41, 0.58, 0.93));
    colours_dark.insert(Variant::_RID, Color(0.41, 0.93, 0.6));
    colours_dark.insert(Variant::OBJECT, Color(0.47, 0.95, 0.91));
    colours_dark.insert(Variant::DICTIONARY, Color(0.47, 0.93, 0.69));
    colours_dark.insert(Variant::ARRAY, Color(0.88, 0.88, 0.88));
    colours_dark.insert(Variant::POOL_BYTE_ARRAY, Color(0.67, 0.96, 0.78));
    colours_dark.insert(Variant::POOL_INT_ARRAY, Color(0.69, 0.86, 0.96));
    colours_dark.insert(Variant::POOL_REAL_ARRAY, Color(0.59, 0.91, 0.97));
    colours_dark.insert(Variant::POOL_STRING_ARRAY, Color(0.62, 0.77, 0.95));
    colours_dark.insert(Variant::POOL_VECTOR2_ARRAY, Color(0.82, 0.7, 0.96));
    colours_dark.insert(Variant::POOL_VECTOR3_ARRAY, Color(0.87, 0.61, 0.95));
    colours_dark.insert(Variant::POOL_COLOR_ARRAY, Color(0.91, 1.0, 0.59));

    colours_light.insert(Variant::NIL,Color(0.15, 0.89, 0.63));
    colours_light.insert(Variant::BOOL,Color(0.43, 0.56, 0.92));
    colours_light.insert(Variant::INT, Color(0.31, 0.7, 0.91));
    colours_light.insert(Variant::REAL,Color(0.15, 0.8, 0.94));
    colours_light.insert(Variant::STRING,Color(0.27, 0.56, 0.91));
    colours_light.insert(Variant::VECTOR2,Color(0.68, 0.46, 0.93));
    colours_light.insert(Variant::RECT2, Color(0.93, 0.46, 0.56));
    colours_light.insert(Variant::VECTOR3,Color(0.86, 0.42, 0.93));
    colours_light.insert(Variant::TRANSFORM2D,Color(0.59, 0.81, 0.1));
    colours_light.insert(Variant::PLANE,Color(0.97, 0.44, 0.44));
    colours_light.insert(Variant::QUAT,Color(0.93, 0.41, 0.64));
    colours_light.insert(Variant::AABB,Color(0.93, 0.47, 0.57));
    colours_light.insert(Variant::BASIS,Color(0.7, 0.73, 0.1));
    colours_light.insert(Variant::TRANSFORM, Color(0.96, 0.56, 0.28));
    colours_light.insert(Variant::COLOR,Color(0.24, 0.75, 0.0));
    colours_light.insert(Variant::NODE_PATH,Color(0.41, 0.58, 0.93));
    colours_light.insert(Variant::_RID,Color(0.17, 0.9, 0.45));
    colours_light.insert(Variant::OBJECT,Color(0.07, 0.84, 0.76));
    colours_light.insert(Variant::DICTIONARY,Color(0.34, 0.91, 0.62));
    colours_light.insert(Variant::ARRAY,Color(0.45, 0.45, 0.45));
    colours_light.insert(Variant::POOL_BYTE_ARRAY,Color(0.38, 0.92, 0.6));
    colours_light.insert(Variant::POOL_INT_ARRAY,Color(0.38, 0.73, 0.92));
    colours_light.insert(Variant::POOL_REAL_ARRAY,Color(0.25, 0.83, 0.95));
    colours_light.insert(Variant::POOL_STRING_ARRAY,Color(0.38, 0.62, 0.92));
    colours_light.insert(Variant::POOL_VECTOR2_ARRAY,Color(0.62, 0.36, 0.92));
    colours_light.insert(Variant::POOL_VECTOR3_ARRAY,Color(0.79, 0.35, 0.92));
    colours_light.insert(Variant::POOL_COLOR_ARRAY,Color(0.57, 0.73, 0.0));
}


static Color _color_from_type(Variant::Type p_type, bool dark_theme = true) {
	// Streamlined version using variant-type:colour mapping. 
    // TODO add an overrides check?
    _load_colours();
    if (dark_theme) {
        if (colours_dark.has(p_type))
        {
            return colours_dark[p_type];
        }else{
            Color color;
            color.set_hsv(p_type / float(Variant::VARIANT_MAX), 0.7, 0.7);
            return color;
        }
    }else{
        if (colours_dark.has(p_type))
        {
            return colours_light[p_type];
        }else{
            Color color;
            color.set_hsv(p_type / float(Variant::VARIANT_MAX), 0.3, 0.3);
            return color;
        }
    }
}


void C11REditor::add_tab_options(Control *root)
{
    Button *button_toggle_visibility = new Button("+/-");
    root->add_child(button_toggle_visibility);
    button_toggle_visibility->connect("pressed", this, "_toggle_inspector_visibility");
    HSeparator *hsep = new HSeparator();
    root->add_child(hsep);
    // TODO add tab options
}

C11REditor::C11REditor()
{
    if(!clipboard)
    {
        clipboard = memnew(Clipboard);
    }

    undo_redo = EditorNode::get_singleton()->get_undo_redo();
    set_process_input(true);
    set_process_unhandled_input(true);

    // TODO create editor graphics
    Panel *panel = new Panel();
    panel->set_h_size_flags(Control::SIZE_EXPAND_FILL);
    panel->set_v_size_flags(Control::SIZE_EXPAND_FILL);
    add_child(panel);

    HBoxContainer *hbox = new HBoxContainer();
    hbox->set_h_size_flags(Control::SIZE_EXPAND_FILL);
    hbox->set_v_size_flags(Control::SIZE_EXPAND_FILL);
    panel->add_child(hbox);


	GraphEdit *block_graph = new GraphEdit();
    block_graph->set_h_size_flags(Control::SIZE_EXPAND_FILL);
    block_graph->set_v_size_flags(Control::SIZE_EXPAND_FILL);
    hbox->add_child(block_graph);

    PanelContainer *inspection_panel = new PanelContainer();
    hbox->add_child(inspection_panel);

	VBoxContainer *tab_options = new VBoxContainer();
    hbox->add_child(tab_options);

    add_tab_options(tab_options);
}

C11REditor::~C11REditor()
{
    undo_redo->clear_history();
}

void C11REditor::_change_tab(Control *new_tab)
{
    if (new_tab == nullptr) return;
    int cur_children = inspection_panel->get_child_count();
    for(int i = 0; i < cur_children; i++)
    { // this should only eval to 1 child. But maybe there's a use case in the future
        Node* child = inspection_panel->get_child(0);
        inspection_panel->remove_child(child);
        child->queue_delete();
    }
    inspection_panel->add_child(new_tab);
}


void C11REditor::_toggle_inspector_visibility()
{
    inspection_panel->set_visible(!inspection_panel->is_visible());
}

static ScriptEditorBase *create_editor(const RES &p_resource) {
	if (Object::cast_to<C11RScript>(*p_resource)) {
        print_line("Creating a C11R editor!");
		return memnew(C11REditor);
	}
    print_line("Failed to create a C11REditor");
	return nullptr;
}

C11REditor::Clipboard *C11REditor::clipboard = nullptr;

void C11REditor::_bind_methods(){
    ClassDB::bind_method(D_METHOD("_toggle_inspector_visibility"), &C11REditor::_toggle_inspector_visibility);
}

void C11REditor::add_syntax_highlighter(SyntaxHighlighter *p_highlighter){}

void C11REditor::set_syntax_highlighter(SyntaxHighlighter *p_highlighter){}


void C11REditor::apply_code(){}

RES C11REditor::get_edited_resource() const
{
    return current_script; // FIXME
}

void C11REditor::set_edited_resource(const RES &p_res){
    current_script = p_res;
}

void C11REditor::enable_editor(){}

Vector<String> C11REditor::get_functions(){
    return Vector<String>(); // FIXME
}
void C11REditor::reload_text(){}

String C11REditor::get_name(){
    if (current_script.is_valid())
    {
        return current_script->get_path().get_file();
    }
    WARN_PRINT_ONCE("Null Script reference???");
    return "Null Script Reference?";
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
    print_line("Adding script editor callback");
    ScriptEditor::register_create_script_editor_function(&create_editor);

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
    print_line("Adding plugin init callback");
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