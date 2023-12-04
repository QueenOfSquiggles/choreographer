use godot::prelude::*;

// moving logic out of these limited files
// mod blocks;
// mod lang;
pub mod editor;
pub mod lang;
pub mod scene;
pub mod servers;

struct Choreographer;

#[gdextension]
unsafe impl ExtensionLibrary for Choreographer {
    fn on_level_init(level: InitLevel) {
        match level {
            InitLevel::Servers => servers::register(),
            InitLevel::Scene => {
                scene::register();
                lang::register();
            }
            InitLevel::Editor => editor::register(),
            _ => (),
        }
    }

    fn on_level_deinit(level: InitLevel) {
        match level {
            InitLevel::Servers => servers::unregister(),
            InitLevel::Scene => {
                scene::unregister();
                lang::register();
            }
            InitLevel::Editor => editor::unregister(),
            _ => (),
        }
    }
}

#[cfg(test)]
mod tests {
    // TODO: figure out what kinds of things can be tested without godot? (I don't believe there's very much!)
}
