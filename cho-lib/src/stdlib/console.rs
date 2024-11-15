use std::{collections::HashMap, sync::Arc};

use crate::{
    nodes::{BasicNodeLogic, Node, NodeError},
    types::{GlobalName, StringName, TypeRegistry, Var, VarRegisters},
    Environment,
};

use super::{add_basic, get_var_string};

pub fn register(registry: &mut TypeRegistry<Node>) {
    add_basic(
        registry,
        GlobalName::from_path("std.print"),
        vec![("exec", Var::Execution(false))],
        vec![],
        BasicNodeLogic::new(node_std_print),
    );
}

fn node_std_print(env: Arc<Environment>, inputs: VarRegisters) -> Result<VarRegisters, NodeError> {
    let text = get_var_string(&GlobalName::from_path("std.print"), &inputs, "text".into())?;
    env.logger.info(format!("{text}"));
    Ok(VarRegisters::default())
}
