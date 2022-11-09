#ifndef C11R_EDITOR_H
#define C11R_EDITOR_H

#include "editor/create_dialog.h"
#include "editor/plugins/script_editor_plugin.h"
#include "editor/property_editor.h"
#include "scene/gui/graph_edit.h"

#include "../c11r_lang.hpp"
#include "../packs/block_pack.hpp"

// FIXME remove before release builds
// #define TOOLS_ENABLED

#ifdef TOOLS_ENABLED

class C11REditor: public ScriptEditorBase
{
    GDCLASS(C11REditor, ScriptEditorBase);

    Ref<C11RScript> script; // FIXME delete (see current_script below)

    UndoRedo *undo_redo;

    struct Clipboard
    {
        String temp;
    };

    static Clipboard *clipboard;

	RES current_script;

	PanelContainer *inspection_panel;
	GraphEdit *block_graph;
	VBoxContainer *tab_options;
	HBoxContainer *edit_menu;

	PopupMenu *popup_add_block;

	VBoxContainer* side_panel;

	PopupPanel *popup_option_set_base_script;

protected:
	void _change_tab(Control *new_tab);
	void _add_block_by_id(int id);
	void _graph_gui_input(const Ref<InputEvent> &p_event);
	void _option_menu_item(int index);
	void _popup_option_change_base_script();

    static void _bind_methods();

public:
	void _toggle_inspector_visibility();

    virtual void add_syntax_highlighter(SyntaxHighlighter *p_highlighter);
	virtual void set_syntax_highlighter(SyntaxHighlighter *p_highlighter);

	virtual void apply_code();
	virtual RES get_edited_resource() const;
	virtual void set_edited_resource(const RES &p_res);
	virtual void enable_editor();
	virtual Vector<String> get_functions();
	virtual void reload_text();
	virtual String get_name();
	virtual Ref<Texture> get_icon();
	virtual bool is_unsaved();
	virtual Variant get_edit_state();
	virtual void set_edit_state(const Variant &p_state);
	virtual void goto_line(int p_line, bool p_with_error = false);
	virtual void set_executing_line(int p_line);
	virtual void clear_executing_line();
	virtual void trim_trailing_whitespace();
	virtual void insert_final_newline();
	virtual void convert_indent_to_spaces();
	virtual void convert_indent_to_tabs();
	virtual void ensure_focus();
	virtual void tag_saved_version();
	virtual void reload(bool p_soft);
	virtual void get_breakpoints(List<int> *p_breakpoints);
	virtual void add_callback(const String &p_function, PoolStringArray p_args);
	virtual void update_settings();
	virtual bool show_members_overview();
	virtual void set_debugger_active(bool p_active);
	virtual void set_tooltip_request_func(String p_method, Object *p_obj);
	virtual Control *get_edit_menu();
	virtual void clear_edit_menu();
	virtual bool can_lose_focus_on_node_selection() { return false; }
	virtual void validate();

	void _input(const Ref<InputEvent> &p_event);

	static void register_editor();

	static void free_clipboard();

    C11REditor();
    ~C11REditor();

};


class _C11REditor : public Object
{
    GDCLASS(_C11REditor, Object);
    friend class C11RScriptLanguage;

protected:
    static void _bind_methods();
    static _C11REditor *singleton;

    static Map<String, RefPtr> custom_blocks;
    static Ref<Block> create_block_custom(const String &p_name);
public:
    static _C11REditor *get_singleton() { return singleton; }

    void add_custom_block(const String &p_name, const String &p_namespace, const Ref<Script> & p_script);
    void remove_custom_block(const String &p_name, const String &p_category);
	void register_block_pack(const Ref<BlockPack> &p_block_pack);
	void unregister_block_pack(const Ref<BlockPack> &p_block_pack);

	_C11REditor();
    ~_C11REditor();
};

#endif // TOOLS_ENABLED

#endif // C11R_EDITOR_H