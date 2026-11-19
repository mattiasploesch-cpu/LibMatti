//! Desktop launcher (Linux/Windows) and the shared entry point for the macOS
//! bundle frontend.
//!
//! `--check` and `--launch` are headless and used by CI/package smoke tests;
//! the default path opens the egui window.

pub mod app;

use std::path::PathBuf;

use launcher_core::launch::{find_game_binary, LaunchPlan, LaunchStatus};
use launcher_core::logs::{self, CrashContext};
use launcher_core::mods;
use launcher_core::platform;
use launcher_core::store::Store;
use launcher_core::{Error, Result};

#[derive(Clone, Debug)]
pub struct LauncherOptions {
    pub root: Option<PathBuf>,
    pub profile: Option<String>,
    pub game: Option<PathBuf>,
    pub check: bool,
    pub headless_launch: bool,
    pub one_frame: bool,
    pub frames: Option<u64>,
}

impl Default for LauncherOptions {
    fn default() -> Self {
        LauncherOptions {
            root: None,
            profile: None,
            game: None,
            check: false,
            headless_launch: false,
            one_frame: false,
            frames: None,
        }
    }
}

pub enum Parsed {
    Help,
    Version,
    Run(LauncherOptions),
}

pub fn parse_args(args: impl IntoIterator<Item = String>) -> std::result::Result<Parsed, String> {
    let mut options = LauncherOptions::default();
    let mut args = args.into_iter();
    while let Some(argument) = args.next() {
        let (name, inline) = match argument.split_once('=') {
            Some((name, value)) => (name.to_string(), Some(value.to_string())),
            None => (argument, None),
        };
        let mut take = |name: &str| -> std::result::Result<String, String> {
            inline
                .clone()
                .or_else(|| args.next())
                .ok_or_else(|| format!("{name} needs a value"))
        };
        match name.as_str() {
            "--help" | "-h" => return Ok(Parsed::Help),
            "--version" | "-V" => return Ok(Parsed::Version),
            "--check" => options.check = true,
            "--launch" => options.headless_launch = true,
            "--ui-once" => options.one_frame = true,
            "--root" => options.root = Some(PathBuf::from(take("--root")?)),
            "--profile" => options.profile = Some(take("--profile")?),
            "--game" => options.game = Some(PathBuf::from(take("--game")?)),
            "--frames" => {
                let value = take("--frames")?;
                options.frames = Some(
                    value
                        .parse()
                        .map_err(|_| format!("--frames expects a number, got '{value}'"))?,
                );
            }
            other => return Err(format!("unknown option '{other}' (try --help)")),
        }
    }
    Ok(Parsed::Run(options))
}

pub fn help_text() -> &'static str {
    "Matticraft launcher\n\
     \n\
     Usage: matticraft-launcher [options]\n\
     \n\
       --check              print the resolved store/profile/mods and exit\n\
       --launch             launch the selected profile without the UI\n\
       --profile <name>     select a profile by id or display name\n\
       --game <path>        use this game binary\n\
       --root <path>        launcher data directory (default: platform data dir)\n\
       --ui-once            draw one frame and close (GUI smoke test)\n\
       --frames <n>         close the GUI after n frames\n\
       --version            print the launcher version\n\
       --help               this text\n"
}

pub fn run(options: LauncherOptions) -> i32 {
    if options.check {
        return match run_check(&options) {
            Ok(()) => 0,
            Err(error) => {
                eprintln!("matticraft-launcher: {error}");
                2
            }
        };
    }
    if options.headless_launch {
        return match run_headless(&options) {
            Ok(code) => code,
            Err(error) => {
                eprintln!("matticraft-launcher: {error}");
                2
            }
        };
    }
    match app::run_native(options) {
        Ok(()) => 0,
        Err(error) => {
            eprintln!("matticraft-launcher: {error}");
            2
        }
    }
}

pub fn open_store(options: &LauncherOptions) -> Result<Store> {
    let root = match &options.root {
        Some(root) => root.clone(),
        None => Store::default_root()?,
    };
    let mut store = Store::load_or_create(root)?;
    if let Some(game) = &options.game {
        store.settings.game_binary = Some(game.clone());
    }
    if let Some(profile) = &options.profile {
        let id = store
            .profiles
            .iter()
            .find(|candidate| candidate.id == *profile || candidate.name == *profile)
            .map(|candidate| candidate.id.clone())
            .ok_or_else(|| Error::NotFound(format!("profile '{profile}'")))?;
        store.set_active(&id)?;
    }
    Ok(store)
}

fn run_check(options: &LauncherOptions) -> Result<()> {
    let store = open_store(options)?;
    println!(
        "launcher {} ({} {})",
        env!("CARGO_PKG_VERSION"),
        platform::Platform::current().name(),
        platform::arch()
    );
    println!("store: {}", store.root().display());
    println!(
        "default selection: {} ({})",
        platform::default_selection(),
        platform::default_selection_source()
    );
    println!(
        "profiles: {} (active: {})",
        store.profiles.len(),
        store.active.as_deref().unwrap_or("-")
    );
    for profile in &store.profiles {
        println!(
            "  {}: {} {} {} {}",
            profile.id,
            profile.name,
            profile.game_version,
            profile.display_selection(),
            profile.instance_dir.display()
        );
    }
    println!("accounts: {}", store.accounts.len());
    match find_game_binary(&store.settings) {
        Ok(path) => println!("game binary: {}", path.display()),
        Err(error) => println!("game binary: missing ({error})"),
    }
    if let Some(profile) = store.active_profile() {
        let validation = mods::validate(&profile.mods_dir)?;
        println!("mods: {}", validation.summary());
        for missing in &validation.missing {
            println!("  missing: {missing}");
        }
        for changed in &validation.mismatched {
            println!("  changed: {}", changed.file);
        }
        for unlisted in &validation.unlisted {
            println!("  unlisted: {unlisted}");
        }
        let crash_dir = logs::crash_reports_dir(&profile.instance_dir);
        println!(
            "crash reports: {}",
            logs::list_crash_reports(&crash_dir)?.len()
        );
    }
    Ok(())
}

fn run_headless(options: &LauncherOptions) -> Result<i32> {
    let mut store = open_store(options)?;
    let profile = store
        .active_profile()
        .cloned()
        .ok_or_else(|| Error::NotFound("active profile".into()))?;
    profile.validate()?;
    store.save()?;
    let account = store.active_account().cloned();
    let plan = LaunchPlan::prepare(&profile, &store.settings, account.as_ref(), &[])?;
    println!("{}", plan.display_command());
    match plan.run()? {
        LaunchStatus::Exited { code, .. } => Ok(code.unwrap_or(0)),
        LaunchStatus::Failed { message, .. } => Err(Error::Invalid(message)),
    }
}

pub fn crash_context(_store: &Store, profile: Option<&launcher_core::Profile>) -> CrashContext {
    CrashContext {
        launcher_version: env!("CARGO_PKG_VERSION").to_string(),
        game_version: profile
            .map(|profile| profile.game_version.clone())
            .unwrap_or_default(),
        profile: profile.map(|profile| profile.name.clone()).unwrap_or_default(),
        selection: profile
            .map(|profile| profile.display_selection())
            .unwrap_or_else(|| platform::default_selection()),
        platform: platform::Platform::current().name().to_string(),
        arch: platform::arch().to_string(),
    }
}
