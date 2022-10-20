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
    
    Panel *panel = memnew(Panel); // Panel seems redundant, but it lets us have child nodes that exist outside of bounds as needed.
    add_child(panel);
    panel->set_h_size_flags(Control::SIZE_EXPAND_FILL);
    panel->set_v_size_flags(Control::SIZE_EXPAND_FILL);
    panel->set_anchors_and_margins_preset(Control::PRESET_WIDE);
    
    HBoxContainer *hbox = memnew(HBoxContainer);
    panel->add_child(hbox);
    hbox->set_h_size_flags(Control::SIZE_EXPAND_FILL);
    hbox->set_v_size_flags(Control::SIZE_EXPAND_FILL);
    hbox->set_anchors_and_margins_preset(Control::PRESET_WIDE);

	block_graph = memnew(GraphEdit);
    hbox->add_child(block_graph);
    block_graph->set_h_size_flags(Control::SIZE_EXPAND_FILL);
    block_graph->set_v_size_flags(Control::SIZE_EXPAND_FILL);
    block_graph->set_anchors_and_margins_preset(Control::PRESET_WIDE);
    block_graph->connect("gui_input", this, "_graph_gui_input");

    inspection_panel = memnew(PanelContainer);
    inspection_panel->set_h_size_flags(Control::SIZE_SHRINK_END); // shrink to minimum size for max graph visibility
    inspection_panel->set_v_size_flags(Control::SIZE_EXPAND_FILL);
    hbox->add_child(inspection_panel);
    { // add some debug stuff to panel container
        VBoxContainer *vbox = memnew(VBoxContainer);
        inspection_panel->add_child(vbox);

        Label *lbl = memnew(Label);
        lbl->set_text("Inspection Panel");
        vbox->add_child(lbl);

        Button *btn_do_nothing = memnew(Button);
        btn_do_nothing->set_text("This button does nothing");
        vbox->add_child(btn_do_nothing);

        EditorSpinSlider *ed_spin_slider = memnew(EditorSpinSlider);
        ed_spin_slider->set_value(10.0);
        vbox->add_child(ed_spin_slider);
    }

	tab_options = memnew(VBoxContainer);
    hbox->add_child(tab_options);


    // TODO hook up all the signals needed
    // block_graph signals

    // tab_options signals
    Button *button_toggle_visibility = memnew(Button);
    button_toggle_visibility->set_custom_minimum_size((Size2){16.0f, 16.0f});
    button_toggle_visibility->set_icon_align(Button::TextAlign::ALIGN_CENTER);
    button_toggle_visibility->set_icon(Control::get_icon("icon_GUI_visibility_visible", "EditorIcons"));
    // TODO figure out why the icon isn't loading?? Or at least not visible?
    button_toggle_visibility->set_text("+");
    button_toggle_visibility->connect("pressed", this, "_toggle_inspector_visibility");
    tab_options->add_child(button_toggle_visibility);

    HSeparator *hsep = memnew(HSeparator);
    tab_options->add_child(hsep);
 
    // TODO add more tab options


    //TODO create popups

    popup_add_block = memnew(PopupMenu);
    panel->add_child(popup_add_block);
    popup_add_block->add_item("Item 1", 0);
    popup_add_block->add_item("Item 2", 1);
    popup_add_block->add_item("Item 3", 2);
    popup_add_block->add_item("Item 4", 3);
    popup_add_block->add_item("Item 5", 4);
    popup_add_block->connect("id_pressed", this, "_add_block_by_id");


}

C11REditor::~C11REditor()
{
    undo_redo->clear_history();
}

void C11REditor::_add_block_by_id(int id)
{
    print_line(vformat("Adding block id: %d", id));

    Ref<BlockEntry> entry;
    entry.instance();
    //script->add_node("Test Block", script->get_available_id(), entry);
}


void C11REditor::_change_tab(Control *new_tab)
{
    if (new_tab == nullptr) return;
    int cur_children = inspection_panel->get_child_count();
    for(int i = 0; i < cur_children; i++)
    { // this should only eval to 1 child. But maybe there's a use case in the future?
        Node* child = inspection_panel->get_child(0);
        inspection_panel->remove_child(child);
        child->queue_delete();
    }
    inspection_panel->add_child(new_tab);
}

void C11REditor::_input(const Ref<InputEvent> &p_event) {
    // TODO is this function needed for anything?
}

void C11REditor::_graph_gui_input(const Ref<InputEvent> &p_event) {
    bool do_add_block = false; 
	{ // right click -> add block
        Ref<InputEventMouseButton> key = p_event;
        do_add_block = (key.is_valid() && key->is_pressed() && key->get_button_index() == BUTTON_MASK_RIGHT);
    }
    { // shift+A -> add block
        Ref<InputEventKey> key = p_event;
        do_add_block = do_add_block || (key.is_valid() && key->get_shift() && key->get_scancode() == KEY_A);
    }

    if(do_add_block)
    { // flag met, perform process
        Rect2 bounds = popup_add_block->get_rect();
        bounds.position = get_global_mouse_position();
        popup_add_block->popup(bounds);
    }
}

void C11REditor::_toggle_inspector_visibility()
{
    if(inspection_panel == nullptr) return;

    bool vis = inspection_panel->is_visible();
    inspection_panel->set_visible(!vis);
}

static ScriptEditorBase *create_editor(const RES &p_resource) {
	if (Object::cast_to<C11RScript>(*p_resource)) {
        return memnew(C11REditor);
	}
	return nullptr;
}

C11REditor::Clipboard *C11REditor::clipboard = nullptr;

