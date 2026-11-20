//! Downloadable game versions.
//!
//! The launcher package contains no game binary. This module reads the GitHub
//! release list, filters the portable artifacts for the current platform and
//! installs a selected version into the launcher data directory under
//! `games/<version>/`. The downloaded tree is kept intact because the game
//! needs its shared library, assets and font next to the executable.

use std::path::{Path, PathBuf};
#[cfg(feature = "upload")]
use std::process::Command;

#[cfg(feature = "upload")]
use serde_json::Value;

use crate::{Error, Result};

/// Repository used when the launcher was built without an explicit override.
pub fn github_repo() -> String {
    option_env!("MATTI_GITHUB_REPO")
        .unwrap_or("mattiasploesch-cpu/LibMatti")
        .trim()
        .trim_matches('/')
        .to_string()
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct GameRelease {
    pub version: String,
    pub tag: String,
    pub name: String,
    pub url: String,
    pub size: u64,
    /// SHA-256 from the GitHub asset digest (`sha256:...`), when available.
    pub digest: Option<String>,
}

#[cfg(feature = "upload")]
pub fn available(repo: &str, platform: &str, arch: &str) -> Result<Vec<GameRelease>> {
    let url = format!("https://api.github.com/repos/{repo}/releases?per_page=30");
    let bytes = crate::update::fetch_bytes(&url)?;
    let releases: Value = serde_json::from_slice(&bytes)?;
    let Some(items) = releases.as_array() else {
        return Err(Error::Invalid("GitHub releases response is not a list".into()));
    };

    let mut result = Vec::new();
    for item in items {
        if item.get("draft").and_then(Value::as_bool).unwrap_or(false) {
            continue;
        }
        let tag = item.get("tag_name").and_then(Value::as_str).unwrap_or("");
        let version = tag.trim_start_matches('v');
        if version.is_empty() {
            continue;
        }
        let Some(assets) = item.get("assets").and_then(Value::as_array) else {
            continue;
        };
        for asset in assets {
            let name = asset.get("name").and_then(Value::as_str).unwrap_or("");
            if !is_game_artifact(name, platform, arch) {
                continue;
            }
            let Some(url) = asset
                .get("browser_download_url")
                .and_then(Value::as_str)
            else {
                continue;
            };
            let digest = asset
                .get("digest")
                .and_then(Value::as_str)
                .and_then(|value| value.strip_prefix("sha256:"))
                .map(ToString::to_string);
            result.push(GameRelease {
                version: version.to_string(),
                tag: tag.to_string(),
                name: name.to_string(),
                url: url.to_string(),
                size: asset.get("size").and_then(Value::as_u64).unwrap_or(0),
                digest,
            });
            break;
        }
    }
    Ok(result)
}

#[cfg(not(feature = "upload"))]
pub fn available(_repo: &str, _platform: &str, _arch: &str) -> Result<Vec<GameRelease>> {
    Err(Error::Unsupported(
        "this launcher was built without network support".into(),
    ))
}

#[cfg(feature = "upload")]
fn is_game_artifact(name: &str, platform: &str, arch: &str) -> bool {
    if !name.starts_with("Matticraft-") || name.contains("Launcher") {
        return false;
    }
    if !name.contains(&format!("-{platform}-")) {
        return false;
    }
    let linux_arch = if arch == "arm64" { "aarch64" } else { arch };
    match platform {
        "linux" => {
            name.ends_with(&format!("-linux-{linux_arch}.tar.zst"))
        }
        "windows" => {
            (arch == "x86_64" && name.contains("-windows-x64."))
                || (arch == "x86" && name.contains("-windows-x86."))
                || (arch == "arm64" && name.contains("-windows-arm64."))
        }
        "macos" => name.contains(&format!("-macos-{arch}.")),
        _ => false,
    }
}

/// Download and unpack a release below `root/games/<version>`.
#[cfg(feature = "upload")]
pub fn install(release: &GameRelease, root: &Path) -> Result<PathBuf> {
    let target = root
        .join("games")
        .join(crate::slugify(&release.version));
    let archive = target.join(&release.name);
    std::fs::create_dir_all(&target)?;
    if !archive.is_file() {
        crate::update::download_to_file(&release.url, &archive)?;
    }
    if let Some(digest) = &release.digest {
        crate::update::verify_file_sha256(&archive, digest, "game archive")?;
    }

    let extracted = target.join("root");
    let _ = std::fs::remove_dir_all(&extracted);
    std::fs::create_dir_all(&extracted)?;
    extract(&archive, &extracted)?;

    let binary = find_binary(&extracted).ok_or_else(|| {
        Error::NotFound(format!(
            "game executable in {}",
            extracted.display()
        ))
    })?;
    Ok(binary)
}

#[cfg(not(feature = "upload"))]
pub fn install(_release: &GameRelease, _root: &Path) -> Result<PathBuf> {
    Err(Error::Unsupported(
        "this launcher was built without network support".into(),
    ))
}

#[cfg(feature = "upload")]
fn extract(archive: &Path, target: &Path) -> Result<()> {
    if archive.extension().and_then(|value| value.to_str()) == Some("zst") {
        let status = Command::new("tar")
            .arg("--zstd")
            .arg("-xf")
            .arg(archive)
            .arg("-C")
            .arg(target)
            .status();
        if status.map(|status| status.success()).unwrap_or(false) {
            return Ok(());
        }
        let zstd = Command::new("zstd")
            .arg("-dc")
            .arg(archive)
            .stdout(std::process::Stdio::piped())
            .spawn()
            .map_err(Error::Io)?;
        let mut tar = Command::new("tar")
            .arg("-xf")
            .arg("-")
            .arg("-C")
            .arg(target)
            .stdin(zstd.stdout.ok_or_else(|| Error::Invalid("zstd has no stdout".into()))?)
            .spawn()
            .map_err(Error::Io)?;
        return tar
            .wait()
            .map_err(Error::Io)
            .and_then(|status| {
                if status.success() {
                    Ok(())
                } else {
                    Err(Error::Invalid("could not unpack game tar.zst".into()))
                }
            });
    }

    let attempts: &[(&str, &[&str])] = if cfg!(target_os = "windows") {
        &[("tar", &["-xf"])]
    } else if cfg!(target_os = "macos") {
        &[("ditto", &["-x", "-k"]), ("unzip", &["-q"])]
    } else {
        &[("unzip", &["-q"]), ("tar", &["-xf"])]
    };
    for (program, args) in attempts {
        let mut command = Command::new(program);
        for arg in *args {
            command.arg(arg);
        }
        command.arg(archive);
        if *program == "ditto" {
            command.arg(target);
        } else if *program == "unzip" {
            command.arg("-d").arg(target);
        } else {
            command.arg("-C").arg(target);
        }
        if command.status().map(|status| status.success()).unwrap_or(false) {
            return Ok(());
        }
    }
    Err(Error::Invalid(format!(
        "could not unpack game archive {}",
        archive.display()
    )))
}

#[cfg(all(test, feature = "upload"))]
mod tests {
    use super::is_game_artifact;

    #[test]
    fn selects_game_archives_but_not_launcher_archives() {
        assert!(is_game_artifact(
            "Matticraft-1.2.0-linux-x86_64.tar.zst",
            "linux",
            "x86_64"
        ));
        assert!(is_game_artifact(
            "Matticraft-1.2.0-linux-aarch64.tar.zst",
            "linux",
            "arm64"
        ));
        assert!(is_game_artifact(
            "Matticraft-1.2.0-windows-x64.zip",
            "windows",
            "x86_64"
        ));
        assert!(!is_game_artifact(
            "Matticraft-Launcher-1.2.0-linux-x86_64.tar.zst",
            "linux",
            "x86_64"
        ));
        assert!(!is_game_artifact(
            "Matticraft-1.2.0-linux-x86_64.tar.zst",
            "windows",
            "x86_64"
        ));
    }
}

#[cfg(feature = "upload")]
fn find_binary(root: &Path) -> Option<PathBuf> {
    let names = if cfg!(target_os = "windows") {
        ["matticraft.exe", "Matticraft.exe", "runClient.exe", "vulkancraft.exe"]
    } else {
        ["matticraft", "Matticraft", "runClient", "vulkancraft"]
    };
    for relative in ["bin", "usr/bin", "Matticraft/bin", "Contents/MacOS"] {
        for name in names {
            let path = root.join(relative).join(name);
            if path.is_file() {
                return Some(path);
            }
        }
    }
    // AppImage/zip layouts sometimes put the executable at the archive root.
    for name in names {
        let path = root.join(name);
        if path.is_file() {
            return Some(path);
        }
    }
    None
}
