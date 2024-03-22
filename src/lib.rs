use godot::prelude::*;

// moving logic out of these limited files
// mod blocks;
// mod lang;
pub mod common;
pub mod editor;
pub mod lang;
pub mod scene;

struct Choreographer;

#[gdextension]
unsafe impl ExtensionLibrary for Choreographer {
    fn on_level_init(level: InitLevel) {
        match level {
            InitLevel::Scene => {
                lang::register();
                scene::register();
            }
            InitLevel::Editor => editor::register(),
            _ => (),
        }
    }

    fn on_level_deinit(level: InitLevel) {
        match level {
            InitLevel::Scene => {
                scene::unregister();
                lang::unregister();
            }
            InitLevel::Editor => editor::unregister(),
            _ => (),
        }
    }
}
