// Register the medium-high level systems that are necessary for the language while still depending on many other core godot systems

use godot::{builtin::StringName, engine::Engine, obj::NewAlloc};

use crate::lang::core_blocks::BlockIfElse;

use self::block_server::ChoreographerServer;

pub mod block_server;

pub const SINGLETON_C11R_SERVER: &str = "ChoreographerServer";

pub fn register() {
    let mut server = ChoreographerServer::new_alloc();
    {
        // register internal block definitions
        let mut sbind = server.bind_mut();
        sbind.register(Box::new(BlockIfElse::new()), "IfElse");
    }
    Engine::singleton()
        .register_singleton(StringName::from(SINGLETON_C11R_SERVER), server.upcast());
}

pub fn unregister() {
    Engine::singleton().unregister_singleton(StringName::from(SINGLETON_C11R_SERVER));
}
