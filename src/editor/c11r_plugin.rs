use godot::{
    engine::{EditorPlugin, IEditorPlugin, ResourceLoader, Texture2D, ThemeDb},
    prelude::*,
};

use crate::common;

#[derive(GodotClass)]
#[class(tool,init,base=EditorPlugin)]
pub struct Class {
    base: Base<EditorPlugin>,
}

#[godot_api]
impl IEditorPlugin for Class {
    fn enter_tree(&mut self) {
        //pass
        // self.base_mut().add_control_to_container(, control)
        if let Some(theme) = &mut ThemeDb::singleton().get_default_theme() {
            if let Some(icon) =
                ResourceLoader::singleton().load(common::PLUGIN_ICON_PATH.to_godot())
            {
                if let Ok(texture) = icon.try_cast::<Texture2D>() {
                    theme.set_icon(
                        StringName::from(common::PLUGIN_NAME),
                        StringName::from("EditorIcons"),
                        texture,
                    )
                }
            }
        }
    }

    fn exit_tree(&mut self) {
        //pass
    }
}
