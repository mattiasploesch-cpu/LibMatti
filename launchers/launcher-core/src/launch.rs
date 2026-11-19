//! Turning a profile into a game process.
//!
//! The launcher passes the selection as `--layer`/`--api` (and as the same
//! `MATTI_LAYER`/`MATTI_API` environment variables for tools that cannot read
//! argv), keeps the game directory at the profile instance and appends the
//! game's output to `logs/latest.log`.

use std::path::PathBuf;
use std::process::{Child, Command, Stdio};

use crate::account::Account;
use crate::profile::Profile;
use crate::store::Settings;
use crate::{Error, Result};

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct LaunchPlan {
    pub command: PathBuf,
    pub args: Vec<String>,
    pub cwd: PathBuf,
    pub log_path: PathBuf,
    pub env: Vec<(String, String)>,
}

impl LaunchPlan {
    pub fn prepare(
        profile: &Profile,
        settings: &Settings,
        account: Option<&Account>,
        extra_args: &[String],
    ) -> Result<Self> {
        profile.validate()?;
        profile.ensure_dirs()?;
        preflight(profile)?;
        let (layer, api) = profile.selection_parts()?;
        let command = find_game_binary(settings)?;

        let mut args = vec![
            "--no-tests".to_string(),
            "--gameDir".to_string(),
            profile.game_dir.display().to_string(),
            "--version".to_string(),
            profile.game_version.clone(),
        ];
        if layer != "auto" {
            args.push("--layer".into());
            args.push(layer.clone());
        }
        if api != "auto" {
            args.push("--api".into());
            args.push(api.clone());
        }
        args.push("--width".into());
        args.push(profile.width.to_string());
        args.push("--height".into());
        args.push(profile.height.to_string());
        if profile.fullscreen {
            args.push("--fullscreen".into());
        }
        if let Some(account) = account {
            args.extend(account.game_arguments());
        }
        args.extend(profile.extra_args.iter().cloned());
        args.extend(extra_args.iter().cloned());

        let selection = format!("{layer}:{api}");
        let mut env = vec![
            ("MATTI_LAYER".to_string(), layer),
            ("MATTI_API".to_string(), api),
            ("MATTI_RENDERER".to_string(), selection),
            ("MATTI_PROFILE".to_string(), profile.id.clone()),
            (
                "MATTI_GAME_VERSION".to_string(),
                profile.game_version.clone(),
            ),
        ];
        env.extend(resource_env());

        Ok(LaunchPlan {
            command,
            args,
            cwd: profile.game_dir.clone(),
            log_path: profile.logs_dir.join(crate::logs::LOG_FILE),
            env,
        })
    }

    pub fn display_command(&self) -> String {
        let mut text = self.command.display().to_string();
        for arg in &self.args {
            text.push(' ');
            if arg.contains(' ') {
                text.push('"');
                text.push_str(arg);
                text.push('"');
            } else {
                text.push_str(arg);
            }
        }
        text
    }

    pub fn spawn(&self) -> Result<Child> {
        std::fs::create_dir_all(&self.cwd)?;
        if let Some(parent) = self.log_path.parent() {
            std::fs::create_dir_all(parent)?;
        }
        let mut log = std::fs::OpenOptions::new()
            .create(true)
            .append(true)
            .open(&self.log_path)?;
        use std::io::Write;
        writeln!(
            log,
            "\n[{}] launch: {}",
            crate::now_rfc3339(),
            self.display_command()
        )?;
        let stderr = log.try_clone()?;

        let mut command = Command::new(&self.command);
        command
            .args(&self.args)
            .current_dir(&self.cwd)
            .stdin(Stdio::null())
            .stdout(Stdio::from(log))
            .stderr(Stdio::from(stderr));
        for (key, value) in &self.env {
            command.env(key, value);
        }
        command.spawn().map_err(|error| {
            Error::Invalid(format!(
                "could not start {}: {error}",
                self.command.display()
            ))
        })
    }

