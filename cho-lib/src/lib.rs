use std::env;

use logger::Logger;
use types::{StringName, TypeRegistry};

#[cfg(feature = "stdlib")]
pub mod stdlib;

pub mod logger;
pub mod types;

#[derive(Debug, Clone)]
pub struct Environment {
    pub flags: Vec<StringName>,
    pub registry: TypeRegistry,
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
        registry: TypeRegistry::default(),
        logger: Logger::new("choreoghrapher.log".into()),
    };
    for (key, value) in env::vars() {
        if key.to_lowercase().starts_with("cho_") && !value.to_lowercase().contains("false") {
            cho_env.flags.push(key.into());
        }
    }
    if cfg!(feature = "stdlib") {
        stdlib::register(&mut cho_env.registry);
    }
    cho_env
}
