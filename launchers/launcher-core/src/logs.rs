//! Local logs and crash reports.
//!
//! Every profile writes `logs/latest.log`; a panic also writes
//! `crash-reports/crash-<timestamp>.txt`. Upload is strictly opt-in: the core
//! only prepares and (with the `upload` feature) sends a report when the
//! settings explicitly allow it.

use std::io::Write;
use std::path::{Path, PathBuf};

use crate::Result;
#[cfg(feature = "upload")]
use crate::Error;

pub const LOG_FILE: &str = "latest.log";
pub const CRASH_DIR: &str = "crash-reports";

#[derive(Clone, Debug)]
pub struct CrashContext {
    pub launcher_version: String,
    pub game_version: String,
    pub profile: String,
    pub selection: String,
    pub platform: String,
    pub arch: String,
}

impl Default for CrashContext {
    fn default() -> Self {
        CrashContext {
            launcher_version: env!("CARGO_PKG_VERSION").to_string(),
            game_version: String::new(),
            profile: String::new(),
            selection: String::new(),
            platform: crate::platform::Platform::current().name().to_string(),
            arch: crate::platform::arch().to_string(),
        }
    }
}

#[derive(Clone, Debug)]
pub struct CrashInfo {
    pub message: String,
    pub backtrace: Option<String>,
    pub context: CrashContext,
}

impl CrashInfo {
    pub fn capture(message: impl Into<String>, context: CrashContext) -> Self {
        CrashInfo {
            message: message.into(),
            backtrace: Some(std::backtrace::Backtrace::force_capture().to_string()),
            context,
        }
    }
}

/// Append-only launcher log.
pub struct LogFile {
    path: PathBuf,
    file: std::fs::File,
}

impl LogFile {
    pub fn create(logs_dir: &Path) -> Result<Self> {
        std::fs::create_dir_all(logs_dir)?;
        let path = logs_dir.join(LOG_FILE);
        let file = std::fs::OpenOptions::new()
            .create(true)
            .truncate(true)
            .write(true)
            .open(&path)?;
        let mut log = LogFile { path, file };
        log.line(&format!(
            "Matticraft launcher {} on {} {}",
            env!("CARGO_PKG_VERSION"),
            crate::platform::Platform::current().name(),
            crate::platform::arch()
        ))?;
        Ok(log)
    }

    pub fn open_append(logs_dir: &Path) -> Result<Self> {
        std::fs::create_dir_all(logs_dir)?;
        let path = logs_dir.join(LOG_FILE);
        let file = std::fs::OpenOptions::new()
            .create(true)
            .append(true)
            .open(&path)?;
        Ok(LogFile { path, file })
    }

    pub fn path(&self) -> &Path {
        &self.path
    }

    pub fn line(&mut self, message: &str) -> Result<()> {
        writeln!(self.file, "[{}] {}", crate::now_rfc3339(), message)?;
        self.file.flush()?;
        Ok(())
    }

    pub fn error(&mut self, message: &str) -> Result<()> {
        self.line(&format!("ERROR {message}"))
    }
}

/// Write a crash report and return its path.
pub fn write_crash_report(crash_dir: &Path, info: &CrashInfo) -> Result<PathBuf> {
    std::fs::create_dir_all(crash_dir)?;
    let stamp = crate::now_rfc3339()
        .replace(':', "-")
        .replace(['.', '+'], "-");
    let path = crash_dir.join(format!("crash-{stamp}.txt"));
    let mut file = std::fs::File::create(&path)?;
    writeln!(file, "Matticraft crash report")?;
    writeln!(file, "time: {}", crate::now_rfc3339())?;
    writeln!(file, "launcher: {}", info.context.launcher_version)?;
    writeln!(file, "game: {}", info.context.game_version)?;
    writeln!(file, "profile: {}", info.context.profile)?;
    writeln!(file, "selection: {}", info.context.selection)?;
    writeln!(file, "platform: {} {}", info.context.platform, info.context.arch)?;
    writeln!(file, "\nmessage:\n{}", info.message)?;
    if let Some(backtrace) = &info.backtrace {
        writeln!(file, "\nbacktrace:\n{backtrace}")?;
    }
    file.flush()?;
    Ok(path)
}

