use godot::engine::file_access::ModeFlags;
use godot::engine::global::Error;
use godot::engine::{
    Engine, FileAccess, IResourceFormatLoader, IResourceFormatSaver, ResourceFormatLoader,
    ResourceFormatSaver,
};
use godot::prelude::*;

use super::core_lang::ChoreographerScript;

#[derive(GodotClass)]
#[class(tool, base=ResourceFormatLoader)]
pub struct ChoreographerLoader {
    #[base]
    base: Base<ResourceFormatLoader>,
}

#[derive(GodotClass)]
#[class(tool, base=ResourceFormatSaver)]
pub struct ChoreographerSaver {
    #[base]
    base: Base<ResourceFormatSaver>,
}

#[godot_api]
impl IResourceFormatLoader for ChoreographerLoader {
    fn init(base: Base<Self::Base>) -> Self {
        Self { base }
    }

    fn get_recognized_extensions(&self) -> PackedStringArray {
        PackedStringArray::from_iter(vec!["c11r".to_godot()])
    }

    fn recognize_path(&self, _path: GString, _type_: StringName) -> bool {
        true
    }

    fn handles_type(&self, type_: StringName) -> bool {
        type_ == StringName::from("Choreographer")
    }

    fn get_resource_type(&self, _path: GString) -> GString {
        GString::from("Choreographer")
    }

    fn get_resource_script_class(&self, _path: GString) -> GString {
        GString::from("")
    }

    fn get_resource_uid(&self, _path: GString) -> i64 {
        0i64
    }

    fn get_dependencies(&self, _path: GString, _add_types: bool) -> PackedStringArray {
        PackedStringArray::new()
    }

    fn rename_dependencies(&self, _path: GString, _renames: Dictionary) -> Error {
        Error::OK
    }

    fn exists(&self, _path: GString) -> bool {
        true
    }

    fn get_classes_used(&self, _path: GString) -> PackedStringArray {
        PackedStringArray::new()
    }

    fn load(
        &self,
        _path: GString,
        original_path: GString,
        _use_sub_threads: bool,
        _cache_mode: i32,
    ) -> Variant {
        let Some(mut file) = FileAccess::open(original_path, ModeFlags::READ) else {
            return Variant::nil();
        };
        let str_data = file.get_as_text().to_string();
        file.close();

        let data = rusty_parsing::parse_string(str_data, !Engine::singleton().is_editor_hint());
        godot_print!("Found data {:?}", data);

        ChoreographerScript::new_gd().to_variant()
    }
}

#[godot_api]
impl IResourceFormatSaver for ChoreographerSaver {
    fn init(base: Base<Self::Base>) -> Self {
        Self { base }
    }

    fn save(&mut self, _resource: Gd<Resource>, _path: GString, _flags: u32) -> Error {
        // let Some(mut file) = FileAccess::open(path, ModeFlags::WRITE) else {
        //     return FileAccess::get_open_error();
        // };
        // file.store_string("this is a c11r script!".to_godot());
        // file.close();

        Error::OK
    }

    fn set_uid(&mut self, _path: GString, _uid: i64) -> Error {
        Error::OK
    }

    fn recognize(&self, resource: Gd<Resource>) -> bool {
        let script: Result<Gd<ChoreographerScript>, _> = resource.try_cast();
        script.is_ok()
    }

    fn get_recognized_extensions(&self, _resource: Gd<Resource>) -> PackedStringArray {
        PackedStringArray::from_iter(vec!["c11r".to_godot()])
    }

    fn recognize_path(&self, _resource: Gd<Resource>, _path: GString) -> bool {
        true
    }
}

#[allow(dead_code, unused_variables)]
mod rusty_parsing {
    use std::collections::HashMap;

    use xml::{name::OwnedName, reader::XmlEvent, ParserConfig};

    #[derive(Debug, PartialEq, PartialOrd)]
    enum Category {
        Godot,
        Dependencies,
        Vars,
        Blocks,
        Connections,
        Metadata,
        None,
    }

