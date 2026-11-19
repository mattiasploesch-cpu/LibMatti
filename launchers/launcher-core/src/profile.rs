//! The shared profile format (`profile.json`).
//!
//! A profile owns one instance directory with `mods/`, `logs/` and the game
//! directory. The renderer selection is the same `layer:api` string the game
//! understands; no launcher stores a build-time renderer.

use std::path::{Path, PathBuf};

use serde::{Deserialize, Serialize};

use crate::{slugify, Error, Result};

pub const PROFILE_FILE: &str = "profile.json";
pub const PROFILE_SCHEMA: u32 = 1;

pub const LAYERS: &[&str] = &["native", "sdl", "auto"];
pub const APIS: &[&str] = &["opengl", "vulkan", "directx", "metal", "auto"];

/// Every selection the options screen and the launcher offer.
pub const PRESETS: &[&str] = &[
    "native:opengl",
    "native:vulkan",
    "native:auto",
    "sdl:opengl",
    "sdl:vulkan",
    "sdl:directx",
    "sdl:metal",
    "sdl:auto",
    "auto:auto",
];

/// Normalize one `layer:api` selection (or a single legacy backend name).
///
/// Rules match the C parser: a single layer means `layer:auto`, a single API
/// means `native:<api>` except DirectX/Metal, which only exist below SDL.
pub fn parse_selection(text: &str) -> Result<(String, String)> {
    let text = text.trim();
    if text.is_empty() {
        return Ok(("auto".to_string(), "auto".to_string()));
    }

    if let Some((layer, api)) = text.split_once(':') {
        if layer.contains(':') || api.contains(':') {
            return Err(Error::Invalid(format!(
                "invalid renderer selection '{text}' (expected layer:api)"
            )));
        }
        let layer = normalize_layer(layer)?;
        let api = normalize_api(api)?;
        return normalize_pair(layer, api);
    }

    if let Some(layer) = match_layer(text) {
        return normalize_pair(layer, "auto");
    }
    if let Some(api) = match_api(text) {
        return normalize_pair("auto", api);
    }
    Err(Error::Invalid(format!(
        "unknown renderer selection '{text}' (expected layer:api)"
    )))
}

fn match_layer(value: &str) -> Option<&'static str> {
    match value.to_ascii_lowercase().as_str() {
        "native" | "glfw" => Some("native"),
        "sdl" | "sdl3" => Some("sdl"),
        "auto" => Some("auto"),
        _ => None,
    }
}

fn match_api(value: &str) -> Option<&'static str> {
    match value.to_ascii_lowercase().as_str() {
        "opengl" | "gl" => Some("opengl"),
        "vulkan" | "vk" => Some("vulkan"),
        "directx" | "dx" | "d3d" | "d3d11" | "d3d12" => Some("directx"),
        "metal" => Some("metal"),
        "auto" => Some("auto"),
        _ => None,
    }
}

fn normalize_layer(value: &str) -> Result<&'static str> {
    if value.is_empty() {
        return Ok("auto");
    }
    match_layer(value).ok_or_else(|| {
        Error::Invalid(format!(
            "unknown renderer layer '{value}' (native, sdl or auto)"
        ))
    })
}

fn normalize_api(value: &str) -> Result<&'static str> {
    if value.is_empty() {
        return Ok("auto");
    }
    match_api(value).ok_or_else(|| {
        Error::Invalid(format!(
            "unknown renderer API '{value}' (opengl, vulkan, directx, metal or auto)"
        ))
    })
}

fn normalize_pair(layer: &'static str, api: &'static str) -> Result<(String, String)> {
    if layer == "native" && matches!(api, "directx" | "metal") {
        return Err(Error::Invalid(format!(
            "{api} is only available through the sdl layer (use sdl:{api})"
        )));
    }
    let layer = match layer {
        "auto" => {
            if api == "auto" {
                "auto"
            } else if matches!(api, "directx" | "metal") {
                "sdl"
            } else {
                "native"
            }
        }
        other => other,
    };
    Ok((layer.to_string(), api.to_string()))
}

