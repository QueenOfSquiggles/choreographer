use godot::prelude::*;

mod blocks;
mod lang;

struct Choreographer;

#[gdextension]
unsafe impl ExtensionLibrary for Choreographer {
    // fn editor_run_behavior() -> godot::init::EditorRunBehavior {
    //     godot::init::EditorRunBehavior::ToolClassesOnly
    // }

    // fn min_level() -> InitLevel {
    //     InitLevel::Scene
    // }

    fn on_level_init(_level: InitLevel) {
        match _level {
            // available in runtime builds
            InitLevel::Scene => lang::init_lang(),
            // available only in editor builds (not during exported runtime)
            // InitLevel::Editor => ,
            _ => (),
        }
    }

    fn on_level_deinit(_level: InitLevel) {
        match _level {
            // available in runtime builds
            InitLevel::Scene => lang::deinit_lang(),
            // available only in editor builds (not during exported runtime)
            // InitLevel::Editor => todo!(),
            _ => (),
        }
    }
}

#[cfg(test)]
mod tests {
    // use super::*;

    // #[test]
    // fn it_works() {
    //     let result = add(2, 2);
    //     assert_eq!(result, 4);
    // }
}