    #[derive(Debug, Clone, Default)]
    pub struct GodotMetaData {
        uid: i64,
    }
    #[derive(Debug, Clone, Default)]
    pub struct GodotResourceDep {
        uid: i64,
        path: String,
    }
    #[derive(Debug, Clone, Default)]
    pub struct GodotVar {
        type_: i32,
        name: String,
        value: String,
        isconst: bool,
    }
    #[derive(Debug, Clone, Default)]
    pub struct Block {
        id: i64,
        type_: String,
        extra_data: HashMap<String, String>,
    }
    #[derive(Debug, Clone, Default)]
    pub struct Connection {
        left_id: i32,
        left_port: i32,
        right_id: i32,
        right_port: i32,
    }
    #[derive(Debug, Clone, Default)]
    pub struct DocString {
        id: i64,
        docstring: String,
    }
    #[derive(Debug, Clone, Default)]
    pub struct EditorMeta {
        id: i64,
        position: (f32, f32),
    }

    #[derive(Debug, Clone, Default)]
    pub struct Comment {
        id: i64,
        position: (f32, f32),
        text: String,
    }
    #[derive(Debug, Clone, Default)]
    pub struct GodotScriptData {
        godot: GodotMetaData,
        dependencies: Vec<GodotResourceDep>,
        vars: Vec<GodotVar>,
        blocks: Vec<Block>,
        connections: Vec<Connection>,
        docs: Vec<DocString>,
        gui: Vec<EditorMeta>,
        comments: Vec<Comment>,
    }

    pub fn parse_string(data: String, skip_meta: bool) -> GodotScriptData {
        type ParserFunc = fn(&mut GodotScriptData, &XmlEvent, &mut Vec<u8>);
        let mut bytes = data.as_bytes();
        let reader = ParserConfig::new()
            .trim_whitespace(true)
            .ignore_comments(true)
            .coalesce_characters(true)
            .create_reader(&mut bytes);
        let mut current_category = Category::None;
        let mut script_data: GodotScriptData = Default::default();
        let mut event_buffer: Vec<XmlEvent> = Vec::new();

        for e in reader {
            let Ok(event) = e else {
                eprintln!("XML Parse Error: {}", e.unwrap_err());
                break;
            };
            event_buffer.push(event.to_owned());

            match event.to_owned() {
                XmlEvent::StartElement {
                    name,
                    attributes,
                    namespace,
                } => {
                    if let Some(cat) = parse_tag(&name) {
                        current_category = cat;
                        event_buffer.clear();
                        event_buffer.push(event.to_owned());
                        if skip_meta && current_category == Category::Metadata {
                            break;
                        }
                    }
                }
                XmlEvent::EndElement { name } => {
                    if current_category != Category::None {
                        if let Some(cat) = parse_tag(&name) {
                            if current_category == cat {
                                match current_category {
                                    Category::Godot => parse_godot(&event_buffer, &mut script_data),
                                    Category::Dependencies => {
                                        parse_deps(&event_buffer, &mut script_data);
                                    }
                                    Category::Vars => parse_vars(&event_buffer, &mut script_data),
                                    Category::Blocks => {
                                        parse_blocks(&event_buffer, &mut script_data)
                                    }
                                    Category::Connections => {
                                        parse_connections(&event_buffer, &mut script_data)
                                    }
                                    Category::Metadata => {
                                        parse_metadata(&event_buffer, &mut script_data)
                                    }
                                    Category::None => (),
                                }
                            }
                        }
                    }
                }
                _ => (),
            }
        }
        script_data
    }

    fn parse_tag(tag: &OwnedName) -> Option<Category> {
        match tag.to_string().as_str() {
            "godot" => Some(Category::Godot),
            "dependencies" => Some(Category::Dependencies),
            "vars" => Some(Category::Vars),
            "blocks" => Some(Category::Blocks),
            "connections" => Some(Category::Connections),
            "metadata" => Some(Category::Metadata),
            _ => None,
        }
    }
    fn parse_godot(event_buffer: &Vec<XmlEvent>, data: &mut GodotScriptData) {
        for event in event_buffer {
            let XmlEvent::StartElement {
                name,
                attributes,
                namespace,
            } = event
            else {
                continue;
            };

            if name.to_string() != "godot" {
                continue;
            }
            let mut uid = 0i64;
            for a in attributes {
                if a.name.to_string() == "uid" {
                    if let Ok(id) = a.value.to_string().parse::<i64>() {
                        uid = id;
                    }
                }
            }
            data.godot.uid = uid;
        }
    }