/// A complete launcher instance.
#[derive(Clone, Debug, Serialize, Deserialize, PartialEq, Eq)]
pub struct Profile {
    #[serde(default = "default_schema")]
    pub schema: u32,
    pub id: String,
    pub name: String,
    #[serde(default = "default_game_version")]
    pub game_version: String,
    #[serde(default = "default_selection_text")]
    pub selection: String,
    pub instance_dir: PathBuf,
    #[serde(default)]
    pub game_dir: PathBuf,
    #[serde(default)]
    pub mods_dir: PathBuf,
    #[serde(default)]
    pub logs_dir: PathBuf,
    #[serde(default = "default_memory")]
    pub memory_mb: u32,
    #[serde(default)]
    pub fullscreen: bool,
    #[serde(default = "default_width")]
    pub width: u32,
    #[serde(default = "default_height")]
    pub height: u32,
    #[serde(default)]
    pub extra_args: Vec<String>,
    #[serde(default)]
    pub created_at: Option<String>,
    #[serde(default)]
    pub last_played: Option<String>,
}

fn default_schema() -> u32 {
    PROFILE_SCHEMA
}
fn default_game_version() -> String {
    "1.21.11".to_string()
}
fn default_selection_text() -> String {
    crate::platform::default_selection()
}
fn default_memory() -> u32 {
    2048
}
fn default_width() -> u32 {
    854
}
fn default_height() -> u32 {
    480
}

impl Profile {
    /// Create the profile and its directory layout below `instances_root`.
    pub fn new(
        instances_root: &Path,
        name: &str,
        game_version: &str,
        selection: &str,
    ) -> Result<Self> {
        let id = slugify(name);
        let instance_dir = instances_root.join(&id);
        let (layer, api) = parse_selection(selection)?;
        let profile = Profile {
            schema: PROFILE_SCHEMA,
            id,
            name: name.trim().to_string(),
            game_version: game_version.trim().to_string(),
            selection: format!("{layer}:{api}"),
            instance_dir: instance_dir.clone(),
            game_dir: instance_dir.clone(),
            mods_dir: instance_dir.join("mods"),
            logs_dir: instance_dir.join("logs"),
            memory_mb: default_memory(),
            fullscreen: false,
            width: default_width(),
            height: default_height(),
            extra_args: Vec::new(),
            created_at: Some(crate::now_rfc3339()),
            last_played: None,
        };
        profile.validate()?;
        profile.ensure_dirs()?;
        Ok(profile)
    }

    /// Load `instance_dir/profile.json`.
    pub fn load(instance_dir: &Path) -> Result<Self> {
        let path = instance_dir.join(PROFILE_FILE);
        let data = std::fs::read(&path).map_err(|error| {
            if error.kind() == std::io::ErrorKind::NotFound {
                Error::NotFound(path.display().to_string())
            } else {
                Error::Io(error)
            }
        })?;
        let mut profile: Profile = serde_json::from_slice(&data)?;
        profile.resolve_relative_paths(instance_dir);
        profile.validate()?;
        Ok(profile)
    }

    pub fn ensure_dirs(&self) -> Result<()> {
        for path in [
            &self.instance_dir,
            &self.game_dir,
            &self.mods_dir,
            &self.logs_dir,
        ] {
            std::fs::create_dir_all(path)?;
        }
        Ok(())
    }

    pub fn save(&self) -> Result<()> {
        self.ensure_dirs()?;
        crate::write_json_atomic(&self.instance_dir.join(PROFILE_FILE), self)
    }

    pub fn validate(&self) -> Result<()> {
        if self.name.trim().is_empty() {
            return Err(Error::Invalid("profile name must not be empty".into()));
        }
        if self.name.chars().count() > 64 {
            return Err(Error::Invalid("profile name is longer than 64 characters".into()));
        }
        if self.game_version.trim().is_empty() {
            return Err(Error::Invalid("game version must not be empty".into()));
        }
        parse_selection(&self.selection)?;
        if !(512..=65_536).contains(&self.memory_mb) {
            return Err(Error::Invalid(format!(
                "memory must be between 512 and 65536 MiB, got {}",
                self.memory_mb
            )));
        }
        if !(320..=7_680).contains(&self.width) || !(240..=4_320).contains(&self.height) {
            return Err(Error::Invalid(format!(
                "window size {}x{} is outside the supported range",
                self.width, self.height
            )));
        }
        if self.instance_dir.as_os_str().is_empty()
            || self.game_dir.as_os_str().is_empty()
            || self.mods_dir.as_os_str().is_empty()
            || self.logs_dir.as_os_str().is_empty()
        {
            return Err(Error::Invalid("profile paths must not be empty".into()));
        }
        Ok(())
    }

