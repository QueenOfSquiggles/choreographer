use std::{collections::HashMap, sync::Arc};

use crate::{
    types::{BasicNodeLogic, GlobalName, NodeError, StringName, TypeRegistry, Var},
    Environment,
};

use super::{add_basic, get_var_number};

pub fn register(registry: &mut TypeRegistry) {
    add_basic(
        registry,
        GlobalName::from_path("std.math.add"),
        vec![("a", Var::Num(0.0)), ("b", Var::Num(0.0))],
        vec![("c", Var::Num(0.0))],
        BasicNodeLogic::new(node_std_add),
    );
    add_basic(
        registry,
        GlobalName::from_path("std.math.subtract"),
        vec![("a", Var::Num(0.0)), ("b", Var::Num(0.0))],
        vec![("c", Var::Num(0.0))],
        BasicNodeLogic::new(node_std_subtract),
    );
    add_basic(
        registry,
        GlobalName::from_path("std.math.multiply"),
        vec![("a", Var::Num(0.0)), ("b", Var::Num(0.0))],
        vec![("c", Var::Num(0.0))],
        BasicNodeLogic::new(node_std_multiply),
    );
    add_basic(
        registry,
        GlobalName::from_path("std.math.divide"),
        vec![("a", Var::Num(0.0)), ("b", Var::Num(0.0))],
        vec![("c", Var::Num(0.0))],
        BasicNodeLogic::new(node_std_divide),
    );
}

fn node_std_add(
    _env: Arc<Environment>,
    inputs: HashMap<StringName, Var>,
) -> Result<HashMap<StringName, Var>, NodeError> {
    let name = GlobalName::from_path("std.math.add");
    let a = get_var_number(&name, &inputs, "a".into())?;
    let b = get_var_number(&name, &inputs, "b".into())?;
    let mut out = HashMap::new();
    out.insert("c".into(), Var::Num(a + b));
    Ok(out)
}

fn node_std_subtract(
    _env: Arc<Environment>,
    inputs: HashMap<StringName, Var>,
) -> Result<HashMap<StringName, Var>, NodeError> {
    let name = GlobalName::from_path("std.math.add");
    let a = get_var_number(&name, &inputs, "a".into())?;
    let b = get_var_number(&name, &inputs, "b".into())?;
    let mut out = HashMap::new();
    out.insert("c".into(), Var::Num(a - b));
    Ok(out)
}

fn node_std_multiply(
    _env: Arc<Environment>,
    inputs: HashMap<StringName, Var>,
) -> Result<HashMap<StringName, Var>, NodeError> {
    let name = GlobalName::from_path("std.math.add");
    let a = get_var_number(&name, &inputs, "a".into())?;
    let b = get_var_number(&name, &inputs, "b".into())?;
    let mut out = HashMap::new();
    out.insert("c".into(), Var::Num(a * b));
    Ok(out)
}

fn node_std_divide(
    _env: Arc<Environment>,
    inputs: HashMap<StringName, Var>,
) -> Result<HashMap<StringName, Var>, NodeError> {
    let name = GlobalName::from_path("std.math.add");
    let a = get_var_number(&name, &inputs, "a".into())?;
    let b = get_var_number(&name, &inputs, "b".into())?;
    let mut out = HashMap::new();
    if b == 0.0 {
        return Err(NodeError::Unhandled(
            "Math error. Cannot divide by zero".into(),
        ));
    }
    out.insert("c".into(), Var::Num(a / b));
    Ok(out)
}