    fn parse_deps(event_buffer: &Vec<XmlEvent>, data: &mut GodotScriptData) {
        for event in event_buffer {
            let XmlEvent::StartElement {
                name,
                attributes,
                namespace,
            } = event
            else {
                continue;
            };
            if name.to_string() != "res" {
                continue;
            }
            let mut uid = 0i64;
            let mut path = String::new();
            for a in attributes {
                match a.name.to_string().as_str() {
                    "guid" => {
                        if let Ok(value) = a.value.parse::<i64>() {
                            uid = value;
                        }
                    }
                    "path" => path = a.value.to_string(),
                    _ => (),
                }
            }
            data.dependencies.push(GodotResourceDep { uid, path });
        }
    }

    fn parse_vars(event_buffer: &Vec<XmlEvent>, data: &mut GodotScriptData) {
        for event in event_buffer {
            let XmlEvent::StartElement {
                name,
                attributes,
                namespace,
            } = event
            else {
                continue;
            };
            if name.to_string() != "item" {
                continue;
            }
            let mut type_ = 0i32;
            let mut name = String::new();
            let mut value = String::new();
            let mut isconst = false;
            for a in attributes {
                match a.name.to_string().as_str() {
                    "type" => {
                        if let Ok(value) = a.value.parse::<i32>() {
                            type_ = value;
                        }
                    }
                    "name" => name = a.value.to_string(),
                    "value" => value = a.value.to_string(),
                    "isconst" => {
                        if let Ok(value) = a.value.parse::<bool>() {
                            isconst = value;
                        }
                    }
                    _ => (),
                }
            }
            data.vars.push(GodotVar {
                type_,
                name,
                value,
                isconst,
            });
        }
    }
    fn parse_blocks(event_buffer: &Vec<XmlEvent>, data: &mut GodotScriptData) {
        for event in event_buffer {
            let XmlEvent::StartElement {
                name,
                attributes,
                namespace,
            } = event
            else {
                continue;
            };
            if name.to_string() != "block" {
                continue;
            }
            let mut type_ = String::new();
            let mut id = 0i64;
            let mut extra_data = HashMap::new();
            for a in attributes {
                match a.name.to_string().as_str() {
                    "type" => type_ = a.value.to_string(),
                    "id" => {
                        if let Ok(value) = a.value.parse::<i64>() {
                            id = value;
                        }
                    }
                    _ => {
                        extra_data.insert(a.name.to_string(), a.value.to_string());
                    }
                }
            }
            data.blocks.push(Block {
                id,
                type_,
                extra_data,
            });
        }
    }

