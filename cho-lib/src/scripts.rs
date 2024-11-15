use std::{
    collections::{HashMap, VecDeque},
    fmt::Debug,
    sync::Arc,
};

use crate::{
    nodes::{Node, NodeData, NodeError},
    types::{GlobalName, NamespacedType, StringName, TypeRegistry, Var, VarRegisters},
    Environment,
};

#[derive(Debug, Clone)]
pub struct Script {
    pub name: GlobalName,
    pub funcs: HashMap<StringName, Function>,
}

#[derive(Debug, Clone)]
pub struct Function {
    // TODO: refactor node storage to store each unique node on the script and only store names on the function itself, which should reduce memory usage
    pub nodes: Vec<FunctionNode>,
    pub entry: usize,
    pub routing: Vec<Connection>,
}

#[derive(Debug, Clone)]
pub struct Connection {
    /// Cached value for this connection
    pub value: Var,
    pub from: usize,
    pub to: usize,
    pub from_param: StringName,
    pub to_param: StringName,
}

#[derive(Clone, PartialEq)]
pub struct FunctionNode {
    pub index: usize,
    pub node: Arc<Node>,
}

#[derive(Default, Clone, Debug)]
struct FrameResults {
    blackboard: VarRegisters,
    next_nodes: Vec<usize>,
}

impl Connection {
    pub fn new(
        from: usize,
        to: usize,
        from_param_name: impl Into<StringName>,
        to_param_name: impl Into<StringName>,
    ) -> Self {
        Self {
            value: Var::Null,
            from,
            to,
            from_param: from_param_name.into(),
            to_param: to_param_name.into(),
        }
    }
}

impl Function {
    pub fn new(
        registry: &TypeRegistry<Node>,
        nodes: Vec<GlobalName>,
        entry: usize,
        routing: Vec<Connection>,
    ) -> Self {
        Function {
            nodes: nodes
                .iter()
                .filter_map(|type_name| registry.get(type_name))
                .enumerate()
                .map(|(index, node)| FunctionNode { index, node })
                .collect(),

            entry,
            routing,
        }
    }
}

impl NamespacedType for Script {
    fn get_name(&self) -> crate::types::GlobalName {
        self.name.clone()
    }
}

impl Script {
    pub fn call_func(
        &self,
        func_name: StringName,
        env: Arc<Environment>,
        inputs: VarRegisters,
        call_stack: &mut Vec<FunctionNode>,
    ) -> Result<VarRegisters, NodeError> {
        let Some(func) = &mut self.funcs.get(&func_name).cloned() else {
            return Err(NodeError::TypeNotFound {
                name: GlobalName::from_path(func_name.to_string()),
                msg: format!("Function not found on script: {:?}", self.name),
            });
        };
        env.logger.debug(format!(
            "calling function {:?} with inputs: \n{:#?}",
            func_name, inputs
        ));
        let mut blackboard = inputs;
        let Some(entry) = func.nodes.get(func.entry) else {
            return Err(NodeError::Unhandled(format!("Failed to find entry node for function {:?}::{:?} at index {} of node array with {} elements", self.name, func_name, func.entry, func.nodes.len())));
        };
        call_stack.push(entry.clone());

        while !call_stack.is_empty() {
            // clean call stack
            Self::purge_duplicate_calls(call_stack, &env);

            // ensure top of stack is present (this should never fail)
            let Some(top) = call_stack.pop() else {
                break;
            };

            // Determine if any inputs need to be loaded first
            if let Some(exec) = self.get_backfill_nodes(func, &top) {
                call_stack.push(top); // reset stack
                for in_node in exec {
                    let Some(in_node_ref) = func.nodes.get(in_node) else {
                        return Err(NodeError::Unhandled(format!("Failed to find entry node for function {:?}::{:?} at index {} of node array with {} elements", self.name, func_name, in_node, func.nodes.len())));
                    };
                    env.logger
                        .debug(format!("Backfilling node: {:?}", in_node_ref));
                    call_stack.push(in_node_ref.clone());
                }
                continue;
            }

            // generate valid input registers
            let inputs = self.get_input_register(func, &top, &blackboard)?;

            // execute the current node
            let results = self.execute_frame(func, &func_name, &env, inputs, &top)?;
            env.logger
                .debug(format!("{:?} Outputs: \n{:#?}", top, results));

            // merge blackboards
            for entry in results.blackboard.0 {
                blackboard.0.insert(entry.0, entry.1);
            }

            env.logger
                .debug(format!("Current blackboard: {:?}", blackboard));

            for index in results.next_nodes {
                let Some(node) = func.nodes.get(func.entry) else {
                    return Err(NodeError::Unhandled(format!("Failed to find next node for function {:?}::{:?} at index {} of node array with {} elements", self.name, func_name, index, func.nodes.len())));
                };
                env.logger.debug(format!("Pushing node: {:?}", node));
                call_stack.push(node.clone());
            }

            // debug frame connection data
            env.logger
                .debug(format!("Frame connection data \n{:#?}", func.routing));
        }

        Ok(blackboard.clone())
    }

