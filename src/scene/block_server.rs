use std::{collections::HashMap, rc::Rc};

use godot::{engine::Engine, prelude::*};

use crate::lang::core_blocks::{BlockCustom, IBlock};

struct BlockRegistry {
    block_instance: Option<Rc<Box<dyn IBlock>>>,
}

#[derive(GodotClass)]
#[class(base=Object)]
pub struct ChoreographerServer {
    instances: HashMap<String, BlockRegistry>,

    node: Base<Object>,
}

#[godot_api]
impl IObject for ChoreographerServer {
    fn init(node: Base<Object>) -> Self {
        Self {
            node,
            instances: HashMap::new(),
        }
    }
}

#[godot_api]
impl ChoreographerServer {
    pub fn register_custom(&mut self, script: Gd<Object>, name: String) -> bool {
        self.instances.insert(
            name,
            BlockRegistry {
                block_instance: Some(Rc::new(Box::new(BlockCustom::new(script)))),
            },
        );
        false
    }

    pub fn register(&mut self, block_instance: Box<dyn IBlock>, name: &str) -> bool {
        let sname = name.to_owned();
        if self.instances.contains_key(&sname) {
            return false;
        }
        self.instances.insert(
            sname,
            BlockRegistry {
                block_instance: Some(Rc::new(block_instance)),
            },
        );
        true
    }

    pub fn get_block(&self, name: String) -> Option<Rc<Box<dyn IBlock>>> {
        let Some(registry) = self.instances.get(&name) else {
            return None;
        };
        let Some(instance) = registry.block_instance.clone() else {
            return None;
        };
        Some(instance.clone())
    }

    pub fn singleton() -> Option<Gd<ChoreographerServer>> {
        let Some(single) =
            Engine::singleton().get_singleton(StringName::from(super::SINGLETON_C11R_SERVER))
        else {
            return None;
        };
        let cast_val: Result<Gd<ChoreographerServer>, _> = single.try_cast();
        if let Ok(val) = cast_val {
            Some(val)
        } else {
            None
        }
    }
}
