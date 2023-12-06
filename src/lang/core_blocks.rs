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
    fn new() -> Self;

    /// Processes the individual block, providing a vec of values and returning the output values
    fn process_block(&mut self, inputs: Vec<&BlockValue>) -> Vec<BlockValue>;

    /// returns the information of inputs
    fn get_input_ports(&self) -> Vec<BlockPort>;

    // returns the information of outputs
    fn get_output_ports(&self) -> Vec<BlockPort>;
}

pub struct BlockIfElse {
    inputs: Vec<BlockPort>,
    outputs: Vec<BlockPort>,
}

impl IBlock for BlockIfElse {
    fn new() -> Self {
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
}
