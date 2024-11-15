use std::{collections::HashMap, sync::Arc};

use crate::{
    nodes::{BasicNodeLogic, Node, NodeError},
    types::{GlobalName, StringName, TypeRegistry, Var, VarRegisters},
    Environment,
};

use super::{add_basic, get_var_number};

pub fn register(registry: &mut TypeRegistry<Node>) {
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

fn node_std_add(_env: Arc<Environment>, inputs: VarRegisters) -> Result<VarRegisters, NodeError> {
    let name = GlobalName::from_path("std.math.add");
    let a = get_var_number(&name, &inputs, "a".into())?;
    let b = get_var_number(&name, &inputs, "b".into())?;
    let mut out = VarRegisters::new();
    out.0.insert("c".into(), Var::Num(a + b));
    Ok(out)
}

fn node_std_subtract(
    _env: Arc<Environment>,
    inputs: VarRegisters,
) -> Result<VarRegisters, NodeError> {
    let name = GlobalName::from_path("std.math.subtract");
    let a = get_var_number(&name, &inputs, "a".into())?;
    let b = get_var_number(&name, &inputs, "b".into())?;
    let mut out = VarRegisters::new();
    out.0.insert("c".into(), Var::Num(a - b));
    Ok(out)
}

fn node_std_multiply(
    _env: Arc<Environment>,
    inputs: VarRegisters,
) -> Result<VarRegisters, NodeError> {
    let name = GlobalName::from_path("std.math.multiply");
    let a = get_var_number(&name, &inputs, "a".into())?;
    let b = get_var_number(&name, &inputs, "b".into())?;
    let mut out = VarRegisters::new();
    out.0.insert("c".into(), Var::Num(a * b));
    Ok(out)
}

fn node_std_divide(
    _env: Arc<Environment>,
    inputs: VarRegisters,
) -> Result<VarRegisters, NodeError> {
    let name = GlobalName::from_path("std.math.divide");
    let a = get_var_number(&name, &inputs, "a".into())?;
    let b = get_var_number(&name, &inputs, "b".into())?;
    let mut out = VarRegisters::new();
    if b == 0.0 {
        return Err(NodeError::Unhandled(
            "Math error. Cannot divide by zero".into(),
        ));
    }
    out.0.insert("c".into(), Var::Num(a / b));
    Ok(out)
}

#[cfg(test)]
mod test {
    use core::panic;
    use std::{collections::HashMap, sync::Arc};

    use crate::{
        nodes::NodeError,
        types::{GlobalName, Var, VarRegisters},
        Environment,
    };

    use super::{node_std_add, node_std_divide, node_std_multiply, node_std_subtract};
    fn get_env() -> Arc<Environment> {
        Arc::new(Environment::new_empty())
    }
    fn get_inputs(a: f64, b: f64) -> VarRegisters {
        VarRegisters(HashMap::from([
            ("a".into(), Var::Num(a)),
            ("b".into(), Var::Num(b)),
        ]))
    }

    fn get_output(result: Result<VarRegisters, NodeError>) -> Result<f64, NodeError> {
        if let Err(er) = result {
            return Err(er);
        }
        let reg = result.unwrap();
        let Some(var) = reg.0.get(&"c".into()) else {
            return Err(NodeError::NullException {
                name: GlobalName::from_path("test"),
                arg: "c".into(),
                msg: "Failed to find output".into(),
            });
        };
        if let Var::Num(x) = var {
            return Ok(*x);
        }
        Err(NodeError::MismatchedData {
            name: GlobalName::from_path("test"),
            arg: "c".into(),
            expected: Var::Num(0.0),
            received: var.clone(),
            msg: "Mismatched output data".into(),
        })
    }

    fn test_math(
        a: f64,
        b: f64,
        c: f64,
        func: impl Fn(Arc<Environment>, VarRegisters) -> Result<VarRegisters, NodeError>,
    ) {
        eprintln!("Testing {a} (op) {b} = {c}");

        let res = func(get_env(), get_inputs(a, b));
        match get_output(res) {
            Ok(val) => assert_eq!(val, c),
            Err(err) => panic!("{:#?}", err),
        }
    }

    #[test]
    fn test_add() {
        test_math(1.0, 1.0, 2.0, node_std_add);
        test_math(0.0, 0.0, 0.0, node_std_add);
        test_math(5.0, -3.0, 2.0, node_std_add);
        test_math(1000.0, 2536.0, (1000.0 + 2536.0), node_std_add);
    }
    #[test]
    fn test_sub() {
        test_math(1.0, 1.0, 0.0, node_std_subtract);
        test_math(0.0, 5.0, -5.0, node_std_subtract);
        test_math(5.0, -3.0, 8.0, node_std_subtract);
        test_math(1000.0, 2536.0, (1000.0 - 2536.0), node_std_subtract);
    }

    #[test]
    fn test_mul() {
        test_math(1.0, 1.0, 1.0, node_std_multiply);
        test_math(0.0, 5.0, 0.0, node_std_multiply);
        test_math(5.0, -3.0, -15.0, node_std_multiply);
        test_math(1000.0, 2536.0, (1000.0 * 2536.0), node_std_multiply);
    }

    #[test]
    fn test_div() {
        test_math(1.0, 1.0, 1.0, node_std_divide);
        test_math(0.0, 3.0, 0.0, node_std_divide);
        test_math(5.0, -3.0, 5.0 / -3.0, node_std_divide);
        test_math(1000.0, 2536.0, (1000.0 / 2536.0), node_std_divide);
    }

    #[test]
    #[should_panic]
    fn test_div_by_zero() {
        test_math(1.0, 0.0, 0.0, node_std_divide);
    }
}
