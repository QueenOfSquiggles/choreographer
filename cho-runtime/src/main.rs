use cho_lib::{
    construct_environment,
    types::{GlobalName, NamespacedType},
    Environment,
};
use clap::{Parser, Subcommand};
use exec::{ExecutableConfig, Execution};
use project::{ProjectFile, PROJECT_CONFIG_FILE};

mod exec;
mod project;

#[derive(Parser)]
#[command(version, about, long_about=None)]
struct CliData {
    #[command(subcommand)]
    command: Option<Commands>,
}

#[derive(Subcommand)]
enum Commands {
    Run {
        entry: Option<String>,

        #[arg(short, long)]
        dump_env: bool,
    },
    New {
        path: Option<String>,

        #[arg(short, long)]
        lib: bool,
    },
}

const FLAG_DUMP_ENV: &'static str = "CHO_DUMP_ENV";

fn main() {
    let cli = CliData::parse();
    let env = construct_environment();
    let Some(cmd) = cli.command else {
        env.logger
            .warn("No commands provided. Refer to the help page for available commands");
        return;
    };
    match cmd {
        Commands::Run { entry, dump_env } => cmd_run(env, entry, dump_env),
        Commands::New { path, lib } => cmd_new(env, path, lib),
    }
}

fn cmd_new(env: Environment, in_path: Option<String>, is_lib: bool) {
    let spath = in_path.clone().unwrap_or(".".into());
    let mut config = ProjectFile::default();
    match is_lib {
        true => (),
        false => {
            config.executable = Some(ExecutableConfig::default());
        }
    }

    ProjectFile::write_default_to(spath, &env.logger, config);
}

fn cmd_run(mut env: Environment, entry: Option<String>, dump_env: bool) {
    let Some(config) = ProjectFile::get_from_cwd(&env.logger) else {
        env.logger.error(format!("Failed to find configuration file at this directory. Make sure you have a {} file in this directory", PROJECT_CONFIG_FILE));
        return;
    };
    if dump_env && !env.flags.contains(&FLAG_DUMP_ENV.into()) {
        env.flags.push(FLAG_DUMP_ENV.into());
    }
    let entry = GlobalName::from_path(entry.unwrap_or("".into()));
    let mut exe = Execution::new(env.clone(), entry, &config);
    let output = exe.run();
    if env.flags.contains(&FLAG_DUMP_ENV.into()) {
        env.logger.info("=== DUMPING ENVIRONMENT ===");
        env.logger.info(format!("{:?}", env));
        env.logger.info("=== END DUMP ===");
    }
    match output {
        Ok(last_frame) => {
            if !last_frame.0.is_empty() {
                env.logger
                    .info("Exited successfully with dangling frame data");
                env.logger.info(format!("{:#?}", last_frame));
            }
        }
        Err((node_err, stack)) => {
            env.logger.error("Encountered error during execution");
            env.logger.error(format!("Error type: {:#?}", node_err));
            if stack.is_empty() {
                env.logger.error("=== CALL STACK (Empty) ===");
            } else {
                env.logger.error("=== CALL STACK ===");
                for frame in stack.iter().enumerate().rev() {
                    env.logger
                        .error(format!("{} - {:?}", frame.0, frame.1.get_name()));
                }
                env.logger.error("=== END STACK ===");
            }
        }
    }
}
