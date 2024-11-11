use std::env;

use logger::Logger;
use nodes::Node;
use scripts::Script;
use types::{StringName, TypeRegistry};

#[cfg(feature = "stdlib")]
pub mod stdlib;

pub mod logger;
pub mod nodes;
pub mod scripts;
pub mod types;

#[derive(Debug, Clone)]
pub struct Environment {
    pub flags: Vec<StringName>,
    pub nodes: TypeRegistry<Node>,
    pub scripts: TypeRegistry<Script>,
    pub logger: Logger,
}

pub fn construct_environment() -> Environment {
    #[cfg(feature = "logging")]
    {
        // initialize the logging utility
        colog::init();
    }
    let mut cho_env = Environment {
        flags: Vec::new(),
        nodes: TypeRegistry::default(),
        scripts: TypeRegistry::default(),
        logger: Logger::new("choreoghrapher.log".into()),
    };
    for (key, value) in env::vars() {
        if key.to_lowercase().starts_with("cho_") && !value.to_lowercase().contains("false") {
            cho_env.flags.push(key.into());
        }
    }
    if cfg!(feature = "stdlib") {
        stdlib::register(&mut cho_env.nodes);
    }
    cho_env
}
