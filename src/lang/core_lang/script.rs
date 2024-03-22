#![allow(dead_code)] // ignores dead code in entire file

use core::fmt;
use std::rc::Rc;

use godot::{
    engine::{
        global::{Error, PropertyHint},
        IScriptExtension, Script, ScriptExtension, ScriptLanguage,
    },
    prelude::*,
};

#[derive(Debug)]
struct MethodInfo {
    name: StringName,
    signature: Vec<VariantType>,
    docs: GString,
    is_static: bool,
}
#[derive(Debug)]
struct PropertyInfo {
    name: StringName,
    value: Variant,
    type_: VariantType,
    hint: PropertyHint,
    hint_string: GString,
    docs: GString,
    is_const: bool,
    is_static: bool,
}

#[derive(Debug)]
pub struct PortBind {
    pub input: bool,
    pub block_index: usize,
    pub port_index: usize,
}

pub struct BlockStore {
    pub block: Rc<Box<dyn IBlock>>,
}
impl fmt::Debug for BlockStore {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "Block<{}>", self.block.get_block_name())
    }
}

use crate::lang::core_blocks::IBlock;

use super::lang::ChoreographerLang;
#[derive(GodotClass, Debug)]
#[class(tool, init, base=ScriptExtension)]
pub struct ChoreographerScript {
    pub parent_class: Option<GString>,
    pub class_name: Option<StringName>,
    pub instance_base_type: Option<StringName>,
    methods: Vec<MethodInfo>,
    properties: Vec<PropertyInfo>,
    pub blocks: Vec<BlockStore>,
    pub port_binds: Vec<PortBind>,
    base: Base<ScriptExtension>,
}

impl ChoreographerScript {
    /// Loads necessary information for use from externally loaded data (blocks, connections, and variables)
    pub fn construct_data(&mut self) {
        // todo!()
    }
}

#[godot_api]
impl IScriptExtension for ChoreographerScript {
    fn editor_can_reload_from_file(&mut self) -> bool {
        true
    }

    unsafe fn placeholder_erased(&mut self, _placeholder: *mut std::ffi::c_void) {}

    fn can_instantiate(&self) -> bool {
        true
    }

    fn get_base_script(&self) -> Option<Gd<Script>> {
        Some(self.base().clone().upcast())
    }

    fn get_global_name(&self) -> StringName {
        self.class_name.clone().unwrap_or(StringName::from(""))
    }

    fn inherits_script(&self, script: Gd<Script>) -> bool {
        if let Some(parent) = self.parent_class.clone() {
            return script.is_class(parent);
        }
        false
    }

    fn get_instance_base_type(&self) -> StringName {
        self.instance_base_type
            .clone()
            .unwrap_or(StringName::from(""))
    }

    unsafe fn instance_create(&self, _for_object: Gd<Object>) -> *mut std::ffi::c_void {
        std::ptr::null_mut::<std::ffi::c_void>()
    }

    unsafe fn placeholder_instance_create(&self, _for_object: Gd<Object>) -> *mut std::ffi::c_void {
        std::ptr::null_mut::<std::ffi::c_void>()
    }

    fn instance_has(&self, _object: Gd<Object>) -> bool {
        false
    }

    fn has_source_code(&self) -> bool {
        false
    }

    fn get_source_code(&self) -> GString {
        GString::from("")
    }

    fn set_source_code(&mut self, _code: GString) {}

    fn reload(&mut self, _keep_state: bool) -> Error {
        Error::OK
    }

    fn get_documentation(&self) -> Array<Dictionary> {
        Array::new()
    }

    fn get_class_icon_path(&self) -> GString {
        GString::new()
    }

    fn has_method(&self, method: StringName) -> bool {
        self.methods.iter().any(|p| p.name == method)
    }

    fn has_static_method(&self, method: StringName) -> bool {
        self.methods.iter().any(|p| p.name == method && p.is_static)
    }

    fn get_method_info(&self, _method: StringName) -> Dictionary {
        Dictionary::new()
    }

    fn is_tool(&self) -> bool {
        false
    }

    fn is_valid(&self) -> bool {
        true
    }

    fn is_abstract(&self) -> bool {
        false
    }

    fn get_language(&self) -> Option<Gd<ScriptLanguage>> {
        ChoreographerLang::singleton().map(|lang| lang.upcast())
    }

    fn has_script_signal(&self, _signal: StringName) -> bool {
        false
    }

    fn get_script_signal_list(&self) -> Array<Dictionary> {
        Array::new()
    }

    fn has_property_default_value(&self, _property: StringName) -> bool {
        true
    }

    fn get_property_default_value(&self, property: StringName) -> Variant {
        if let Some(prop) = self.properties.iter().find(|p| p.name == property) {
            prop.type_.to_variant()
        } else {
            Variant::default()
        }
    }

    fn update_exports(&mut self) {}

    fn get_script_method_list(&self) -> Array<Dictionary> {
        Array::from_iter(self.methods.iter().map(|info| {
            let mut dict = Dictionary::new();
            dict.insert("name", info.name.clone());
            dict
        }))
    }

    fn get_script_property_list(&self) -> Array<Dictionary> {
        Array::from_iter(self.properties.iter().map(|info| {
            let mut dict = Dictionary::new();
            dict.insert("name", info.name.clone());
            dict.insert("type", info.type_);
            dict.insert("hint", info.hint.ord());
            dict.insert("hint_string", info.hint_string.clone());
            dict
        }))
    }

    fn get_member_line(&self, _member: StringName) -> i32 {
        0
    }

    fn get_constants(&self) -> Dictionary {
        Dictionary::new()
    }

    fn get_members(&self) -> Array<StringName> {
        Array::new()
    }

    fn is_placeholder_fallback_enabled(&self) -> bool {
        false
    }

    fn get_rpc_config(&self) -> Variant {
        Variant::nil()
    }

    fn setup_local_to_scene(&mut self) {}
}
