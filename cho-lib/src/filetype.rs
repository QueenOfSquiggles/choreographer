use std::{cmp::Ordering, collections::HashMap};

use serde::{Deserialize, Serialize};

use crate::{
    nodes::Node,
    scripts::{Connection, Function, Script},
    types::{GlobalName, NamespacedType, TypeRegistry},
};

#[derive(Debug, Clone, Deserialize, Serialize)]
pub struct ScriptProto {
    pub global_name: String,
    pub funcs: Vec<(String, FunctionProto)>,
}

#[derive(Debug, Clone, Deserialize, Serialize)]
pub struct FunctionProto {
    pub entry: usize,
    pub nodes: Vec<String>,
    pub connections: Vec<ConnectionProto>,
}

#[derive(Debug, Clone, Deserialize, Serialize)]
pub struct ConnectionProto {
    pub from: usize,
    pub to: usize,
    pub to_param: String,
    pub from_param: String,
}

impl ScriptProto {
    /// Generates an in-memory script from a given prototype, allowing for execution and registration of the parsed information
    pub fn to_script(self, registry: &TypeRegistry<Node>) -> Script {
        let mut script = Script {
            name: GlobalName::from_path(self.global_name),
            funcs: HashMap::new(),
        };
        for (name, proto) in self.funcs {
            script
                .funcs
                .insert(name.into(), proto.to_function(registry));
        }
        script
    }

    /// Generates a prototype based on the in-memory script. Useful for making an editor
    pub fn from_script(script: &Script) -> Self {
        let mut proto = Self {
            global_name: script.name.clone().to_path(),
            funcs: Vec::new(),
        };
        for (name, func) in script.funcs.iter() {
            proto.funcs.push((
                name.clone().to_string(),
                FunctionProto {
                    entry: func.entry,
                    nodes: func
                        .nodes
                        .iter()
                        .map(|f| f.node.get_name().to_path())
                        .collect(),
                    connections: func
                        .routing
                        .iter()
                        .map(|c| ConnectionProto {
                            from: c.from,
                            to: c.to,
                            to_param: c.to_param.to_string(),
                            from_param: c.from_param.to_string(),
                        })
                        .collect(),
                },
            ));
        }
        proto
    }

    /// Stabilizes data entries for better support from VCS
    pub fn stabilize(mut self) -> Self {
        self.funcs.sort_by(|a, b| a.0.cmp(&b.0));
        for (_, func) in self.funcs.iter_mut() {
            func.nodes.sort_by(|a, b| a.cmp(b));

            func.connections.sort_by(|a, b| match a.from.cmp(&b.from) {
                Ordering::Less => Ordering::Less,
                Ordering::Greater => Ordering::Greater,
                Ordering::Equal => match a.to.cmp(&b.to) {
                    Ordering::Less => Ordering::Less,
                    Ordering::Greater => Ordering::Greater,
                    Ordering::Equal => a.from_param.cmp(&b.from_param),
                },
            });
        }
        self
    }
}

impl FunctionProto {
    pub fn to_function(self, registry: &TypeRegistry<Node>) -> Function {
        Function::new(
            registry,
            self.nodes
                .into_iter()
                .map(|f| GlobalName::from_path(f))
                .collect::<Vec<GlobalName>>(),
            self.entry,
            self.connections
                .into_iter()
                .map(|c| Connection::new(c.from, c.to, c.from_param, c.to_param))
                .collect(),
        )
    }
}
#[cfg(test)]
mod test {

    use crate::{types::GlobalName, Environment};

    use super::ScriptProto;

    const SCRIPT_TEXT: &'static str = r#"
(
    global_name: "test.script",
    funcs: [
        (
            "func", FunctionProto(
                entry: 2,
                nodes: [
                    "std.math.add",
                    "std.math.subtract",
                    "std.math.add",
                ],
                connections: [
                    ConnectionProto(
                        from: 0,
                        to: 1,
                        to_param: "c",
                        from_param: "b",                
                    ),
                    ConnectionProto(
                        from: 0,
                        to: 2,
                        to_param: "c",
                        from_param: "b",                
                    ),
                    ConnectionProto(
                        from: 1,
                        to: 2,
                        to_param: "c",
                        from_param: "a",                
                    ),
                ],     
            )
        )
    ],
)"#;

    #[test]
    fn test_deser() {
        let read_res = ron::de::from_str::<ScriptProto>(&SCRIPT_TEXT);
        let proto = read_res.unwrap();
        eprintln!("Prototype: {:#?}", proto);
        let env = Environment::new();

        let script = proto.to_script(&env.nodes);
        assert_eq!(script.name, GlobalName::from_path("test.script"));
        assert_eq!(script.funcs.len(), 1);
        assert!(script.funcs.contains_key(&"func".into()));
        let func = script.funcs.get(&"func".into()).unwrap();
        assert_eq!(func.entry, 2);
        assert_eq!(func.nodes.len(), 3);
        assert_eq!(func.routing.len(), 3);
    }
}
