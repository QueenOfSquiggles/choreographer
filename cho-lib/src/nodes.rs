use std::sync::Arc;

use crate::{
    scripts::Script,
    types::{GlobalName, NamespacedType, StringName, Var, VarRegisters},
    Environment,
};

#[derive(Debug, Clone, PartialEq)]
pub enum Node {
    Basic(BasicNode),
    Script(ScriptNode),
}

#[derive(Debug, Clone)]
pub enum NodeError {
    // TODO: add more specific errors
    Unhandled(String),
    TypeNotFound {
        name: GlobalName,
        msg: String,
    },
    NullException {
        name: GlobalName,
        arg: StringName,
        msg: String,
    },
    MismatchedData {
        name: GlobalName,
        arg: StringName,
        expected: Var,
        received: Var,
        msg: String,
    },
}

#[derive(Clone)]
pub struct BasicNodeLogic(
    pub Arc<dyn Fn(Arc<Environment>, VarRegisters) -> Result<VarRegisters, NodeError>>,
);

#[derive(Clone)]
pub struct BasicNode {
    pub name: GlobalName,
    pub inputs: VarRegisters,
    pub outputs: VarRegisters,
    pub logic: BasicNodeLogic,
}

#[derive(Clone)]
pub struct ScriptNode {
    pub name: GlobalName,
    pub func: StringName,
    pub script: Arc<Script>,
}

pub trait NodeData {
    fn execute(
        &self,
        env: Arc<Environment>,
        inputs: VarRegisters,
    ) -> Result<VarRegisters, NodeError>;
    fn get_inputs(&self) -> Vec<StringName>;
    fn get_outputs(&self) -> Vec<StringName>;
}

impl PartialEq for BasicNode {
    fn eq(&self, other: &Self) -> bool {
        self.name == other.name
    }
}

impl NamespacedType for BasicNode {
    fn get_name(&self) -> GlobalName {
        self.name.clone()
    }
}

impl NodeData for BasicNode {
    fn execute(
        &self,
        env: Arc<Environment>,
        inputs: VarRegisters,
    ) -> Result<VarRegisters, NodeError> {
        self.logic.0(env, inputs)
    }

    fn get_inputs(&self) -> Vec<StringName> {
        self.inputs.0.keys().cloned().collect()
    }

    fn get_outputs(&self) -> Vec<StringName> {
        self.outputs.0.keys().cloned().collect()
    }
}

impl NamespacedType for ScriptNode {
    fn get_name(&self) -> GlobalName {
        self.name.clone()
    }
}

impl NodeData for ScriptNode {
    fn execute(
        &self,
        env: Arc<Environment>,
        inputs: VarRegisters,
    ) -> Result<VarRegisters, NodeError> {
        env.logger
            .debug(format!("Executing script node: {:?}", self.name));

        self.script
            .call_func(self.func.clone(), env, inputs, &mut Vec::new())
    }

    fn get_inputs(&self) -> Vec<StringName> {
        Vec::new()
    }

    fn get_outputs(&self) -> Vec<StringName> {
        Vec::new()
    }
}

impl NamespacedType for Node {
    fn get_name(&self) -> GlobalName {
        match self {
            Node::Basic(basic_node) => basic_node.get_name(),
            Node::Script(script_node) => script_node.get_name(),
        }
    }
}

impl NodeData for Node {
    fn execute(
        &self,
        env: Arc<Environment>,
        inputs: VarRegisters,
    ) -> Result<VarRegisters, NodeError> {
        match self {
            Node::Basic(basic_node) => basic_node.execute(env, inputs),
            Node::Script(script_node) => script_node.execute(env, inputs),
        }
    }

    fn get_inputs(&self) -> Vec<StringName> {
        match self {
            Node::Basic(basic_node) => basic_node.get_inputs(),
            Node::Script(script_node) => script_node.get_inputs(),
        }
    }

    fn get_outputs(&self) -> Vec<StringName> {
        match self {
            Node::Basic(basic_node) => basic_node.get_outputs(),
            Node::Script(script_node) => script_node.get_outputs(),
        }
    }
}

impl std::fmt::Debug for BasicNode {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("BasicNode")
            .field("name", &self.name)
            .field("inputs", &self.inputs)
            .field("outputs", &self.outputs)
            .finish()
    }
}

impl std::fmt::Debug for ScriptNode {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("ScriptNode")
            .field("name", &self.name)
            .finish()
    }
}

impl PartialEq for ScriptNode {
    fn eq(&self, other: &Self) -> bool {
        self.name == other.name
    }
}

impl BasicNodeLogic {
    pub fn new(
        func_ref: impl Fn(Arc<Environment>, VarRegisters) -> Result<VarRegisters, NodeError> + 'static,
    ) -> Self {
        Self(Arc::new(func_ref))
    }
}