    pub fn selection_parts(&self) -> Result<(String, String)> {
        parse_selection(&self.selection)
    }

    pub fn normalized_selection(&self) -> Result<String> {
        let (layer, api) = self.selection_parts()?;
        Ok(format!("{layer}:{api}"))
    }

    pub fn set_selection(&mut self, selection: &str) -> Result<()> {
        let (layer, api) = parse_selection(selection)?;
        self.selection = format!("{layer}:{api}");
        Ok(())
    }

    pub fn touch_played(&mut self) {
        self.last_played = Some(crate::now_rfc3339());
    }

    pub fn display_selection(&self) -> String {
        self.normalized_selection()
            .unwrap_or_else(|_| self.selection.clone())
    }

    pub(crate) fn resolve_relative_paths(&mut self, instance_dir: &Path) {
        for path in [
            &mut self.instance_dir,
            &mut self.game_dir,
            &mut self.mods_dir,
            &mut self.logs_dir,
        ] {
            if path.is_relative() {
                *path = instance_dir.join(&*path);
            }
        }
        if self.game_dir.as_os_str().is_empty() {
            self.game_dir = self.instance_dir.clone();
        }
        if self.mods_dir.as_os_str().is_empty() {
            self.mods_dir = self.instance_dir.join("mods");
        }
        if self.logs_dir.as_os_str().is_empty() {
            self.logs_dir = self.instance_dir.join("logs");
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn selection_forms_normalize() {
        assert_eq!(
            parse_selection("vulkan").unwrap(),
            ("native".into(), "vulkan".into())
        );
        assert_eq!(
            parse_selection("sdl3").unwrap(),
            ("sdl".into(), "auto".into())
        );
        assert_eq!(
            parse_selection("sdl:dx").unwrap(),
            ("sdl".into(), "directx".into())
        );
        assert_eq!(
            parse_selection("metal").unwrap(),
            ("sdl".into(), "metal".into())
        );
        assert_eq!(
            parse_selection("auto").unwrap(),
            ("auto".into(), "auto".into())
        );
        assert_eq!(
            parse_selection("").unwrap(),
            ("auto".into(), "auto".into())
        );
        assert!(parse_selection("native:directx").is_err());
        assert!(parse_selection("sdl:glide").is_err());
    }

    #[test]
    fn profile_roundtrip() {
        let root = crate::temp_dir("profile");
        let mut profile = Profile::new(&root, "My World", "1.21.11", "sdl:vulkan").unwrap();
        assert_eq!(profile.selection, "sdl:vulkan");
        assert!(profile.mods_dir.ends_with("mods"));
        profile.memory_mb = 4096;
        profile.save().unwrap();
        let loaded = Profile::load(&profile.instance_dir).unwrap();
        assert_eq!(loaded, profile);
        let _ = std::fs::remove_dir_all(root);
    }

    #[test]
    fn invalid_profiles_are_rejected() {
        let root = crate::temp_dir("profile-invalid");
        let mut profile = Profile::new(&root, "Test", "1.21.11", "auto").unwrap();
        profile.selection = "native:metal".into();
        assert!(profile.validate().is_err());
        profile.selection = "sdl:auto".into();
        profile.memory_mb = 12;
        assert!(profile.validate().is_err());
        let _ = std::fs::remove_dir_all(root);
    }

    #[test]
    fn relative_paths_resolve_into_instance() {
        let root = crate::temp_dir("profile-relative");
        let mut profile = Profile::new(&root, "Relative", "1.21.11", "auto").unwrap();
        let saved = profile.clone();
        let mut value = serde_json::to_value(&saved).unwrap();
        value["instance_dir"] = serde_json::json!(".");
        value["mods_dir"] = serde_json::json!("mods");
        value["game_dir"] = serde_json::json!("");
        std::fs::write(
            profile.instance_dir.join(PROFILE_FILE),
            serde_json::to_vec_pretty(&value).unwrap(),
        )
        .unwrap();
        profile = Profile::load(&profile.instance_dir).unwrap();
        assert_eq!(profile.game_dir, saved.instance_dir);
        assert_eq!(profile.mods_dir, saved.instance_dir.join("mods"));
        let _ = std::fs::remove_dir_all(root);
    }
}
