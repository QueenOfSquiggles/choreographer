// provide core systems for the language itself

use godot::{
    engine::{Engine, ResourceLoader, ResourceSaver},
    obj::UserClass,
};

use self::{
    core_lang::ChoreographerLang,
    core_resource::{ChoreographerLoader, ChoreographerSaver},
};

mod core_blocks;
mod core_lang;
mod core_resource;
pub fn register() {
    Engine::singleton().register_script_language(ChoreographerLang::alloc_gd().upcast());
    ResourceLoader::singleton().add_resource_format_loader(ChoreographerLoader::new_gd().upcast());
    ResourceSaver::singleton().add_resource_format_saver(ChoreographerSaver::new_gd().upcast());
}
pub fn unregister() {
    if let Some(lang) = ChoreographerLang::singleton() {
        Engine::singleton().unregister_script_language(lang.upcast());
    }
}