    fn purge_duplicate_calls(call_stack: &mut Vec<FunctionNode>, env: &Arc<Environment>) {
        let mut stack_indices = Vec::<usize>::new();
        let mut n_stack = VecDeque::<FunctionNode>::new();
        for entry in call_stack.iter().rev() {
            if stack_indices.contains(&entry.index) {
                env.logger
                    .debug(format!("Dropping duplicate call in stack to {:?}", entry));
                continue;
            }
            stack_indices.push(entry.index);
            n_stack.push_front(entry.clone());
        }
        call_stack.clear();
        for e in n_stack {
            call_stack.push(e);
        }
    }

    fn get_input_register(
        &self,
        func: &Function,
        node: &FunctionNode,
        blackboard: &VarRegisters,
    ) -> Result<VarRegisters, NodeError> {
        let target = node.index;
        let inputs = func
            .routing
            .iter()
            .filter_map(|route| {
                if route.to != target {
                    return None;
                }
                if route.value == Var::Null {
                    return None;
                }
                Some((route.to_param.clone(), route.value.clone()))
            })
            .collect::<Vec<_>>();
        let mut registers = VarRegisters::new();
        for (key, value) in inputs {
            registers.0.insert(key, value);
        }
        for req in node.node.get_inputs() {
            if registers.0.contains_key(&req) {
                continue;
            }
            if let Some(entry) = blackboard.0.get(&req) {
                registers.0.insert(req, entry.clone());
                continue;
            }
            return Err(NodeError::NullException {
                name: node.node.get_name(),
                arg: req.clone(),
                msg: "Failed to find valid input between blackboard and connections".into(),
            });
        }
        Ok(registers)
    }

    /// Determines if any nodes need to be injected into the call stack first to load the registers
    fn get_backfill_nodes(&self, func: &Function, node: &FunctionNode) -> Option<Vec<usize>> {
        let target = node.index;
        let invalid_inputs = func
            .routing
            .iter()
            .filter(|p| p.to == target && p.value == Var::Null)
            .collect::<Vec<_>>();
        if invalid_inputs.is_empty() {
            return None;
        }
        Some(invalid_inputs.iter().map(|route| route.from).collect())
    }

