use cho_lib::{
    construct_environment,
    types::{GlobalName, NamespacedType},
};
use clap::Parser;
use exec::Execution;

mod exec;

#[derive(Parser)]
#[command(version, about, long_about=None)]
struct CliData {
    entry: Option<String>,

    #[arg(short, long)]
    dump_env: bool,
}

const FLAG_DUMP_ENV: &'static str = "CHO_DUMP_ENV";

fn main() {
    let cli = CliData::parse();
    let mut env = construct_environment();
    if cli.dump_env && !env.flags.contains(&FLAG_DUMP_ENV.into()) {
        env.flags.push(FLAG_DUMP_ENV.into());
    }
    let entry = GlobalName::from_path(cli.entry.unwrap_or("".into()));
    let mut exe = Execution::new(env.clone(), entry);
    let output = exe.run();
    if env.flags.contains(&FLAG_DUMP_ENV.into()) {
        env.logger.info("=== DUMPING ENVIRONMENT ===");
        env.logger.info(format!("{:?}", env));
        env.logger.info("=== END DUMP ===");
    }
    match output {
        Ok(last_frame) => {
            if !last_frame.is_empty() {
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
