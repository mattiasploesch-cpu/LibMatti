fn main() {
    let parsed = launcher_desktop::parse_args(std::env::args().skip(1));
    let code = match parsed {
        Ok(launcher_desktop::Parsed::Help) => {
            print!("{}", launcher_desktop::help_text());
            0
        }
        Ok(launcher_desktop::Parsed::Version) => {
            println!("matticraft-launcher {}", env!("CARGO_PKG_VERSION"));
            0
        }
        Ok(launcher_desktop::Parsed::Run(options)) => launcher_desktop::run(options),
        Err(error) => {
            eprintln!("matticraft-launcher: {error}");
            2
        }
    };
    std::process::exit(code);
}
