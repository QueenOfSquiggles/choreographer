use std::{
    fs::{File, OpenOptions},
    io::Write,
    path::PathBuf,
};

use chrono::Local;
use log::{debug, error, info, trace, warn};

#[derive(Debug, Clone)]
pub struct Logger {
    file: PathBuf,
}

impl Logger {
    pub fn new(file: PathBuf) -> Self {
        if let Ok(mut fout) = File::create(file.clone()) {
            // purges old data from last logging session
            if let Err(e) = fout.write("".as_bytes()) {
                eprintln!("Failed to clear previous file {} :: {}", file.display(), e);
            }
        }
        Self { file }
    }

    pub fn error(&self, msg: impl Into<String>) {
        let s: String = msg.into();
        self.emit_to_file(&format!("[E] {:} {s}\n", Local::now()));
        error!("{s}");
    }
    pub fn warn(&self, msg: impl Into<String>) {
        let s: String = msg.into();
        self.emit_to_file(&format!("[W] {:} {s}\n", Local::now()));
        warn!("{s}");
    }
    pub fn info(&self, msg: impl Into<String>) {
        let s: String = msg.into();
        self.emit_to_file(&format!("[*] {:} {s}\n", Local::now()));
        info!("{s}");
    }
    pub fn debug(&self, msg: impl Into<String>) {
        let s: String = msg.into();
        self.emit_to_file(&format!("[D] {:} {s}\n", Local::now()));
        debug!("{s}");
    }
    pub fn trace(&self, msg: impl Into<String>) {
        let s: String = msg.into();
        self.emit_to_file(&format!("[T] {:} {s}\n", Local::now()));
        trace!("{s}");
    }

    fn emit_to_file(&self, msg: &String) {
        let Ok(mut file) = OpenOptions::new()
            .write(true)
            .append(true)
            .create(true)
            .open(self.file.clone())
        else {
            return;
        };
        match file.write(msg.as_bytes()) {
            Ok(_) => (),
            Err(e) => eprintln!("failed to write to log file: {}", e),
        }
    }
}