    fn parse_connections(event_buffer: &Vec<XmlEvent>, data: &mut GodotScriptData) {
        for event in event_buffer {
            let XmlEvent::StartElement {
                name,
                attributes,
                namespace,
            } = event
            else {
                continue;
            };
            if name.to_string() != "link" {
                continue;
            }
            let mut conn = Connection {
                left_id: -1,
                left_port: -1,
                right_id: -1,
                right_port: -1,
            };
            for a in attributes {
                if let Ok(value) = a.value.parse::<i32>() {
                    match a.name.to_string().as_str() {
                        "left_id" => conn.left_id = value,
                        "left_port" => conn.left_port = value,
                        "right_id" => conn.right_id = value,
                        "right_port" => conn.right_port = value,
                        _ => (),
                    }
                }
            }
            data.connections.push(conn);
        }
    }
    fn parse_metadata(event_buffer: &Vec<XmlEvent>, data: &mut GodotScriptData) {
        let mut buffer = String::new();
        let mut last_start: Option<XmlEvent> = None;
        let mut current_tag = String::new();
        for event in event_buffer {
            match event {
                XmlEvent::StartElement {
                    name,
                    attributes,
                    namespace,
                } => {
                    last_start = Some(event.to_owned());
                    current_tag = name.to_string();
                }

                XmlEvent::EndElement { name } => {
                    if name.to_string() != current_tag {
                        continue;
                    }
                    let Some(ref start_event) = last_start else {
                        continue;
                    };
                    let mut tag_id = 0i64;
                    if let Some(XmlEvent::StartElement {
                        ref name,
                        ref attributes,
                        ref namespace,
                    }) = last_start
                    {
                        tag_id = attributes
                            .iter()
                            .find(|val| val.name.to_string() == "id")
                            .map(|in_val| in_val.value.parse::<i64>())
                            .unwrap_or(Ok(0i64))
                            .unwrap_or(0i64);
                    }
                    match current_tag.as_str() {
                        "doc" => {
                            data.docs.push(DocString {
                                id: tag_id,
                                docstring: buffer.clone(),
                            });
                            buffer.clear();
                        }
                        "comment" => {
                            let Some(XmlEvent::StartElement {
                                ref name,
                                ref attributes,
                                ref namespace,
                            }) = last_start
                            else {
                                continue;
                            };
                            let mut pos = (0f32, 0f32);
                            if let Some(att) =
                                attributes.iter().find(|p| p.name.to_string() == "pos")
                            {
                                let vals: Vec<f32> = att
                                    .value
                                    .to_string()
                                    .splitn(2, ';')
                                    .map(|inval| inval.parse::<f32>().unwrap_or(0f32))
                                    .collect();
                                if vals.len() == 2 {
                                    pos = (vals[0], vals[1]);
                                }
                            }
                            data.comments.push(Comment {
                                id: tag_id,
                                position: pos,
                                text: buffer.clone(),
                            });
                            buffer.clear();
                        }
                        "gui" => {
                            let Some(XmlEvent::StartElement {
                                ref name,
                                ref attributes,
                                ref namespace,
                            }) = last_start
                            else {
                                continue;
                            };
                            let id = tag_id;
                            let mut position = (0f32, 0f32);
                            if let Some(attr) =
                                attributes.iter().find(|p| p.name.to_string() == "pos")
                            {
                                let parts: Vec<f32> = attr
                                    .value
                                    .to_string()
                                    .splitn(2, ';')
                                    .map(|inval| inval.parse::<f32>().unwrap_or(0f32))
                                    .collect();
                                if parts.len() == 2 {
                                    position = (parts[0], parts[1]);
                                }
                            }
                            data.gui.push(EditorMeta { id, position });
                        }
                        _ => (),
                    }
                }
                XmlEvent::Characters(chars) => buffer.push_str(chars),
                _ => (),
            }
        }
    }

    #[cfg(test)]
    mod test {
        // Because the parent module is pure rust (no godot dependencies) we can do unit testing on it, and be assured the parsing is working correctly

        #[test]
        fn godot_meta() {
            let test_data = "<c11r><godot uid=\"001\"></godot></c11r>";
            let meta = super::parse_string(test_data.to_owned(), false);
            println!("Test data found: {:?}", meta.godot);
            assert_eq!(meta.godot.uid, 1i64);
        }

        #[test]
        fn dependencies() {
            let test_data = String::from(
                "<c11r><dependencies>
					<res guid=\"0\" path=\"res://some.tres\"></res>
					<res guid=\"1\" path=\"res://other.tres\"/>
				</dependencies></c11r>",
            );
            let meta = super::parse_string(test_data, false);
            println!("Test data found: {:?}", meta.dependencies);

            assert_eq!(meta.dependencies.len(), 2);
            assert_eq!(meta.dependencies[0].uid, 0i64);
            assert_eq!(meta.dependencies[1].uid, 1i64);
            assert_eq!(meta.dependencies[0].path, String::from("res://some.tres"));
            assert_eq!(meta.dependencies[1].path, String::from("res://other.tres"));
        }

