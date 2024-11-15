use std::{collections::HashMap, sync::Arc};

use serde::Deserialize;

use crate::{
    nodes::{BasicNodeLogic, Node, NodeError},
    types::{GlobalName, StringName, TypeRegistry, Var, VarRegisters},
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

fn node_var(_env: Arc<Environment>, inputs: VarRegisters) -> Result<VarRegisters, NodeError> {
    let name = GlobalName::from_path("std.vars.value");
    let type_value = get_var_string(&name, &inputs, "value".into())?;
    let var_name = get_var_string(&name, &inputs, "name".into()).unwrap_or("var".into());
    let mut out = VarRegisters::new();
    out.0.insert(
        var_name.into(),
        ron::from_str(&type_value).unwrap_or_default(),
    );
    Ok(out)
}
