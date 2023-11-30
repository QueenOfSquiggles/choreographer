use crate::blocks::*;
use godot::engine::{
    self, Engine, IScript, IScriptLanguageExtension, Script, ScriptLanguageExtension,
};
use godot::prelude::*;

pub fn init_lang() {
    let mut engine = Engine::singleton();
    let lang: Gd<C11RLang> = Gd::new_default();
    engine.register_script_language(lang.upcast());
}

pub fn deinit_lang() {
    //TODO: is anything needed for this?? The extension cannot be disabled with removing the files from the project, and the next time the engine is initialized, it will simply not initialize anything. I'll leave this in here just in case.
}

#[derive(GodotClass)]
#[class(tool, base=Script)]
struct C11RScript {
    // node_graph: DirectedCsrGraph<C11RBlock>,
    nodes: Vec<Gd<Block>>,
    #[base]
    base: Base<Script>,
}

#[godot_api]
impl IScript for C11RScript {
    fn init(base: Base<Self::Base>) -> Self {
        let mut temp = Self {
            base,
            nodes: Vec::new(),
        };
        let block: Gd<Block> = Gd::new_default();

        temp.nodes.push(block);
        temp
    }

    fn to_string(&self) -> GString {
        GString::from("")
    }

    fn on_notification(&mut self, _what: engine::notify::ObjectNotification) {}
}

#[derive(GodotClass)]
#[class(tool, base=ScriptLanguageExtension)]
struct C11RLang {
    script_name: GString,
    file_ext: GString,
    file_ext_arr: PackedStringArray,
    #[base]
    base: Base<ScriptLanguageExtension>,
}

// #[godot_api]
// impl C11RLang {}

#[godot_api]
impl IScriptLanguageExtension for C11RLang {
    fn init(base: Base<ScriptLanguageExtension>) -> Self {
        let file_ext = GString::from("c11r");
        let mut file_ext_arr = PackedStringArray::new();
        file_ext_arr.insert(0, file_ext.clone());
        Self {
            base,
            script_name: GString::from("Choreographer"),
            file_ext,
            file_ext_arr,
        }
    }

    fn to_string(&self) -> GString {
        self.base.to_string().to_godot()
    }

    fn on_notification(&mut self, what: godot::engine::notify::ObjectNotification) {
        match what {
            engine::notify::ObjectNotification::Postinitialize => {
                godot_print!("C11R Notification: Post Init")
            }
            engine::notify::ObjectNotification::Predelete => {
                godot_print!("C11R Notification: Pre Delete")
            }
            engine::notify::ObjectNotification::Unknown(notify_code) => {
                godot_print!("C11R Notification: Code[{notify_code}]")
            }
        }
    }

    fn get_name(&self) -> GString {
        self.script_name.clone()
    }

    fn init_ext(&mut self) {}

    fn get_type(&self) -> GString {
        self.file_ext.clone()
    }

    fn get_extension(&self) -> GString {
        self.file_ext.clone()
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

    fn get_string_delimiters(&self) -> PackedStringArray {
        PackedStringArray::new()
    }

    fn make_template(
        &self,
        _template: GString,
        _class_name: GString,
        _base_class_name: GString,
    ) -> Option<Gd<Script>> {
        None
    }

    fn get_built_in_templates(&self, _object: StringName) -> Array<Dictionary> {
        Array::new()
    }

    fn is_using_templates(&mut self) -> bool {
        false
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

    fn create_script(&self) -> Option<Gd<engine::Object>> {
        let script: Gd<C11RScript> = Gd::new_default();
        Some(script.upcast())
    }

    fn has_named_classes(&self) -> bool {
        false
    }

    fn supports_builtin_mode(&self) -> bool {
        false
    }

    fn supports_documentation(&self) -> bool {
        false
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

    fn open_in_external_editor(
        &mut self,
        _script: Gd<engine::Script>,
        _line: i32,
        _column: i32,
    ) -> engine::global::Error {
        engine::global::Error::OK
    }

    fn overrides_external_editor(&mut self) -> bool {
        false
    }

    fn complete_code(
        &self,
        _code: GString,
        _path: GString,
        _owner: Gd<engine::Object>,
    ) -> Dictionary {
        Dictionary::new()
    }

    fn lookup_code(
        &self,
        _code: GString,
        _symbol: GString,
        _path: GString,
        _owner: Gd<engine::Object>,
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
        0 as *mut std::ffi::c_void
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
        GString::from("")
    }

    fn debug_get_current_stack_info(&mut self) -> Array<Dictionary> {
        Array::new()
    }

    fn reload_all_scripts(&mut self) {}

    fn reload_tool_script(&mut self, _script: Gd<engine::Script>, _soft_reload: bool) {}

    fn get_recognized_extensions(&self) -> PackedStringArray {
        self.file_ext_arr.clone()
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

    fn handles_global_class_type(&self, _type: GString) -> bool {
        false
    }

    fn get_global_class_name(&self, _path: GString) -> Dictionary {
        Dictionary::new()
    }
}
