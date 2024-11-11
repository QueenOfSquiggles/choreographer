use std::{fs::File, path::Path};

use cho_lib::logger::Logger;
use ron::{extensions::Extensions, ser::PrettyConfig};
use serde::{Deserialize, Serialize};

use crate::exec::ExecutableConfig;

pub const PROJECT_CONFIG_FILE: &'static str = "choreo.ron";

#[derive(Serialize, Deserialize, Clone, Debug, Default)]
pub struct ProjectFile {
    pub meta: Metadata,
    pub executable: Option<ExecutableConfig>,
}

#[derive(Serialize, Deserialize, Clone, Debug)]
pub struct Metadata {
    name: String,
    version: String,
    authors: Vec<String>,
    description: String,
    source: String,
    license: String,
}

impl Default for Metadata {
    fn default() -> Self {
        Self {
            name: "project".into(),
            version: "0.1.0".into(),
            authors: vec!["your name here <youremail@provider>".into()],
            description: "a project".into(),
            source: "https://your.vcs/homepage".into(),
            license: "MIT + Apache 2.0".into(),
        }
    }
}

impl ProjectFile {
    pub fn write_default_to(spath: String, log: &Logger, instance: Self) {
        let path = Path::new(&spath);
        if !path.is_dir() {
            log.error(format!("Failed to write out {:?}", spath));
            return;
        }
        let file_path = path.join(PROJECT_CONFIG_FILE);

        let Ok(file) = File::create(file_path.clone()) else {
            log.error(format!("Failed to create project file at {:?}", spath));
            return;
        };
        match ron::ser::to_writer_pretty(file, &instance, PrettyConfig::default()) {
            Ok(_) => {
                log.info(format!("Wrote out project file to {}", file_path.display()));
            }
            Err(_) => {
                log.error(format!(
                    "Failed to write out default project data, {}",
                    file_path.display()
                ));
            }
        }
    }

    pub fn get_from_cwd(log: &Logger) -> Option<Self> {
        let Ok(file) = File::open(PROJECT_CONFIG_FILE) else {
            log.error("Failed to open project file");
            return None;
        };
        match ron::de::from_reader::<_, Self>(file) {
            Ok(val) => Some(val),
            Err(_) => None,
        }
    }
}
