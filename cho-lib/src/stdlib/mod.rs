#![allow(unused)] // probably bad but I hate the warning and it'll take time to build the stdlib
use std::collections::HashMap;

use crate::{
    nodes::{BasicNode, BasicNodeLogic, Node, NodeError},
    types::{GlobalName, StringName, TypeRegistry, Var, VarRegisters},
};

pub mod console;
pub mod math;
pub mod vars;

pub fn register(registry: &mut TypeRegistry<Node>) {
    console::register(registry);
    math::register(registry);
    vars::register(registry);
}

fn add_basic(
    reg: &mut TypeRegistry<Node>,
    name: GlobalName,
    inputs: impl IntoIterator<Item = (&'static str, Var)>,
    outputs: impl IntoIterator<Item = (&'static str, Var)>,
    logic: BasicNodeLogic,
) {
    let mut input_map = VarRegisters::new();
    let mut output_map = VarRegisters::new();
    for (k, v) in inputs.into_iter() {
        input_map.0.insert(k.into(), v);
    }
    for (k, v) in outputs.into_iter() {
        output_map.0.insert(k.into(), v);
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
    inputs: &VarRegisters,
    field: impl Into<StringName>,
) -> Result<Var, NodeError> {
    let sn: StringName = field.into();
    let var = inputs.0.get(&sn).cloned().unwrap_or_default();
    if var == Var::Null {
        return Err(NodeError::NullException {
            name: name.clone(),
            arg: sn.into(),
            msg: "Field was found null".into(),
        });
    }
    Ok(var)
}

fn get_var_string(
    name: &GlobalName,
    inputs: &VarRegisters,
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
    inputs: &VarRegisters,
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
    inputs: &VarRegisters,
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
