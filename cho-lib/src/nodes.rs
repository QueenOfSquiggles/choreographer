use std::{collections::HashMap, sync::Arc};

use crate::{
    types::{GlobalName, NamespacedType, StringName, Var},
    Environment,
};

#[derive(Debug, Clone)]
pub enum Node {
    Basic(BasicNode),
    Composite(CompositeNode),
}

#[derive(Debug, Clone)]
pub enum NodeError {
    // TODO: add more specific errors
    Unhandled(String),
    TypeNotFound {
        name: GlobalName,
        msg: String,
    },
    NullExecption {
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
    pub  Arc<
        dyn Fn(
            Arc<Environment>,
            HashMap<StringName, Var>,
        ) -> Result<HashMap<StringName, Var>, NodeError>,
    >,
);

#[derive(Clone)]
pub struct BasicNode {
    pub name: GlobalName,
    pub inputs: HashMap<String, Var>,
    pub outputs: HashMap<String, Var>,
    pub logic: BasicNodeLogic,
}

pub trait NodeData {
    fn execute(
        &self,
        env: Arc<Environment>,
        inputs: HashMap<StringName, Var>,
    ) -> Result<HashMap<StringName, Var>, NodeError>;
}

#[derive(Debug, Clone)]
pub struct CompositeNode {
    pub name: GlobalName,
    pub nodes: Vec<GlobalName>,
    pub entry: GlobalName,
    pub outputs: HashMap<GlobalName, HashMap<StringName, (GlobalName, StringName)>>,
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
        inputs: HashMap<StringName, Var>,
    ) -> Result<HashMap<StringName, Var>, NodeError> {
        self.logic.0(env, inputs)
    }
}

impl NamespacedType for CompositeNode {
    fn get_name(&self) -> GlobalName {
        self.name.clone()
    }
}

impl NodeData for CompositeNode {
    fn execute(
        &self,
        env: Arc<Environment>,
        inputs: HashMap<StringName, Var>,
    ) -> Result<HashMap<StringName, Var>, NodeError> {
        let Some(entry) = env.nodes.get(&self.entry) else {
            return Err(NodeError::TypeNotFound {
                name: self.entry.clone(),
                msg: "Failed to find composite node's entry node".into(),
            });
        };
        entry.execute(env, inputs)
    }
}

impl NamespacedType for Node {
    fn get_name(&self) -> GlobalName {
        match self {
            Node::Basic(basic_node) => basic_node.get_name(),
            Node::Composite(composite_node) => composite_node.get_name(),
        }
    }
}

impl NodeData for Node {
    fn execute(
        &self,
        env: Arc<Environment>,
        inputs: HashMap<StringName, Var>,
    ) -> Result<HashMap<StringName, Var>, NodeError> {
        match self {
            Node::Basic(basic_node) => basic_node.execute(env, inputs),
            Node::Composite(composite_node) => composite_node.execute(env, inputs),
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

impl BasicNodeLogic {
    pub fn new(
        func_ref: impl Fn(
                Arc<Environment>,
                HashMap<StringName, Var>,
            ) -> Result<HashMap<StringName, Var>, NodeError>
            + 'static,
    ) -> Self {
        Self(Arc::new(func_ref))
    }
}
