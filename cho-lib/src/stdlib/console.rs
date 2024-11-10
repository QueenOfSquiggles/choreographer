use std::{collections::HashMap, sync::Arc};

use crate::{
    types::{BasicNodeLogic, GlobalName, NodeError, StringName, TypeRegistry, Var},
    Environment,
};

use super::{add_basic, get_var_string};

pub fn register(registry: &mut TypeRegistry) {
    add_basic(
        registry,
        GlobalName::from_path("std.print"),
        vec![],
        vec![],
        BasicNodeLogic::new(node_std_print),
    );
}

fn node_std_print(
    env: Arc<Environment>,
    inputs: HashMap<StringName, Var>,
) -> Result<HashMap<StringName, Var>, NodeError> {
    let text = get_var_string(&GlobalName::from_path("std.print"), &inputs, "text".into())?;
    env.logger.info(format!("{text}"));
    Ok(HashMap::new())
}