    /// Start and wait for the process (headless/CI path).
    pub fn run(&self) -> Result<LaunchStatus> {
        let mut child = self.spawn()?;
        let pid = child.id();
        match child.wait() {
            Ok(status) => Ok(LaunchStatus::Exited {
                pid,
                code: status.code(),
            }),
            Err(error) => Ok(LaunchStatus::Failed {
                pid,
                message: error.to_string(),
            }),
        }
    }
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub enum LaunchStatus {
    Exited { pid: u32, code: Option<i32> },
    Failed { pid: u32, message: String },
}

/// Resource locations for installs that do not embed the GUI in the game
/// binary (the Windows/macOS packages). Existing user overrides always win.
fn resource_env() -> Vec<(String, String)> {
    let mut env = Vec::new();
    if std::env::var_os("MATTI_UI_ASSETS").is_none() {
        if let Some(path) = find_resource("assets") {
            env.push(("MATTI_UI_ASSETS".to_string(), path.display().to_string()));
        }
    }
    if std::env::var_os("MATTI_THEME_FONT").is_none() {
        if let Some(path) = find_resource("Monocraft.ttf") {
            env.push(("MATTI_THEME_FONT".to_string(), path.display().to_string()));
        }
    }
    env
}

fn find_resource(name: &str) -> Option<PathBuf> {
    let exe = std::env::current_exe().ok()?;
    let dir = exe.parent()?;
    let mut candidates = vec![
        // Portable package: resources next to the launcher.
        dir.join(name),
        // Unix install prefix: <prefix>/bin/launcher -> <prefix>/share/matticraft.
        dir.join("..").join("share").join("matticraft").join(name),
        // macOS app bundle: Contents/MacOS/launcher -> Contents/Resources.
        dir.join("..").join("Resources").join(name),
    ];
    if name == "assets" {
        // Development tree: launchers/target/<profile>/launcher.
        candidates.push(
            dir.join("..")
                .join("..")
                .join("..")
                .join("library/src/libmatti/matti/ui/assets"),
        );
    } else {
        candidates.push(
            dir.join("..")
                .join("..")
                .join("..")
                .join("vendor/FancyModLoader/earlydisplay/src/main/resources/net/neoforged/fml/earlydisplay/theme/Monocraft.ttf"),
        );
    }
    candidates
        .into_iter()
        .find(|path| path.exists())
        .map(|path| std::fs::canonicalize(&path).unwrap_or(path))
}

pub fn find_game_binary(settings: &Settings) -> Result<PathBuf> {
    if let Some(path) = &settings.game_binary {
        if path.exists() {
            return Ok(path.clone());
        }
        return Err(Error::NotFound(format!(
            "configured game binary {}",
            path.display()
        )));
    }
    if let Some(path) = std::env::var_os("MATTI_GAME_BINARY") {
        let path = PathBuf::from(path);
        if path.exists() {
            return Ok(path);
        }
        return Err(Error::NotFound(format!(
            "MATTI_GAME_BINARY={}",
            path.display()
        )));
    }

    for candidate in local_candidates() {
        if candidate.is_file() {
            return Ok(candidate);
        }
    }
    for name in binary_names() {
        if let Some(path) = find_in_path(name) {
            return Ok(path);
        }
    }
    Err(Error::NotFound(
        "the game binary (matticraft/runClient); set it in the launcher settings or MATTI_GAME_BINARY"
            .into(),
    ))
}

fn binary_names() -> &'static [&'static str] {
    if cfg!(target_os = "windows") {
        &["matticraft.exe", "runClient.exe", "vulkancraft.exe"]
    } else {
        &["matticraft", "runClient", "vulkancraft"]
    }
}

fn local_candidates() -> Vec<PathBuf> {
    let mut candidates = Vec::new();
    if let Ok(exe) = std::env::current_exe() {
        if let Some(dir) = exe.parent() {
            for name in binary_names() {
                candidates.push(dir.join(name));
                candidates.push(dir.join("..").join(name));
            }
            // Development tree: launchers/target/<profile>/launcher ->
            // <repo>/build/runClient and <repo>/build/matticraft.
            candidates.push(dir.join("../../../build").join(binary_names()[0]));
            candidates.push(dir.join("../../../build").join(binary_names()[1]));
        }
    }
    candidates
}

