// Registers the editor-only features that will not be available in release environments

// use godot::engine::{EditorInterface, Engine, Script, ScriptEditor};
mod c11r_plugin;
mod editor_gui;

pub fn register() {
    // let mut editor_option = EditorInterface::singleton().get_script_editor();
    // let Some(mut editor) = editor_option else {
    //     return;
    // };
}
pub fn unregister() {}
