use godot::{
    engine::{global::Error, Engine, IScriptLanguageExtension, Script, ScriptLanguageExtension},
    prelude::*,
};

use crate::common;

use super::script::ChoreographerScript;

#[derive(GodotClass)]
#[class(tool, base=ScriptLanguageExtension)]
pub struct ChoreographerLang {
    base: Base<ScriptLanguageExtension>,
}

impl ChoreographerLang {
    pub fn singleton() -> Option<Gd<ChoreographerLang>> {
        for i in 0..Engine::singleton().get_script_language_count() {
            let lang_opt = Engine::singleton().get_script_language(i);
            if let Some(lang) = lang_opt {
                let choreo: Result<Gd<ChoreographerLang>, _> = lang.try_cast();
                if let Ok(c) = choreo {
                    return Some(c);
                }
            }
        }
        None
    }
}

#[godot_api]
impl IScriptLanguageExtension for ChoreographerLang {
    fn init(base: Base<Self::Base>) -> Self {
        Self { base }
    }

    fn get_name(&self) -> GString {
        GString::from(common::PLUGIN_NAME)
    }

    fn init_ext(&mut self) {}

    fn get_type(&self) -> GString {
        GString::from("Choreographer")
    }

    fn get_extension(&self) -> GString {
        GString::from("c11r")
    }

    fn finish(&mut self) {}

    fn get_reserved_words(&self) -> PackedStringArray {
        PackedStringArray::new()
    }

    fn is_control_flow_keyword(&self, _keyword: GString) -> bool {
        false
    }

    fn get_comment_delimiters(&self) -> PackedStringArray {
        PackedStringArray::new()
    }

    fn get_doc_comment_delimiters(&self) -> PackedStringArray {
        PackedStringArray::new()
    }

    fn get_string_delimiters(&self) -> PackedStringArray {
        PackedStringArray::new()
    }
    fn make_template(
        &self,
        _template: GString,
        _class_name: GString,
        _base_class_name: GString,
    ) -> Option<Gd<Script>> {
        // TODO: implement templating creation (from raw source code files???)
        if let Some(obj) = self.create_script() {
            let script_option: Result<Gd<ChoreographerScript>, _> = obj.try_cast();
            if let Ok(script) = script_option {
                return Some(script.upcast());
            }
        }
        None
    }

    fn get_built_in_templates(&self, _object: StringName) -> Array<Dictionary> {
        Array::new()
    }

    fn is_using_templates(&mut self) -> bool {
        false // TODO implement
    }

    fn validate(
        &self,
        _script: GString,
        _path: GString,
        _validate_functions: bool,
        _validate_errors: bool,
        _validate_warnings: bool,
        _validate_safe_lines: bool,
    ) -> Dictionary {
        Dictionary::new()
    }

    fn validate_path(&self, _path: GString) -> GString {
        GString::from("")
    }

    fn create_script(&self) -> Option<Gd<Object>> {
        Some(ChoreographerScript::new_gd().upcast())
    }

    fn has_named_classes(&self) -> bool {
        true
    }

    fn supports_builtin_mode(&self) -> bool {
        false
    }

    fn supports_documentation(&self) -> bool {
        true
    }

    fn can_inherit_from_file(&self) -> bool {
        true
    }

    fn find_function(&self, _class_name: GString, _function_name: GString) -> i32 {
        0
    }

    fn make_function(
        &self,
        _class_name: GString,
        _function_name: GString,
        _function_args: PackedStringArray,
    ) -> GString {
        GString::from("")
    }

    fn open_in_external_editor(&mut self, _script: Gd<Script>, _line: i32, _column: i32) -> Error {
        // TODO: maybe here is where we implement opening within the editor??
        Error::OK
    }

    fn overrides_external_editor(&mut self) -> bool {
        true
    }

    fn complete_code(&self, _code: GString, _path: GString, _owner: Gd<Object>) -> Dictionary {
        Dictionary::new()
    }

    fn lookup_code(
        &self,
        _code: GString,
        _symbol: GString,
        _path: GString,
        _owner: Gd<Object>,
    ) -> Dictionary {
        Dictionary::new()
    }

    fn auto_indent_code(&self, code: GString, _from_line: i32, _to_line: i32) -> GString {
        code
    }

    fn add_global_constant(&mut self, _name: StringName, _value: Variant) {}

    fn add_named_global_constant(&mut self, _name: StringName, _value: Variant) {}

    fn remove_named_global_constant(&mut self, _name: StringName) {}

    fn thread_enter(&mut self) {}

    fn thread_exit(&mut self) {}

    fn debug_get_error(&self) -> GString {
        GString::from("")
    }

    fn debug_get_stack_level_count(&self) -> i32 {
        0
    }

    fn debug_get_stack_level_line(&self, _level: i32) -> i32 {
        0
    }

    fn debug_get_stack_level_function(&self, _level: i32) -> GString {
        GString::from("")
    }

    fn debug_get_stack_level_locals(
        &mut self,
        _level: i32,
        _max_subitems: i32,
        _max_depth: i32,
    ) -> Dictionary {
        Dictionary::new()
    }

    fn debug_get_stack_level_members(
        &mut self,
        _level: i32,
        _max_subitems: i32,
        _max_depth: i32,
    ) -> Dictionary {
        Dictionary::new()
    }

    unsafe fn debug_get_stack_level_instance(&mut self, _level: i32) -> *mut std::ffi::c_void {
        std::ptr::null_mut::<std::ffi::c_void>()
    }

    fn debug_get_globals(&mut self, _max_subitems: i32, _max_depth: i32) -> Dictionary {
        Dictionary::new()
    }

    fn debug_parse_stack_level_expression(
        &mut self,
        _level: i32,
        _expression: GString,
        _max_subitems: i32,
        _max_depth: i32,
    ) -> GString {
        GString::new()
    }

    fn debug_get_current_stack_info(&mut self) -> Array<Dictionary> {
        Array::new()
    }

    fn reload_all_scripts(&mut self) {}

    fn reload_tool_script(&mut self, _script: Gd<Script>, _soft_reload: bool) {}

    fn get_recognized_extensions(&self) -> PackedStringArray {
        PackedStringArray::from_iter(vec!["c11r".to_godot()])
    }

    fn get_public_functions(&self) -> Array<Dictionary> {
        Array::new()
    }

    fn get_public_constants(&self) -> Dictionary {
        Dictionary::new()
    }

    fn get_public_annotations(&self) -> Array<Dictionary> {
        Array::new()
    }

    fn profiling_start(&mut self) {}

    fn profiling_stop(&mut self) {}

    unsafe fn profiling_get_accumulated_data(
        &mut self,
        _info_array: *mut godot::engine::native::ScriptLanguageExtensionProfilingInfo,
        _info_max: i32,
    ) -> i32 {
        0
    }

    unsafe fn profiling_get_frame_data(
        &mut self,
        _info_array: *mut godot::engine::native::ScriptLanguageExtensionProfilingInfo,
        _info_max: i32,
    ) -> i32 {
        0
    }

    fn frame(&mut self) {}

    fn handles_global_class_type(&self, _type_: GString) -> bool {
        false
    }

    fn get_global_class_name(&self, _path: GString) -> Dictionary {
        Dictionary::new()
    }
}