        #[test]
        fn vars() {
            let data = String::from(
                "<c11r><vars>
					<item type=\"0\" name=\"some\" value=\"A\" isconst=\"true\"></item>
					<item type=\"1\" name=\"foo\" value=\"false\" isconst=\"false\"></item>
					<item type=\"2\" name=\"bar\" value=\"35.5\" isconst=\"true\"></item>
					<item type=\"3\" name=\"other\" value=\"nil\" isconst=\"false\"/>
				</vars></c11r>
				",
            );
            let meta = super::parse_string(data, false);

            println!("Test data found: {:?}", meta.vars);

            assert_eq!(meta.vars.len(), 4);

            // types
            assert_eq!(meta.vars[0].type_, 0i32);
            assert_eq!(meta.vars[1].type_, 1i32);
            assert_eq!(meta.vars[2].type_, 2i32);
            assert_eq!(meta.vars[3].type_, 3i32);
            // names
            assert_eq!(meta.vars[0].name, "some");
            assert_eq!(meta.vars[1].name, "foo");
            assert_eq!(meta.vars[2].name, "bar");
            assert_eq!(meta.vars[3].name, "other");
            // values
            assert_eq!(meta.vars[0].value, "A");
            assert_eq!(meta.vars[1].value, "false");
            assert_eq!(meta.vars[2].value, "35.5");
            assert_eq!(meta.vars[3].value, "nil");
            // consts
            assert_eq!(meta.vars[0].isconst, true);
            assert_eq!(meta.vars[1].isconst, false);
            assert_eq!(meta.vars[2].isconst, true);
            assert_eq!(meta.vars[3].isconst, false);
        }

        #[test]
        fn blocks() {
            let data = String::from(
                "<c11r><blocks>
					<block id=\"0\" type=\"IfElse\"></block>
					<block id=\"2\" type=\"Some\"></block>
					<block id=\"100\" type=\"Foo\"></block>
					<block id=\"001\" type=\"Bar\"/>
				</blocks></c11r>
				",
            );
            let meta = super::parse_string(data, false);

            println!("Test data found: {:?}", meta.blocks);

            assert_eq!(meta.blocks.len(), 4);

            // ids
            assert_eq!(meta.blocks[0].id, 0);
            assert_eq!(meta.blocks[1].id, 2);
            assert_eq!(meta.blocks[2].id, 100);
            assert_eq!(meta.blocks[3].id, 001);
            // types
            assert_eq!(meta.blocks[0].type_, "IfElse");
            assert_eq!(meta.blocks[1].type_, "Some");
            assert_eq!(meta.blocks[2].type_, "Foo");
            assert_eq!(meta.blocks[3].type_, "Bar");

            // types case sensitive
            assert_ne!(meta.blocks[0].type_, "ifelse");
            assert_ne!(meta.blocks[1].type_, "some");
            assert_ne!(meta.blocks[2].type_, "foo");
            assert_ne!(meta.blocks[3].type_, "bar");
        }

        #[test]
        fn connections() {
            let data = String::from(
                "<c11r><connections>
					<link left_id=\"0\" left_port=\"0\" right_id=\"0\" right_port=\"0\"></link>
					<link left_id=\"1\" left_port=\"2\" right_id=\"3\" right_port=\"4\"></link>
					<link left_id=\"26\" left_port=\"25\" right_id=\"24\" right_port=\"23\"/>
					<link left_id=\"0\" left_port=\"0\" right_id=\"0\" right_port=\"0\"/>
				</connections></c11r>
				",
            );
            let meta = super::parse_string(data, false);

            println!("Test data found: {:?}", meta.connections);

            assert_eq!(meta.connections.len(), 4);

            // left id
            assert_eq!(meta.connections[0].left_id, 0);
            assert_eq!(meta.connections[1].left_id, 1);
            assert_eq!(meta.connections[2].left_id, 26);
            assert_eq!(meta.connections[3].left_id, 0);
            // left port
            assert_eq!(meta.connections[0].left_port, 0);
            assert_eq!(meta.connections[1].left_port, 2);
            assert_eq!(meta.connections[2].left_port, 25);
            assert_eq!(meta.connections[3].left_port, 0);
            // right id
            assert_eq!(meta.connections[0].right_id, 0);
            assert_eq!(meta.connections[1].right_id, 3);
            assert_eq!(meta.connections[2].right_id, 24);
            assert_eq!(meta.connections[3].right_id, 0);
            // right port
            assert_eq!(meta.connections[0].right_port, 0);
            assert_eq!(meta.connections[1].right_port, 4);
            assert_eq!(meta.connections[2].right_port, 23);
            assert_eq!(meta.connections[3].right_port, 0);
        }

