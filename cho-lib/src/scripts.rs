use std::{collections::HashMap, sync::Arc};

use log::warn;

use crate::{
    nodes::{Node, NodeData, NodeError},
    types::{GlobalName, NamespacedType, StringName, Var},
    Environment,
};

#[derive(Debug, Clone)]
pub struct Script {
    name: GlobalName,
    nodes: Vec<GlobalName>,
    funcs: HashMap<StringName, Function>,
    vars: HashMap<StringName, Var>,
    blackboard: HashMap<StringName, Var>,
}

#[derive(Debug, Clone)]
pub struct Function {
    nodes: Vec<Arc<Node>>,
    entry: usize,
    routing: Vec<Connection>,
}

#[derive(Debug, Clone)]
pub struct Connection {
    value: Var,
    from: usize,
    to: usize,
    from_param: StringName,
    to_param: StringName,
}

#[derive(Default, Clone, Debug)]
struct FrameResults {
    vars: HashMap<StringName, Var>,
    blackboard: HashMap<StringName, Var>,
    next_nodes: Vec<usize>,
}

impl NamespacedType for Script {
    fn get_name(&self) -> crate::types::GlobalName {
        self.name.clone()
    }
}

impl Script {
    pub fn call_func(
        &mut self,
        func_name: StringName,
        env: Arc<Environment>,
        inputs: HashMap<StringName, Var>,
        call_stack: &mut Vec<Arc<Node>>,
    ) -> Result<HashMap<StringName, Var>, NodeError> {
        let Some(func) = self.funcs.get(&func_name) else {
            return Err(NodeError::TypeNotFound {
                name: GlobalName::from_path(func_name.to_string()),
                msg: format!("Function not found on script: {:?}", self.name),
            });
        };
        self.blackboard = inputs;
        let Some(entry) = func.nodes.get(func.entry) else {
            return Err(NodeError::Unhandled(format!("Failed to find entry node for function {:?}::{:?} at index {} of node array with {} elements", self.name, func_name, func.entry, func.nodes.len())));
        };
        call_stack.push(entry.clone());
        while !call_stack.is_empty() {
            let Some(top) = call_stack.pop() else {
                break;
            };
            let results =
                self.execute_frame(func, func_name.clone(), env.clone(), func.entry, top)?;
            for var in results.vars {
                self.vars.insert(var.0, var.1);
            }
            self.blackboard = results.blackboard;
            for index in results.next_nodes {
                let Some(node) = func.nodes.get(func.entry) else {
                    return Err(NodeError::Unhandled(format!("Failed to find next node for function {:?}::{:?} at index {} of node array with {} elements", self.name, func_name, index, func.nodes.len())));
                };
                call_stack.push(node.clone());
            }
        }

        Ok(HashMap::new())
    }

    fn execute_frame(
        &self,
        func: &Function,
        func_name: StringName,
        env: Arc<Environment>,
        index: usize,
        node: Arc<Node>,
    ) -> Result<FrameResults, NodeError> {
        let mut frame_results = FrameResults::default();
        let mut next_frame = node.execute(env.clone(), self.blackboard.clone())?;
        let outputs = func
            .routing
            .iter()
            .filter(|p| p.from == index)
            .collect::<Vec<_>>();
        for conn in &outputs {
            frame_results.blackboard.insert(
                conn.to_param.clone(),
                match next_frame.get(&conn.from_param) {
                    Some(var) => var.clone(),
                    None => Var::Null,
                },
            );
            next_frame.remove(&conn.from_param);
        }

        for (key, value) in next_frame {
            if let Var::Execution(enabled) = value {
                if enabled {
                    let Some(con) = outputs
                        .iter()
                        .filter(|p| p.from_param == key)
                        .cloned()
                        .next()
                    else {
                        env.logger.info(format!("Dangling execution route enabled without any valid target at {:?}::{:?}::{}. This may not be a problem", self.name, func_name, index));
                        continue;
                    };
                    frame_results.next_nodes.push(con.to);
                }
            } else {
                if self.vars.contains_key(&key) {
                    frame_results.vars.insert(key, value);
                } else {
                    frame_results.blackboard.insert(key, value);
                }
            }
        }
        Ok(frame_results)
    }
}

#[cfg(test)]
mod test {
    use std::{
        collections::{hash_map, HashMap},
        hash::Hash,
        sync::Arc,
    };

    use crate::types::{GlobalName, Var};

    use super::{Connection, Function, Script};

    #[test]
    fn test_script() {
        let env = crate::construct_environment();

        let mut script = Script {
            name: GlobalName::from_path("test.script"),
            nodes: vec![
                GlobalName::from_path("std.math.add"),
                GlobalName::from_path("std.math.subtract"),
            ],
            funcs: HashMap::new(),
            vars: HashMap::new(),
            blackboard: HashMap::new(),
        };
        script.funcs.insert(
            "func".into(),
            Function {
                nodes: vec![],
                entry: 0,
                routing: vec![Connection {
                    value: Var::Null,
                    from: 0,
                    to: 1,
                    from_param: "c".into(),
                    to_param: "a".into(),
                }],
            },
        );
        let mut stack = Vec::new();
        let result = script.call_func("func".into(), Arc::new(env), HashMap::new(), &mut stack);
        // TODO run assertions for testing purposes
    }
}
