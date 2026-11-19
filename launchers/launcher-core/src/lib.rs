//! Matticraft launcher core.
//!
//! One implementation of profiles, accounts, versions, mods, logs and update
//! verification for every frontend (desktop, macOS bundle and Android). The
//! frontends only render this data and call [`launch::LaunchPlan`]; the
//! `profile.json` format is the same on every platform.
//!
//! The core never opens a window and never contains UI code, so it can be
//! tested headlessly on the CI hosts.

pub mod account;
pub mod launch;
pub mod logs;
pub mod mods;
pub mod platform;
pub mod profile;
pub mod store;
pub mod update;

pub use account::{Account, AccountKind};
pub use launch::{LaunchPlan, LaunchStatus};
pub use profile::{parse_selection, Profile, PRESETS};
pub use store::{Settings, Store};

use std::fmt;

/// What the launcher reports when an operation cannot be completed.
#[derive(Debug)]
pub enum Error {
    Io(std::io::Error),
    Json(serde_json::Error),
    Invalid(String),
    NotFound(String),
    Signature(String),
    Unsupported(String),
}

impl fmt::Display for Error {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Error::Io(error) => write!(f, "I/O error: {error}"),
            Error::Json(error) => write!(f, "JSON error: {error}"),
            Error::Invalid(message) => write!(f, "{message}"),
            Error::NotFound(message) => write!(f, "not found: {message}"),
            Error::Signature(message) => write!(f, "signature error: {message}"),
            Error::Unsupported(message) => write!(f, "unsupported: {message}"),
        }
    }
}

impl std::error::Error for Error {
    fn source(&self) -> Option<&(dyn std::error::Error + 'static)> {
        match self {
            Error::Io(error) => Some(error),
            Error::Json(error) => Some(error),
            _ => None,
        }
    }
}

impl From<std::io::Error> for Error {
    fn from(error: std::io::Error) -> Self {
        Error::Io(error)
    }
}

impl From<serde_json::Error> for Error {
    fn from(error: serde_json::Error) -> Self {
        Error::Json(error)
    }
}

pub type Result<T> = std::result::Result<T, Error>;

/// The current time as an RFC 3339 string (UTC).
pub fn now_rfc3339() -> String {
    time::OffsetDateTime::now_utc()
        .format(&time::format_description::well_known::Rfc3339)
        .unwrap_or_else(|_| "1970-01-01T00:00:00Z".to_string())
}

/// A stable, filesystem-safe slug for a profile or instance name.
pub fn slugify(name: &str) -> String {
    let mut out = String::with_capacity(name.len());
    let mut last_dash = false;
    for ch in name.chars() {
        let mapped = if ch.is_ascii_alphanumeric() {
            Some(ch.to_ascii_lowercase())
        } else if ch == '-' || ch == '_' || ch == '.' || ch == '/' || ch == '\\' || ch.is_whitespace()
        {
            Some('-')
        } else {
            None
        };
        if let Some(ch) = mapped {
            if ch == '-' {
                if !last_dash && !out.is_empty() {
                    out.push('-');
                    last_dash = true;
                }
            } else {
                out.push(ch);
                last_dash = false;
            }
        }
    }
    while out.ends_with('-') {
        out.pop();
    }
    if out.is_empty() {
        "default".to_string()
    } else {
        out
    }
}

/// Write JSON through a temporary file so a crash cannot leave a half file.
pub(crate) fn write_json_atomic<T: serde::Serialize>(
    path: &std::path::Path,
    value: &T,
) -> Result<()> {
    if let Some(parent) = path.parent() {
        std::fs::create_dir_all(parent)?;
    }
    let tmp = path.with_extension("json.tmp");
    let bytes = serde_json::to_vec_pretty(value)?;
    std::fs::write(&tmp, bytes)?;
    if path.exists() {


        std::fs::remove_file(path)?;
    }
    std::fs::rename(&tmp, path)?;
    Ok(())
}

#[cfg(test)]
pub(crate) fn temp_dir(tag: &str) -> std::path::PathBuf {
    use std::sync::atomic::{AtomicU64, Ordering};
    static COUNTER: AtomicU64 = AtomicU64::new(0);
    let count = COUNTER.fetch_add(1, Ordering::Relaxed);
    let path = std::env::temp_dir().join(format!(
        "matticraft-launcher-{tag}-{}-{count}",
        std::process::id()
    ));
    let _ = std::fs::remove_dir_all(&path);
    std::fs::create_dir_all(&path).expect("create test directory");
    path
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn slug_keeps_safe_characters_only() {
        assert_eq!(slugify("My World 1.21"), "my-world-1-21");
        assert_eq!(slugify("../../etc/passwd"), "etc-passwd");
        assert_eq!(slugify("   "), "default");
        assert_eq!(slugify("Ünïcode"), "ncode");
    }

    #[test]
    fn json_write_replaces_existing_file() {
        let dir = temp_dir("json");
        let path = dir.join("profile.json");
        write_json_atomic(&path, &serde_json::json!({"v": 1})).unwrap();
        write_json_atomic(&path, &serde_json::json!({"v": 2})).unwrap();
        let value: serde_json::Value = serde_json::from_slice(&std::fs::read(path).unwrap()).unwrap();
        assert_eq!(value["v"], 2);
        let _ = std::fs::remove_dir_all(dir);
    }
}
