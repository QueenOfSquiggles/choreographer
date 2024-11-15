use std::{collections::HashMap, fmt::Debug, sync::Arc};

use serde::{Deserialize, Serialize};

#[derive(PartialEq, Eq, Hash, Clone)]
pub struct StringName(Arc<Vec<u8>>);

#[derive(PartialEq, Eq, Hash, Clone, Debug)]
pub struct Namespace(pub StringName);

#[derive(PartialEq, Eq, Hash, Clone, Debug)]
pub struct TypeName(pub StringName);

#[derive(PartialEq, Clone, Debug, Default)]
pub struct VarRegisters(pub HashMap<StringName, Var>);

#[derive(Clone, PartialEq, Default, Debug, Serialize, Deserialize)]
pub enum Var {
    #[default]
    Null,
    Num(f64),
    Bool(bool),
    String(String),
    Execution(bool),
}

#[derive(PartialEq, Eq, Hash, Clone)]

pub struct GlobalName(pub Namespace, pub TypeName);

pub trait NamespacedType {
    fn get_name(&self) -> GlobalName;
}

#[derive(Debug, Clone)]
pub struct TypeRegistry<T: NamespacedType + Clone> {
    types: HashMap<Namespace, HashMap<TypeName, Arc<T>>>,
}

impl VarRegisters {
    pub fn new() -> Self {
        Self::default()
    }
}

impl<T: NamespacedType + Clone> std::default::Default for TypeRegistry<T> {
    fn default() -> Self {
        Self {
            types: Default::default(),
        }
    }
}

impl<T: NamespacedType + Clone> TypeRegistry<T> {
    pub fn register(&mut self, value: T) {
        let GlobalName(n, t) = value.get_name();
        if let Some(namespace_reg) = self.types.get_mut(&n) {
            if namespace_reg.contains_key(&t) {
                panic!("Cannot create duplicate type of {:?}", GlobalName(n, t));
            }
            namespace_reg.insert(t, Arc::new(value));
        } else {
            let mut map = HashMap::new();
            map.insert(t, Arc::new(value));
            self.types.insert(n, map);
        }
    }

    pub fn get(&self, name: &GlobalName) -> Option<Arc<T>> {
        self.types
            .get(&name.0)
            .cloned()
            .and_then(|map| map.get(&name.1).cloned())
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
