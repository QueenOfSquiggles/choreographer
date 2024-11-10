use std::{collections::HashMap, fmt::Debug, sync::Arc};

use serde::{Deserialize, Serialize};

use crate::Environment;

#[derive(PartialEq, Eq, Hash, Clone)]
pub struct StringName(Arc<Vec<u8>>);

#[derive(PartialEq, Eq, Hash, Clone, Debug)]
pub struct Namespace(pub StringName);

#[derive(PartialEq, Eq, Hash, Clone, Debug)]
pub struct TypeName(pub StringName);

#[derive(Clone, PartialEq, Default, Debug, Serialize, Deserialize)]
pub enum Var {
    #[default]
    Null,
    Num(f64),
    Bool(bool),
    String(String),
}

#[derive(PartialEq, Eq, Hash, Clone)]

pub struct GlobalName(pub Namespace, pub TypeName);

#[derive(Debug)]
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

pub struct BasicNodeLogic(
    pub  Arc<
        dyn Fn(
            Arc<Environment>,
            HashMap<StringName, Var>,
        ) -> Result<HashMap<StringName, Var>, NodeError>,
    >,
);

pub struct BasicNode {
    pub name: GlobalName,
    pub inputs: HashMap<String, Var>,
    pub outputs: HashMap<String, Var>,
    pub logic: BasicNodeLogic,
}

pub trait NodeData {
    fn get_name(&self) -> GlobalName;
    fn execute(
        &self,
        env: Arc<Environment>,
        inputs: HashMap<StringName, Var>,
    ) -> Result<HashMap<StringName, Var>, NodeError>;
}

#[derive(Debug)]
pub struct CompositeNode {
    pub name: GlobalName,
    pub nodes: Vec<GlobalName>,
    pub entry: GlobalName,
    pub outputs: HashMap<GlobalName, HashMap<StringName, (GlobalName, StringName)>>,
}

#[derive(Default, Debug, Clone)]
pub struct TypeRegistry {
    types: HashMap<Namespace, HashMap<TypeName, Arc<Node>>>,
}

impl TypeRegistry {
    pub fn register(&mut self, node: Node) {
        let GlobalName(n, t) = match &node {
            Node::Basic(basic_node) => basic_node.name.clone(),
            Node::Composite(composite_node) => composite_node.name.clone(),
        };
        if let Some(namespace_reg) = self.types.get_mut(&n) {
            if namespace_reg.contains_key(&t) {
                panic!("Cannot create duplicate type of {:?}", GlobalName(n, t));
            }
            namespace_reg.insert(t, Arc::new(node));
        } else {
            let mut map = HashMap::new();
            map.insert(t, Arc::new(node));
            self.types.insert(n, map);
        }
    }

    pub fn get(&self, name: &GlobalName) -> Option<Arc<Node>> {
        self.types
            .get(&name.0)
            .cloned()
            .and_then(|map| map.get(&name.1).cloned())
    }
}

impl NodeData for BasicNode {
    fn get_name(&self) -> GlobalName {
        self.name.clone()
    }

    fn execute(
        &self,
        env: Arc<Environment>,
        inputs: HashMap<StringName, Var>,
    ) -> Result<HashMap<StringName, Var>, NodeError> {
        self.logic.0(env, inputs)
    }
}

impl NodeData for CompositeNode {
    fn get_name(&self) -> GlobalName {
        self.name.clone()
    }

    fn execute(
        &self,
        env: Arc<Environment>,
        inputs: HashMap<StringName, Var>,
    ) -> Result<HashMap<StringName, Var>, NodeError> {
        let Some(entry) = env.registry.get(&self.entry) else {
            return Err(NodeError::TypeNotFound {
                name: self.entry.clone(),
                msg: "Failed to find composite node's entry node".into(),
            });
        };
        entry.execute(env, inputs)
    }
}

impl NodeData for Node {
    fn get_name(&self) -> GlobalName {
        match self {
            Node::Basic(basic_node) => basic_node.get_name(),
            Node::Composite(composite_node) => composite_node.get_name(),
        }
    }

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

impl From<&'static str> for StringName {
    fn from(value: &'static str) -> Self {
        StringName(Arc::new(value.as_bytes().into()))
    }
}

impl From<String> for StringName {
    fn from(value: String) -> Self {
        StringName(Arc::new(value.into_bytes()))
    }
}

impl<T: Into<StringName>> From<T> for Namespace {
    fn from(value: T) -> Self {
        Namespace(value.into())
    }
}
impl<T: Into<StringName>> From<T> for TypeName {
    fn from(value: T) -> Self {
        TypeName(value.into())
    }
}

impl ToString for StringName {
    fn to_string(&self) -> String {
        String::from_utf8(self.0.iter().cloned().collect()).unwrap_or("".into())
    }
}

impl GlobalName {
    pub const PATH_DELIM: &'static str = ".";

    pub fn from_path(path: impl Into<String>) -> Self {
        let string: String = path.into();
        let reverse = string.chars().rev().collect::<String>();
        let Some((tr, nr)) = reverse.split_once(Self::PATH_DELIM) else {
            return GlobalName("".into(), string.into());
        };
        let type_name = tr.chars().rev().collect::<String>();
        let namespace = nr.chars().rev().collect::<String>();
        GlobalName(namespace.into(), type_name.into())
    }

    pub fn to_path(&self) -> String {
        let ns = self.0 .0.to_string();
        let tn = self.1 .0.to_string();
        format!("{ns}.{tn}")
    }

    pub fn is_empty(&self) -> bool {
        self.0 .0 .0.is_empty() && self.1 .0 .0.is_empty()
    }
}
impl std::fmt::Debug for GlobalName {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        if self.0 .0 .0.is_empty() {
            f.write_fmt(format_args!("@{:}", self.1 .0.to_string()))
        } else {
            f.write_fmt(format_args!(
                "@{:}.{:}",
                self.0 .0.to_string(),
                self.1 .0.to_string()
            ))
        }
    }
}

impl std::fmt::Debug for StringName {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let human_readable =
            String::from_utf8(self.0.iter().cloned().collect::<Vec<u8>>()).unwrap_or("".into());
        f.write_fmt(format_args!("StringName(\"{}\")", human_readable))
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
