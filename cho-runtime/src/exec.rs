use std::{collections::HashMap, fs::File, sync::Arc};

use cho_lib::{
    nodes::{Node, NodeData, NodeError},
    types::{GlobalName, StringName, Var},
    Environment,
};
use serde::{Deserialize, Serialize};

pub const CONFIG_FILE: &'static str = "choreo.ron";

pub struct Execution {
    env: Environment,
    entry: GlobalName,
    call_stack: Vec<Arc<Node>>,
    config: ExecutableConfig,
}

#[derive(Serialize, Deserialize, Default, Debug, Clone)]
pub struct ExecutableConfig {
    entry: String,
    start_frame_data: HashMap<String, Var>,
}

impl Execution {
    pub fn new(env: Environment, entry: GlobalName) -> Self {
        let config: ExecutableConfig = match File::open(CONFIG_FILE) {
            Ok(file) => ron::de::from_reader::<_, ExecutableConfig>(file).unwrap_or_default(),
            Err(_) => ExecutableConfig::default(),
        };
        Self {
            env,
            entry: if entry.is_empty() {
                GlobalName::from_path(config.clone().entry)
            } else {
                entry
            },
            call_stack: Default::default(),
            config,
        }
    }

    pub fn run(&mut self) -> Result<HashMap<StringName, Var>, (NodeError, Vec<Arc<Node>>)> {
        let Some(entry) = self.env.nodes.get(&self.entry) else {
            return Err((NodeError::TypeNotFound {
                name: self.entry.clone(),
                msg: "Failed to load entry type. Make sure your entry point name matches the name that is being called by the runtime."
                    .into(),
            }, Vec::new()));
        };
        self.call_stack.push(entry);

        let aenv = Arc::new(self.env.clone());
        let mut frame_data = HashMap::<StringName, Var>::new();
        for (k, v) in &self.config.start_frame_data {
            frame_data.insert(k.clone().into(), v.clone());
        }

        while !self.call_stack.is_empty() {
            let Some(top) = self.call_stack.pop() else {
                return Err((NodeError::Unhandled("Failed to pop call stack. This should never happen and is a seriousc failiure of the core logic of this execution environment. Seek solace in whatever god(s) you pray to, though even they may not be able to save you.".into()), self.call_stack.clone()));
            };
            match top.execute(aenv.clone(), frame_data) {
                Ok(data) => frame_data = data,
                Err(error) => {
                    self.call_stack.push(top); // restore last called
                    return Err((error, self.call_stack.clone()));
                }
            }
        }

        Ok(frame_data)
    }
}
