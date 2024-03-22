use godot::{
    engine::{
        control::LayoutPreset, EditorInterface, EditorPlugin, IEditorPlugin, ResourceLoader,
        Texture2D, ThemeDb,
    },
    prelude::*,
};

use crate::{common, lang::core_lang::script::ChoreographerScript};

use super::editor_gui::C11REditorGUI;

#[derive(GodotClass)]
#[class(tool,init, editor_plugin,base=EditorPlugin)]
pub struct ChoreographerEditorPlugin {
    gui: Option<Gd<C11REditorGUI>>,
    base: Base<EditorPlugin>,
}

#[godot_api]
impl IEditorPlugin for ChoreographerEditorPlugin {
    fn ready(&mut self) {
        if let Err(msg) = Self::try_register_icon(common::PLUGIN_NAME, common::PLUGIN_ICON_PATH) {
            godot_warn!("Encountered an error loading plugin icon: {}", msg);
        }
        let mut gui = C11REditorGUI::new_alloc();
        self.gui = Some(gui.clone());
        if let Some(main) = &mut EditorInterface::singleton().get_editor_main_screen() {
            main.add_child(self.gui.clone().unwrap().upcast());
        }
        gui.set_anchors_and_offsets_preset(LayoutPreset::FULL_RECT);
        self.make_visible(false);
    }

    fn handles(&self, object: Gd<Object>) -> bool {
        // tells godot to open this menu when an object of that type is selected
        object.try_cast::<ChoreographerScript>().is_ok()
    }

    fn has_main_screen(&self) -> bool {
        true
    }

    fn make_visible(&mut self, visible: bool) {
        if let Some(gui) = &mut self.gui {
            gui.set_visible(visible);
        }
    }

    fn get_plugin_icon(&self) -> Option<Gd<Texture2D>> {
        if let Some(res) = ResourceLoader::singleton().load(common::PLUGIN_ICON_PATH.to_godot()) {
            return match res.try_cast::<Texture2D>() {
                Ok(value) => Some(value),
                Err(_) => None,
            };
        }
        None
    }

    fn get_plugin_name(&self) -> GString {
        common::PLUGIN_NAME.to_godot()
    }

    fn exit_tree(&mut self) {
        if let Some(gui) = &mut self.gui {
            gui.queue_free();
        }
    }
}

impl ChoreographerEditorPlugin {
    fn try_register_icon(name: &str, path: &str) -> Result<(), String> {
        let Some(theme) = &mut ThemeDb::singleton().get_default_theme() else {
            return Err(format!(
                "Failed to load default theme when loading icon {} from {}",
                name, path
            ));
        };
        let Some(icon) = ResourceLoader::singleton().load(common::PLUGIN_ICON_PATH.to_godot())
        else {
            return Err(format!(
                "Failed to load resource file for icon name {}, path= {}",
                name, path
            ));
        };
        let Ok(texture) = icon.try_cast::<Texture2D>() else {
            return Err(format!(
                "Failed to cast resource to type of Texture2D for icon {}, path= {}",
                name, path
            ));
        };
        theme.set_icon(
            StringName::from(common::PLUGIN_NAME),
            StringName::from("EditorIcons"),
            texture,
        );

        Ok(())
    }
}
