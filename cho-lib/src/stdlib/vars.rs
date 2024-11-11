use std::{collections::HashMap, sync::Arc};

use serde::Deserialize;

use crate::{
    nodes::{BasicNodeLogic, Node, NodeError},
    types::{GlobalName, StringName, TypeRegistry, Var},
    Environment,
};

use super::{add_basic, get_var_number, get_var_string};

pub fn register(registry: &mut TypeRegistry<Node>) {
    add_basic(
        registry,
        GlobalName::from_path("std.vars.value"),
        vec![("value", Var::String("".into()))],
        vec![("var", Var::Null)],
        BasicNodeLogic::new(node_var),
    );
}

fn node_var(
    _env: Arc<Environment>,
    inputs: HashMap<StringName, Var>,
) -> Result<HashMap<StringName, Var>, NodeError> {
    let name = GlobalName::from_path("std.vars.value");
    let type_value = get_var_string(&name, &inputs, "value".into())?;
    let mut out = HashMap::new();
    out.insert("var".into(), ron::from_str(&type_value).unwrap_or_default());
    Ok(out)
}
