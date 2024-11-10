#![allow(unused)] // probably bad but I hate the warning and it'll take time to build the stdlib
use std::collections::HashMap;

use crate::types::{
    BasicNode, BasicNodeLogic, GlobalName, Node, NodeError, StringName, TypeRegistry, Var,
};

pub mod console;
pub mod math;

pub fn register(registry: &mut TypeRegistry) {
    console::register(registry);
    math::register(registry);
}

fn add_basic(
    reg: &mut TypeRegistry,
    name: GlobalName,
    inputs: impl IntoIterator<Item = (&'static str, Var)>,
    outputs: impl IntoIterator<Item = (&'static str, Var)>,
    logic: BasicNodeLogic,
) {
    let mut input_map = HashMap::new();
    let mut output_map = HashMap::new();
    for (k, v) in inputs.into_iter() {
        input_map.insert(k.into(), v);
    }
    for (k, v) in outputs.into_iter() {
        output_map.insert(k.into(), v);
    }
    reg.register(Node::Basic(BasicNode {
        name,
        logic,
        inputs: input_map,
        outputs: output_map,
    }));
}

fn get_var(
    name: &GlobalName,
    inputs: &HashMap<StringName, Var>,
    field: impl Into<StringName>,
) -> Result<Var, NodeError> {
    let sn: StringName = field.into();
    let var = inputs.get(&sn).cloned().unwrap_or_default();
    if var == Var::Null {
        return Err(NodeError::NullExecption {
            name: name.clone(),
            arg: sn.into(),
            msg: "Field was found null".into(),
        });
    }
    Ok(var)
}

fn get_var_string(
    name: &GlobalName,
    inputs: &HashMap<StringName, Var>,
    field: StringName,
) -> Result<String, NodeError> {
    let var = get_var(name, inputs, field.clone())?;
    let Var::String(value) = var else {
        return Err(NodeError::MismatchedData {
            name: name.clone(),
            arg: field,
            expected: Var::String(Default::default()),
            received: var,
            msg: "".into(),
        });
    };
    Ok(value)
}

fn get_var_bool(
    name: &GlobalName,
    inputs: &HashMap<StringName, Var>,
    field: StringName,
) -> Result<bool, NodeError> {
    let var = get_var(name, inputs, field.clone())?;
    let Var::Bool(value) = var else {
        return Err(NodeError::MismatchedData {
            name: name.clone(),
            arg: field,
            expected: Var::Bool(Default::default()),
            received: var,
            msg: "".into(),
        });
    };
    Ok(value)
}

fn get_var_number(
    name: &GlobalName,
    inputs: &HashMap<StringName, Var>,
    field: StringName,
) -> Result<f64, NodeError> {
    let var = get_var(name, inputs, field.clone())?;
    let Var::Num(value) = var else {
        return Err(NodeError::MismatchedData {
            name: name.clone(),
            arg: field,
            expected: Var::Num(Default::default()),
            received: var,
            msg: "".into(),
        });
    };
    Ok(value)
}