void C11REditor::_bind_methods(){

    ClassDB::bind_method(D_METHOD("_toggle_inspector_visibility"), &C11REditor::_toggle_inspector_visibility);
    ClassDB::bind_method(D_METHOD("_input"), &C11REditor::_input);
    ClassDB::bind_method(D_METHOD("_add_block_by_id"), &C11REditor::_add_block_by_id);
    ClassDB::bind_method(D_METHOD("_graph_gui_input"), &C11REditor::_graph_gui_input);
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
    return Control::get_icon("c11r_lang", "EditorIcons");
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
	ED_SHORTCUT("choreographer/add_block", TTR("Add Block"), KEY_MASK_SHIFT + KEY_A);
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
Map<String, RefPtr> _C11REditor::custom_blocks;


Ref<Block> _C11REditor::create_block_custom(const String &p_name) {
    RefPtr ref_ptr_script = singleton->custom_blocks[p_name];
    Ref<Script> script(ref_ptr_script);
    if (script->get_language() == C11RScriptLanguage::singleton)
    {
        Ref<C11RScript> c11r_script = Object::cast_to<C11RScript>(script.ptr());
        if(c11r_script.is_null()){
            ERR_PRINT(vformat("ERROR in creating custom block \"%s\". Failed to cast script to a C11RScript reference. Potential module error.", p_name));
        } else if(c11r_script->is_sub_graph) {
            // load as a sub-graph
            Ref<BlockSubGraph> block;
            block.instance();
            block->set_script(ref_ptr_script);
            return block;
        } else {
            // load as a composite graph
            Ref<BlockComposite> block;
            block.instance();
            block->set_script(ref_ptr_script);
            return block;
        }
    } else if(script->get_instance_base_type() == "Block"){
        // custom block (usually GDScript)
        Ref<BlockCustom> block;
        block.instance();
        block->set_script(ref_ptr_script);
        return block;
    } else {
        // Unrecognized format. Not subgraph/composite and not custom block definition     
        // TODO create implementation for specific classes targeted at these types of blocks   
        WARN_PRINT(vformat("Unrecognized script format for \"%s\". Should either be a Choreographer script, or a traditional script that extends `Block`.", p_name));
    }
    return nullptr;

	// Ref<Block> node;
	// node.instance();
	// node->set_script(ref_ptr_script);
	// return node;
}

void _C11REditor::register_block_pack(const Ref<BlockPack> &p_block_pack)
{
    Array scripts = p_block_pack->custom_block_scripts;
    for(int i = 0; i < scripts.size(); i++)
    {
        Variant var = scripts[i];

        if(var.get_type() == Variant::NIL || var.get_type() != Variant::OBJECT) continue;
        Ref<Script> script = Object::cast_to<Script>(var);
        String block_name = "unnamed block";
        if (script->get_language() == C11RScriptLanguage::singleton)
        {
            // add as a sub-graph block
            block_name = script->get_path().get_file(); // auto generate to the filename?
            // TODO implement a built-in script class name property to C11RScripts
        }else {
            if(!(script->get_instance_base_type() == "Block")) continue; // skip if not inheriting from block?
            // TODO verify that this works ^^

            block_name = script->get("block_namespace");
            // add as a custom block

        }
        // after finding the name, provide it to the registry
        add_custom_block(block_name, p_block_pack->namespace_prefix, script);
    }
}
void _C11REditor::unregister_block_pack(const Ref<BlockPack> &p_block_pack)
{
    Array scripts = p_block_pack->custom_block_scripts;
    for(int i = 0; i < scripts.size(); i++)
    {
        Variant var = scripts[i];

        if(var.get_type() == Variant::NIL || var.get_type() != Variant::OBJECT) continue;
        Ref<Script> script = Object::cast_to<Script>(var);
        String block_name = "unnamed block";
        if (script->get_language() == C11RScriptLanguage::singleton)
        {
            // add as a sub-graph block
            block_name = script->get_path().get_file(); // auto generate to the filename?
            // TODO implement a built-in script class name property to C11RScripts
        }else {
            if(!(script->get_instance_base_type() == "Block")) continue; // skip if not inheriting from block?
            // TODO verify that this works ^^

            block_name = script->get("block_namespace");
            // add as a custom block

        }
        // after finding the name, provide it to the registry
        remove_custom_block(block_name, p_block_pack->namespace_prefix);
    }
}

void _C11REditor::add_custom_block(const String &p_name, const String &p_namespace, const Ref<Script> &p_script) {
	String node_name = p_namespace + "." + p_name;
	custom_blocks.insert(node_name, p_script.get_ref_ptr());
	C11RScriptLanguage::singleton->add_register_func(node_name, &_C11REditor::create_block_custom);
	emit_signal("custom_nodes_updated");
}

void _C11REditor::remove_custom_block(const String &p_name, const String &p_namespace) {
	String node_name = p_namespace + "." + p_name;
	custom_blocks.erase(node_name);
	C11RScriptLanguage::singleton->remove_register_func(node_name);
	emit_signal("custom_blocks_updated");
}
_C11REditor::_C11REditor()
{
    singleton = this;
}
_C11REditor::~_C11REditor()
{
}

void _C11REditor::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("register_block_pack", "block_pack"), &_C11REditor::register_block_pack);
	ClassDB::bind_method(D_METHOD("unregister_block_pack", "block_pack"), &_C11REditor::unregister_block_pack);
	
    ClassDB::bind_method(D_METHOD("add_custom_block", "name", "category", "script"), &_C11REditor::add_custom_block);
	ClassDB::bind_method(D_METHOD("remove_custom_block", "name", "category"), &_C11REditor::remove_custom_block);
	ADD_SIGNAL(MethodInfo("custom_blocks_updated"));
}


#endif