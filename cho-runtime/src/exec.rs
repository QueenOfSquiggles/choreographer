use std::{collections::HashMap, fs::File, path::PathBuf, sync::Arc};

use cho_lib::{
    filetype::ScriptProto,
    nodes::{Node, NodeData, NodeError, ScriptNode},
    types::{GlobalName, Var, VarRegisters},
    Environment,
};
use serde::{Deserialize, Serialize};

use crate::project::ProjectFile;

pub struct Execution {
    env: Environment,
    entry: GlobalName,
    call_stack: Vec<Arc<Node>>,
    config: ProjectFile,
}

#[derive(Serialize, Deserialize, Debug, Clone)]
pub struct ExecutableConfig {
    entry: String,
    start_frame_data: HashMap<String, Var>,
}

impl Execution {
    pub fn new(env: Environment, entry: GlobalName, config: &ProjectFile) -> Self {
        Self {
            env,
            entry: if entry.is_empty() {
                GlobalName::from_path(config.executable.clone().unwrap_or_default().entry)
            } else {
                entry
            },
            call_stack: Default::default(),
            config: config.clone(),
        }
    }

    pub fn run(&mut self) -> Result<VarRegisters, (NodeError, Vec<Arc<Node>>)> {
        if !self.env.nodes.contains(&self.entry) {
            let target = self.entry.clone();
            if let Some(script) = self.try_get_script_for(&target) {
                self.env
                    .logger
                    .debug(format!("Loading script for name: {:?}", target));
                self.env.nodes.register(Node::Script(script));
            }
        }
        let Some(entry) = self.env.nodes.get(&self.entry) else {
            return Err((NodeError::TypeNotFound {
                name: self.entry.clone(),
                msg: "Failed to load entry type. Make sure your entry point name matches the name that is being called by the runtime."
                    .into(),
            }, Vec::new()));
        };
        self.call_stack.push(entry);

        let aenv = Arc::new(self.env.clone());
        let mut frame_data = VarRegisters::new();
        if let Some(exec_data) = &self.config.executable {
            for (k, v) in &exec_data.start_frame_data {
                frame_data.0.insert(k.clone().into(), v.clone());
            }
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

    fn try_get_script_for(&mut self, name: &GlobalName) -> Option<ScriptNode> {
        let mut fpath = PathBuf::new();
        for element in name.to_path().split('.') {
            fpath.push(element);
        }
        fpath.set_extension("cho");
        if !fpath.exists() {
            return None;
        }
        let Ok(file) = File::open(fpath) else {
            return None;
        };
        let Ok(proto) = ron::de::from_reader::<_, ScriptProto>(file) else {
            return None;
        };

        self.env.scripts.register(proto.to_script(&self.env.nodes));

        match self.env.scripts.get(name) {
            Some(script) => Some(ScriptNode {
                name: name.clone(),
                func: "main".into(),
                script,
            }),
            None => None,
        }
    }
}

impl Default for ExecutableConfig {
    fn default() -> Self {
        let mut frame = HashMap::new();
        frame.insert("a".into(), Var::Bool(true));
        frame.insert("b".into(), Var::String("A string".into()));
        frame.insert("c".into(), Var::Num(3.1415));

        Self {
            entry: "main".into(),
            start_frame_data: frame,
        }
    }
}
