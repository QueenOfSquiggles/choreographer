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

impl Environment {
    pub fn new() -> Self {
        let mut cho_env = Self::new_empty();

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

    pub fn new_empty() -> Self {
        Self {
            flags: Vec::new(),
            nodes: TypeRegistry::default(),
            scripts: TypeRegistry::default(),
            logger: Logger::new("choreoghrapher.log".into()),
        }
    }
}