        #[test]
        fn metadata_docs() {
            let data = String::from(
                "<c11r><metadata>
					<doc id=\"001\">
						This should be trimmed
					</doc>
					<doc id=\"1\">Some Text With Trailing Whitespace  </doc>
					<doc id=\"2\"/>
				</metadata></c11r>
				",
            );
            let meta = super::parse_string(data, false);

            println!("Test data found: {:?}", meta.docs);

            assert_eq!(meta.docs.len(), 3);

            // ids
            assert_eq!(meta.docs[0].id, 1);
            assert_eq!(meta.docs[1].id, 1);
            assert_eq!(meta.docs[2].id, 2);
            // docstrings
            assert_eq!(meta.docs[0].docstring, "This should be trimmed"); // "proper" blocking with trimming
            assert_eq!(meta.docs[1].docstring, "Some Text With Trailing Whitespace"); // inline w/ trimming
            assert_eq!(meta.docs[2].docstring, ""); // buffer should be empty
        }

        #[test]
        fn metadata_gui() {
            let data = String::from(
                "<c11r><metadata>
					<gui id=\"0\" pos=\"1.0;0.0\"></gui>
					<gui id=\"1\" pos=\"1;1\"></gui>
					<gui id=\"2\" pos=\"3.256;365.12\"/>
					<gui id=\"3\" pos=\"-454.6;-789.5\"/>
				</metadata></c11r>
				",
            );
            let meta = super::parse_string(data, false);

            println!("Test data found: {:?}", meta.gui);

            assert_eq!(meta.gui.len(), 4);

            // ids
            assert_eq!(meta.gui[0].id, 0);
            assert_eq!(meta.gui[1].id, 1);
            assert_eq!(meta.gui[2].id, 2);
            assert_eq!(meta.gui[3].id, 3);
            // pos
            // are f32s accurate enough to do equalities reliably???
            assert_eq!(meta.gui[0].position, (1f32, 0f32));
            assert_eq!(meta.gui[1].position, (1f32, 1f32));
            assert_eq!(meta.gui[2].position, (3.256f32, 365.12f32));
            assert_eq!(meta.gui[3].position, (-454.6f32, -789.5f32));
        }

        #[test]
        fn metadata_comment() {
            let data = String::from(
                "<c11r><metadata>
					<comment id=\"0\" pos=\"1.0;-1.0\">
						This is a comment
					</comment>
					<comment id=\"1\" pos=\"100;100\">  This is a comment  </comment>
					<comment id=\"2\" pos=\"3.1415;-1.2245\"/>
				</metadata></c11r>
				",
            );
            let meta = super::parse_string(data, false);

            println!("Test data found: {:?}", meta.comments);

            assert_eq!(meta.comments.len(), 3);

            // ids
            assert_eq!(meta.comments[0].id, 0);
            assert_eq!(meta.comments[1].id, 1);
            assert_eq!(meta.comments[2].id, 2);

            // pos
            assert_eq!(meta.comments[0].position, (1f32, -1f32));
            assert_eq!(meta.comments[1].position, (100f32, 100f32));
            assert_eq!(meta.comments[2].position, (3.1415f32, -1.2245f32));

            // text
            assert_eq!(meta.comments[0].text, "This is a comment");
            assert_eq!(meta.comments[1].text, "This is a comment");
            assert_eq!(meta.comments[2].text, "");
        }
    }
}