pub fn find_in_path(name: &str) -> Option<PathBuf> {
    let path = std::env::var_os("PATH")?;
    std::env::split_paths(&path)
        .map(|directory| directory.join(name))
        .find(|candidate| candidate.is_file())
}

/// Check that the selection is usable on this host before spawning the game.
pub fn preflight(profile: &Profile) -> Result<()> {
    let (layer, api) = profile.selection_parts()?;
    let platform = crate::platform::Platform::current();
    match api.as_str() {
        "directx" if layer != "sdl" => Err(Error::Invalid(
            "DirectX is only available through the SDL layer (sdl:directx)".into(),
        )),
        "directx" if platform != crate::platform::Platform::Windows => Err(Error::Invalid(
            "DirectX is only available on Windows".into(),
        )),
        "metal" if layer != "sdl" => Err(Error::Invalid(
            "Metal is only available through the SDL layer (sdl:metal)".into(),
        )),
        "metal" if platform != crate::platform::Platform::MacOs => Err(Error::Invalid(
            "Metal is only available on macOS".into(),
        )),
        _ => Ok(()),
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::profile::Profile;
    use std::path::Path;

    fn profile(root: &Path) -> Profile {
        Profile::new(root, "Test", "1.21.11", "sdl:vulkan").unwrap()
    }

    #[test]
    fn plan_carries_the_selection_and_account() {
        let root = crate::temp_dir("launch-plan");
        let profile = profile(&root);
        let mut settings = Settings::default();
        settings.game_binary = Some(
            find_in_path("true").unwrap_or_else(|| PathBuf::from("/bin/true")),
        );
        let account = Account::offline("Steve").unwrap();
        let plan = LaunchPlan::prepare(&profile, &settings, Some(&account), &[]).unwrap();
        assert!(plan.args.contains(&"--layer".to_string()));
        assert!(plan.args.contains(&"sdl".to_string()));
        assert!(plan.args.contains(&"--api".to_string()));
        assert!(plan.args.contains(&"vulkan".to_string()));
        assert!(plan.args.contains(&"--username".to_string()));
        assert!(plan.args.contains(&"Steve".to_string()));
        assert!(plan.env.contains(&("MATTI_LAYER".into(), "sdl".into())));
        assert_eq!(plan.cwd, profile.game_dir);
        let _ = std::fs::remove_dir_all(root);
    }

    #[test]
    fn missing_binary_is_reported() {
        let root = crate::temp_dir("launch-missing");
        let profile = profile(&root);
        let mut settings = Settings::default();
        settings.game_binary = Some(root.join("does-not-exist"));
        assert!(LaunchPlan::prepare(&profile, &settings, None, &[]).is_err());
        let _ = std::fs::remove_dir_all(root);
    }

    #[cfg(unix)]
    #[test]
    fn spawn_redirects_output_to_the_log() {
        let root = crate::temp_dir("launch-spawn");
        let profile = profile(&root);
        let mut settings = Settings::default();
        settings.game_binary = Some(find_in_path("echo").unwrap_or_else(|| PathBuf::from("/bin/echo")));
        let plan = LaunchPlan::prepare(&profile, &settings, None, &["hello".into()]).unwrap();
        let status = plan.run().unwrap();
        assert!(matches!(status, LaunchStatus::Exited { code: Some(0), .. }));
        let text = std::fs::read_to_string(&plan.log_path).unwrap();
        assert!(text.contains("hello"));
        let _ = std::fs::remove_dir_all(root);
    }

    #[test]
    fn preflight_rejects_wrong_platform_apis() {
        let root = crate::temp_dir("launch-preflight");
        let mut profile = profile(&root);
        if !cfg!(target_os = "windows") {
            profile.set_selection("sdl:directx").unwrap();
            assert!(preflight(&profile).is_err());
        }
        profile.set_selection("native:vulkan").unwrap();
        assert!(preflight(&profile).is_ok());
        let _ = std::fs::remove_dir_all(root);
    }
}
