use std::{collections::HashMap, sync::Arc};

use crate::{
    nodes::{BasicNodeLogic, Node, NodeError},
    types::{GlobalName, TypeRegistry, Var, VarRegisters},
    Environment,
};

use super::{add_basic, get_var_bool, get_var_number};

const STD_IF: &'static str = "std.control.if";

pub fn register(registry: &mut TypeRegistry<Node>) {
    add_basic(
        registry,
        GlobalName::from_path(STD_IF),
        vec![("flag", Var::Bool(false))],
        vec![
            ("if", Var::Execution(false)),
            ("else", Var::Execution(false)),
        ],
        BasicNodeLogic::new(node_std_if_else),
    );
}

fn node_std_if_else(
    _env: Arc<Environment>,
    inputs: VarRegisters,
) -> Result<VarRegisters, NodeError> {
    let name = GlobalName::from_path(STD_IF);
    let flag = get_var_bool(&name, &inputs, "flag".into())?;
    Ok(VarRegisters(HashMap::from([
        ("if".into(), Var::Execution(flag)),
        ("else".into(), Var::Execution(!flag)),
    ])))
}

#[cfg(test)]
mod test {
    use std::{collections::HashMap, sync::Arc};

    use crate::{
        nodes::NodeError,
        types::{GlobalName, Var, VarRegisters},
        Environment,
    };

    use super::node_std_if_else;

    fn get_env() -> Arc<Environment> {
        Arc::new(Environment::new_empty())
    }

    #[test]
    fn test_if_else() {
        let env = get_env();

        // input true
        let res = node_std_if_else(
            env.clone(),
            VarRegisters(HashMap::from([("flag".into(), Var::Bool(true))])),
        );
        assert!(res.is_ok());
        let reg = res.unwrap();
        assert_eq!(reg.0.get(&"if".into()).cloned(), Some(Var::Execution(true)));
        assert_eq!(
            reg.0.get(&"else".into()).cloned(),
            Some(Var::Execution(false))
        );

        // input false
        let res = node_std_if_else(
            env.clone(),
            VarRegisters(HashMap::from([("flag".into(), Var::Bool(false))])),
        );
        assert!(res.is_ok());
        let reg = res.unwrap();
        assert_eq!(
            reg.0.get(&"if".into()).cloned(),
            Some(Var::Execution(false))
        );
        assert_eq!(
            reg.0.get(&"else".into()).cloned(),
            Some(Var::Execution(true))
        );
    }
}
