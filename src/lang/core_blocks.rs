use godot::prelude::*;

pub const ICON_EVENT: &str = "res://addons/choreographer/assets/event.svg";
pub const ICON_EMPTY: &str = "";

#[derive(Debug, Clone)]
pub enum BlockValue {
    Var { value: Variant, typing: VariantType },
    Event { enabled: bool },
}

#[derive(Debug, Clone)]
#[allow(dead_code)]
pub struct BlockPort {
    value: BlockValue,
    label: String,
    icon_path: String,
}

pub trait IBlock {
    // fn new() -> Self;

    /// Processes the individual block, providing a vec of values and returning the output values
    fn process_block(&mut self, inputs: Vec<&BlockValue>) -> Vec<BlockValue>;

    /// returns the information of inputs
    fn get_input_ports(&self) -> Vec<BlockPort>;

    // returns the information of outputs
    fn get_output_ports(&self) -> Vec<BlockPort>;

    fn get_block_name(&self) -> String;
}

pub struct BlockIfElse {
    inputs: Vec<BlockPort>,
    outputs: Vec<BlockPort>,
}

impl Default for BlockIfElse {
    fn default() -> Self {
        Self::new()
    }
}

impl BlockIfElse {
    pub fn new() -> Self {
        Self {
            inputs: vec![
                BlockPort {
                    value: BlockValue::Event { enabled: true },
                    label: "Event".into(),
                    icon_path: ICON_EVENT.into(),
                },
                BlockPort {
                    value: BlockValue::Var {
                        value: Variant::nil(),
                        typing: VariantType::Bool,
                    },
                    label: "Predicate".into(),
                    icon_path: ICON_EMPTY.into(),
                },
            ],
            outputs: vec![
                BlockPort {
                    value: BlockValue::Event { enabled: true },
                    label: "If".into(),
                    icon_path: ICON_EVENT.into(),
                },
                BlockPort {
                    value: BlockValue::Event { enabled: true },
                    label: "Else".into(),
                    icon_path: ICON_EVENT.into(),
                },
            ],
        }
    }
}

impl IBlock for BlockIfElse {
    fn process_block(&mut self, inputs: Vec<&BlockValue>) -> Vec<BlockValue> {
        #[allow(unused_variables)]
        if let Some(BlockValue::Var { value, typing }) = inputs.first() {
            let mut predicate = true;
            if let Ok(is_active) = bool::try_from_variant(value) {
                predicate = is_active;
            }
            self.outputs[0].value = BlockValue::Event { enabled: predicate };
            self.outputs[1].value = BlockValue::Event {
                enabled: !predicate,
            };
        }
        self.outputs
            .iter()
            .map(|in_val| in_val.value.clone())
            .collect()
    }

    fn get_input_ports(&self) -> Vec<BlockPort> {
        self.inputs.clone()
    }

    fn get_output_ports(&self) -> Vec<BlockPort> {
        self.outputs.clone()
    }

    fn get_block_name(&self) -> String {
        "IfElse".to_owned()
    }
}

#[allow(dead_code)] // need to hold a reference to the object instance so Godot doesn't drop it!
pub struct BlockCustom {
    object: Gd<Object>,
    call_process: Callable,
    call_get_inputs: Callable,
    call_get_outputs: Callable,
}

#[allow(unused_variables)]
impl IBlock for BlockCustom {
    fn process_block(&mut self, inputs: Vec<&BlockValue>) -> Vec<BlockValue> {
        let ret = self.call_process.callv(
            inputs
                .iter()
                .map(|value| match value {
                    BlockValue::Var { value, typing } => value.to_owned(),
                    BlockValue::Event { enabled } => enabled.to_variant(),
                })
                .collect(),
        );
        let opt_arr_val: Result<Array<Variant>, _> = Array::try_from_variant(&ret);
        let Ok(arr_val) = opt_arr_val else {
            return Vec::new();
        };
        let ports = self.get_output_ports();
        let mut values: Vec<BlockValue> = Vec::new();
        for (i, port) in ports.iter().enumerate() {
            if i >= arr_val.len() {
                // fill remaining with nil values
                values.push(BlockValue::Var {
                    value: Variant::nil(),
                    typing: VariantType::Nil,
                });
                break;
            }
            let n_val = match port.value.clone() {
                BlockValue::Var { value, typing } => BlockValue::Var {
                    value: arr_val.get(i),
                    typing: arr_val.get(i).get_type(),
                },
                BlockValue::Event { enabled } => BlockValue::Event {
                    enabled: bool::try_from_variant(&arr_val.get(i)).unwrap_or(false),
                },
            };
            values.push(n_val);
        }
        values
    }

    fn get_input_ports(&self) -> Vec<BlockPort> {
        self.get_ports_from_callable(&self.call_get_inputs)
    }

    fn get_output_ports(&self) -> Vec<BlockPort> {
        self.get_ports_from_callable(&self.call_get_outputs)
    }

    fn get_block_name(&self) -> String {
        String::try_from_variant(&self.object.get(StringName::from("name")))
            .unwrap_or("Invalid Custom Block".to_owned())
    }
}

impl BlockCustom {
    pub fn new(object: Gd<Object>) -> Self {
        let call_process = object.callable("process_block");
        let call_get_inputs = object.callable("get_input_ports");
        let call_get_outputs = object.callable("get_output_ports");
        Self {
            object,
            call_process,
            call_get_inputs,
            call_get_outputs,
        }
    }

    fn get_ports_from_callable(&self, call: &Callable) -> Vec<BlockPort> {
        let var = call.callv(Array::new());
        let res_arr: Result<Array<Dictionary>, _> = Array::try_from_variant(&var);
        let Ok(port_array) = res_arr else {
            return Vec::new();
        };
        let mut return_values: Vec<BlockPort> = Vec::new();
        for val in port_array.iter_shared() {
            let port_type = val.get("value").unwrap_or(false.to_variant());
            let label = String::try_from_variant(&val.get("label").unwrap_or("".to_variant()))
                .unwrap_or_default();
            let icon_path =
                String::try_from_variant(&val.get("icon_path").unwrap_or("".to_variant()))
                    .unwrap_or_default();
            let value: BlockValue = if let Ok(string_val) = String::try_from_variant(&port_type) {
                if string_val == "event" {
                    // I'd really like to chain with the iflet above!!
                    BlockValue::Event { enabled: false }
                } else {
                    BlockValue::Var {
                        value: port_type.clone(),
                        typing: port_type.get_type(),
                    }
                }
            } else {
                BlockValue::Var {
                    value: port_type.clone(),
                    typing: port_type.get_type(),
                }
            };

            return_values.push(BlockPort {
                value,
                label,
                icon_path,
            });
        }
        return_values
    }
}