    fn execute_frame(
        &self,
        func: &mut Function,
        _func_name: &StringName,
        env: &Arc<Environment>,
        inputs: VarRegisters,
        node: &FunctionNode,
    ) -> Result<FrameResults, NodeError> {
        env.logger
            .debug(format!("Executing {:?} with inputs: \n{:#?}", node, inputs));
        let next_frame = node.node.execute(env.clone(), inputs)?;

        let mut outputs = func
            .routing
            .iter_mut()
            .filter(|p| p.from == node.index)
            .collect::<Vec<_>>();

        let mut results = FrameResults::default();

        for (key, var) in &next_frame.0 {
            for out in outputs.iter_mut() {
                if let Var::Execution(enabled) = var {
                    if *enabled {
                        results.next_nodes.push(out.to);
                    }
                }
                if out.from_param == key.clone() {
                    out.value = var.clone();
                    continue;
                }
            }
            results.blackboard.0.insert(key.clone(), var.clone());
        }

        Ok(results)
    }
}

impl Debug for FunctionNode {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.write_fmt(format_args!("{:?} #{}", self.node.get_name(), self.index))
    }
}

#[cfg(test)]
mod test {
    use std::{collections::HashMap, sync::Arc};

    use crate::{
        types::{GlobalName, Var, VarRegisters},
        Environment,
    };

    use super::{Connection, Function, Script};

    struct TestScript {
        script: Script,
        env: Environment,
    }

    fn get_test_script() -> TestScript {
        let env = Environment::new();
        let mut script = Script {
            name: GlobalName::from_path("test.script"),
            funcs: HashMap::new(),
        };
        script.funcs.insert(
            "func".into(),
            Function::new(
                &env.nodes,
                vec![
                    GlobalName::from_path("std.math.add"),
                    GlobalName::from_path("std.math.subtract"),
                    GlobalName::from_path("std.math.add"),
                ],
                2,
                vec![
                    Connection::new(0, 1, "c", "b"), // 0:c => 1:b
                    Connection::new(0, 2, "c", "b"), // 0:c => 2:b
                    Connection::new(1, 2, "c", "a"), // 1:c => 2:a
                ],
            ),
        );
        TestScript { script, env }
    }

    #[test]
    /// Test a simple script execution
    ///
    /// Script simulates:
    /// ```
    /// // input
    /// let a = 3.0
    /// let b = 4.0
    /// // logic steps
    /// let x_ = a + b; //  @std.math.add#0 (a=3.0, b=4.0) => 7.0
    /// let y_ = a - x_; // @std.math.subtract#1 (a=3.0, b=7.0) => -4.0
    /// let c = x_ + y_; //  @std.math.add#2 (a=7.0, b=-4.0) => 3.0
    ///
    /// assert_eq!(c, 3.0)
    /// ```
    ///
    /// where `a`, `b`, `c` are known variables
    /// and `x_`, `y_` are internal caches (non variables)
    fn test_script_simple_math() {
        let test_script = get_test_script();

        let result = test_script.script.call_func(
            "func".into(),
            Arc::new(test_script.env),
            VarRegisters(HashMap::from([
                ("a".into(), Var::Num(3.0)),
                ("b".into(), Var::Num(4.0)),
            ])),
            &mut Vec::new(),
        );

        eprintln!("{:#?}", result);
        assert!(result.is_ok());
        let output = result.unwrap();
        assert_eq!(output.0.get(&"c".into()).cloned(), Some(Var::Num(3.0)));
    }

    #[test]
    /// Same general logic as [test_script_simple_math], but repeating the function call 5 times, to validate that these function calls do not leave any lasting effects on the script object itself. Specifically because it is called as an &self instead of an &mut self, this should be impossible, but for surety, this test exists.
    fn test_script_immutable() {
        let test_script = get_test_script();
        let env = Arc::new(test_script.env);
        for i in 0..5 {
            let result = test_script.script.call_func(
                "func".into(),
                env.clone(),
                VarRegisters(HashMap::from([
                    ("a".into(), Var::Num(3.0)),
                    ("b".into(), Var::Num(4.0)),
                ])),
                &mut Vec::new(),
            );
            eprintln!("Calling func iter: {i}");
            assert!(result.is_ok());
            let output = result.unwrap();
            assert_eq!(output.0.get(&"c".into()).cloned(), Some(Var::Num(3.0)));
        }
    }
}