/// Install a panic hook that writes a crash report before the default hook
/// prints the panic. The report stays local unless the caller uploads it.
pub fn install_panic_hook(crash_dir: PathBuf, context: CrashContext) {
    let default_hook = std::panic::take_hook();
    std::panic::set_hook(Box::new(move |panic| {
        let message = if let Some(payload) = panic.payload().downcast_ref::<&str>() {
            (*payload).to_string()
        } else if let Some(payload) = panic.payload().downcast_ref::<String>() {
            payload.clone()
        } else {
            "non-string panic payload".to_string()
        };
        let location = panic
            .location()
            .map(|location| format!("{}:{}", location.file(), location.line()))
            .unwrap_or_else(|| "unknown location".to_string());
        let info = CrashInfo {
            message: format!("{message} at {location}"),
            backtrace: Some(std::backtrace::Backtrace::force_capture().to_string()),
            context: context.clone(),
        };
        match write_crash_report(&crash_dir, &info) {
            Ok(path) => eprintln!("matticraft: crash report written to {}", path.display()),
            Err(error) => eprintln!("matticraft: could not write crash report: {error}"),
        }
        default_hook(panic);
    }));
}

pub fn crash_reports_dir(profile_root: &Path) -> PathBuf {
    profile_root.join(CRASH_DIR)
}

pub fn list_crash_reports(crash_dir: &Path) -> Result<Vec<PathBuf>> {
    if !crash_dir.exists() {
        return Ok(Vec::new());
    }
    let mut reports = Vec::new();
    for entry in std::fs::read_dir(crash_dir)? {
        let entry = entry?;
        if entry.file_type()?.is_file()
            && entry
                .path()
                .extension()
                .and_then(|extension| extension.to_str())
                == Some("txt")
        {
            reports.push(entry.path());
        }
    }
    reports.sort();
    reports.reverse();
    Ok(reports)
}

/// Read at most `max_lines` from the end of a log/report.
pub fn tail(path: &Path, max_lines: usize) -> Result<Vec<String>> {
    let text = std::fs::read_to_string(path)?;
    let mut lines: Vec<String> = text.lines().map(ToString::to_string).collect();
    if lines.len() > max_lines {
        lines.drain(0..lines.len() - max_lines);
    }
    Ok(lines)
}

#[cfg(feature = "upload")]
#[derive(Clone, Debug, PartialEq, Eq)]
pub enum UploadOutcome {
    /// Consent was not given.
    Disabled,
    Uploaded(u16),
}

/// Upload one crash report. Callers must pass the explicit opt-in flag from
/// the settings; there is no implicit upload path.
#[cfg(feature = "upload")]
pub fn upload_crash_report(
    report: &Path,
    endpoint: &str,
    consent: bool,
) -> Result<UploadOutcome> {
    if !consent {
        return Ok(UploadOutcome::Disabled);
    }
    let body = std::fs::read(report)?;
    let response = ureq::post(endpoint)
        .timeout(std::time::Duration::from_secs(20))
        .set("Content-Type", "text/plain; charset=utf-8")
        .set("X-Matticraft-Report", "crash")
        .send_bytes(&body);
    match response {
        Ok(response) if response.status() < 300 => Ok(UploadOutcome::Uploaded(response.status())),
        Ok(response) => Err(Error::Invalid(format!(
            "crash upload rejected with HTTP {}",
            response.status()
        ))),
        Err(error) => Err(Error::Invalid(format!("crash upload failed: {error}"))),
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn log_is_written_and_tailable() {
        let dir = crate::temp_dir("logs");
        let mut log = LogFile::create(&dir).unwrap();
        log.line("first").unwrap();
        log.error("second").unwrap();
        let lines = tail(log.path(), 2).unwrap();
        assert_eq!(lines.len(), 2);
        assert!(lines[1].contains("ERROR second"));
        let _ = std::fs::remove_dir_all(dir);
    }

    #[test]
    fn crash_report_contains_context() {
        let dir = crate::temp_dir("crash");
        let info = CrashInfo {
            message: "boom".into(),
            backtrace: Some("frame".into()),
            context: CrashContext {
                profile: "Default".into(),
                selection: "sdl:auto".into(),
                ..CrashContext::default()
            },
        };
        let path = write_crash_report(&dir, &info).unwrap();
        let text = std::fs::read_to_string(&path).unwrap();
        assert!(text.contains("profile: Default"));
        assert!(text.contains("selection: sdl:auto"));
        assert!(text.contains("boom"));
        assert_eq!(list_crash_reports(&dir).unwrap(), vec![path]);
        let _ = std::fs::remove_dir_all(dir);
    }
}
