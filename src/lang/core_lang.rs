use godot::{
    engine::{
        file_access::ModeFlags, global::Error, Engine, FileAccess, IScriptExtension,
        IScriptLanguageExtension, Script, ScriptExtension, ScriptLanguage, ScriptLanguageExtension,
    },
    prelude::*,
};

#[derive(GodotClass)]
#[class(tool, base=ScriptLanguageExtension)]
pub struct ChoreographerLang {
    #[base]
    base: Base<ScriptLanguageExtension>,
}

#[derive(GodotClass)]
#[class(tool, base=ScriptExtension)]
pub struct ChoreographerScript {
    #[base]
    base: Base<ScriptExtension>,
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
        GString::from("Choreographer")
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
        true
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
        Error::OK
    }

    fn overrides_external_editor(&mut self) -> bool {
        false
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

#[godot_api]
impl IScriptExtension for ChoreographerScript {
    fn init(base: Base<Self::Base>) -> Self {
        Self { base }
    }

    fn editor_can_reload_from_file(&mut self) -> bool {
        true
    }

    unsafe fn placeholder_erased(&mut self, _placeholder: *mut std::ffi::c_void) {}

    fn can_instantiate(&self) -> bool {
        true
    }

    fn get_base_script(&self) -> Option<Gd<Script>> {
        None
    }

    fn get_global_name(&self) -> StringName {
        StringName::from("")
    }

    fn inherits_script(&self, _script: Gd<Script>) -> bool {
        false
    }

    fn get_instance_base_type(&self) -> StringName {
        StringName::from("")
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
        true
    }

    fn get_source_code(&self) -> GString {
        if let Some(file) = FileAccess::open(self.base.get_path(), ModeFlags::READ) {
            file.get_as_text()
        } else {
            GString::from("")
        }
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
    fn has_method(&self, _method: StringName) -> bool {
        false
    }

    fn has_static_method(&self, _method: StringName) -> bool {
        false
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
        false
    }

    fn get_property_default_value(&self, _property: StringName) -> Variant {
        Variant::nil()
    }

    fn update_exports(&mut self) {}

    fn get_script_method_list(&self) -> Array<Dictionary> {
        Array::new()
    }

    fn get_script_property_list(&self) -> Array<Dictionary> {
        Array::new()
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
