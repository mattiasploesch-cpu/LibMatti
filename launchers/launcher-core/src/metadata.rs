use std::path::{Path, PathBuf};

use serde::{Deserialize, Serialize};
use serde_json::Value;

use crate::{slugify, Error, Result};

pub const METADATA_FILE: &str = "metadata.matti";
pub const METADATA_SCHEMA: u32 = 1;

#[derive(Clone, Debug, Serialize, Deserialize, PartialEq, Eq)]
pub struct ModMetadata {
    #[serde(default = "default_schema")]
    pub schema: u32,
    pub id: String,
    pub name: String,
    pub version: String,
    #[serde(default)]
    pub author: String,
    #[serde(default)]
    pub description: String,
    #[serde(default)]
    pub license: String,
    #[serde(default)]
    pub homepage: String,
    #[serde(default)]
    pub entry: String,
    #[serde(default)]
    pub game_versions: Vec<String>,
    #[serde(default)]
    pub tags: Vec<String>,
    #[serde(default)]
    pub required: bool,
}

fn default_schema() -> u32 {
    METADATA_SCHEMA
}

impl ModMetadata {
    pub fn new(id: &str, name: &str, version: &str) -> Result<Self> {
        let metadata = Self {
            schema: METADATA_SCHEMA,
            id: slugify(id),
            name: name.trim().to_string(),
            version: version.trim().to_string(),
            author: String::new(),
            description: String::new(),
            license: String::new(),
            homepage: String::new(),
            entry: String::new(),
            game_versions: Vec::new(),
            tags: Vec::new(),
            required: true,
        };
        metadata.validate()?;
        Ok(metadata)
    }

    pub fn validate(&self) -> Result<()> {
        if self.schema > METADATA_SCHEMA {
            return Err(Error::Unsupported(format!(
                "metadata schema {} is newer than {METADATA_SCHEMA}",
                self.schema
            )));
        }
        if self.id.trim().is_empty() {
            return Err(Error::Invalid("metadata id must not be empty".into()));
        }
        if slugify(&self.id) != self.id {
            return Err(Error::Invalid(format!(
                "metadata id '{}' must be a lowercase slug",
                self.id
            )));
        }
        if self.name.trim().is_empty() {
            return Err(Error::Invalid("metadata name must not be empty".into()));
        }
        if self.version.trim().is_empty() {
            return Err(Error::Invalid("metadata version must not be empty".into()));
        }
        Ok(())
    }

    pub fn to_json(&self) -> Result<String> {
        self.validate()?;
        serde_json::to_string_pretty(self).map_err(Error::from)
    }

    pub fn from_json(bytes: &[u8]) -> Result<Self> {
        let value: Value = serde_json::from_slice(bytes)?;
        let metadata = parse_value(&value)?;
        metadata.validate()?;
        Ok(metadata)
    }

    pub fn from_slice(bytes: &[u8]) -> Result<Self> {
        if let Ok(metadata) = Self::from_json(bytes) {
            return Ok(metadata);
        }
        let text = String::from_utf8_lossy(bytes);
        let mut id = String::new();
        let mut name = String::new();
        let mut version = String::new();
        let mut author = String::new();
        let mut description = String::new();
        for line in text.lines() {
            let line = line.trim();
            if line.is_empty() || line.starts_with('#') {
                continue;
            }
            let Some((key, value)) = line.split_once(':') else {
                continue;
            };
            let value = value.trim().to_string();
            match key.trim().to_ascii_lowercase().as_str() {
                "matti-mod-id" | "id" | "modid" => id = value,
                "matti-mod-name" | "name" => name = value,
                "matti-mod-version" | "version" => version = value,
                "author" => author = value,
                "description" | "summary" => description = value,
                _ => {}
            }
        }
        if id.is_empty() || name.is_empty() || version.is_empty() {
            return Err(Error::Invalid(
                "metadata.matti must be JSON or contain id, name and version".into(),
            ));
        }
        let metadata = Self {
            schema: METADATA_SCHEMA,
            id: slugify(&id),
            name,
            version,
            author,
            description,
            license: String::new(),
            homepage: String::new(),
            entry: String::new(),
            game_versions: Vec::new(),
            tags: Vec::new(),
            required: true,
        };
        metadata.validate()?;
        Ok(metadata)
    }

    pub fn save(&self, dir: &Path) -> Result<PathBuf> {
        self.validate()?;
        std::fs::create_dir_all(dir)?;
        let path = dir.join(METADATA_FILE);
        let temp = path.with_extension("matti.tmp");
        std::fs::write(&temp, self.to_json()?)?;
        if path.exists() {
            std::fs::remove_file(&path)?;
        }
        std::fs::rename(&temp, &path)?;
        Ok(path)
    }

    pub fn load(dir: &Path) -> Result<Self> {
        let path = dir.join(METADATA_FILE);
        if !path.is_file() {
            return Err(Error::NotFound(format!(
                "{} is required in the mod root",
                path.display()
            )));
        }
        Self::from_slice(&std::fs::read(path)?)
    }

    pub fn load_file(path: &Path) -> Result<Self> {
        Self::from_slice(&std::fs::read(path)?)
    }

    pub fn exists(dir: &Path) -> bool {
        dir.join(METADATA_FILE).is_file()
    }

    pub fn from_package(path: &Path) -> Result<Self> {
        let bytes = std::fs::read(path)?;
        let marker = b"Matti-Mod-Id:";
        if let Some(position) = find_bytes(&bytes, marker) {
            let start = position.saturating_sub(4096);
            let end = (position + 8192).min(bytes.len());
            if let Ok(metadata) = Self::from_slice(&bytes[start..end]) {
                return Ok(metadata);
            }
        }
        let text = String::from_utf8_lossy(&bytes);
        if let Some(start) = text.find("{\"schema\"") {
            if let Some(end) = text[start..].find('}') {
                if let Ok(metadata) = Self::from_slice(text[start..=start + end].as_bytes()) {
                    return Ok(metadata);
                }
            }
        }
        Err(Error::NotFound(format!(
            "embedded metadata in {}",
            path.display()
        )))
    }
}

pub fn read_package(path: &Path) -> Result<ModMetadata> {
    if path.extension().and_then(|value| value.to_str()) != Some("matti") {
        return Err(Error::Unsupported(format!(
            "{} is not a .matti package",
            path.display()
        )));
    }
    ModMetadata::from_package(path)
}

fn find_bytes(haystack: &[u8], needle: &[u8]) -> Option<usize> {
    haystack
        .windows(needle.len())
        .position(|window| window == needle)
}

fn parse_value(value: &Value) -> Result<ModMetadata> {
    let object = value
        .as_object()
        .ok_or_else(|| Error::Invalid("metadata.matti must be a JSON object".into()))?;
    let id = string_field(object, &["id", "mod_id", "modId", "slug"]);
    let name = string_field(object, &["name", "title", "display_name", "displayName"]);
    let version = string_field(object, &["version", "mod_version", "modVersion"]);
    let id = if id.is_empty() {
        slugify(&name)
    } else {
        id
    };
    let author = match object.get("author") {
        Some(Value::String(text)) => text.clone(),
        Some(Value::Object(author)) => string_field(author, &["name", "username"]),
        _ => string_field(object, &["authors"]),
    };
    let mut game_versions = Vec::new();
    if let Some(values) = object.get("game_versions").and_then(Value::as_array) {
        game_versions = values
            .iter()
            .filter_map(Value::as_str)
            .map(ToString::to_string)
            .collect();
    }
    let mut tags = Vec::new();
    if let Some(values) = object.get("tags").and_then(Value::as_array) {
        tags = values
            .iter()
            .filter_map(Value::as_str)
            .map(ToString::to_string)
            .collect();
    }
    let metadata = ModMetadata {
        schema: object
            .get("schema")
            .and_then(Value::as_u64)
            .unwrap_or(METADATA_SCHEMA as u64) as u32,
        id,
        name,
        version,
        author,
        description: string_field(object, &["description", "summary", "body"]),
        license: string_field(object, &["license"]),
        homepage: string_field(object, &["homepage", "url", "website"]),
        entry: string_field(object, &["entry", "entrypoint", "main"]),
        game_versions,
        tags,
        required: object
            .get("required")
            .and_then(Value::as_bool)
            .unwrap_or(true),
    };
    Ok(metadata)
}

fn string_field(object: &serde_json::Map<String, Value>, keys: &[&str]) -> String {
    keys.iter()
        .find_map(|key| object.get(*key).and_then(Value::as_str))
        .unwrap_or("")
        .trim()
        .to_string()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn json_metadata_roundtrip() {
        let mut metadata = ModMetadata::new("My Mod", "My Mod", "1.2.0").unwrap();
        metadata.author = "Matticraft".into();
        let dir = crate::temp_dir("metadata");
        metadata.save(&dir).unwrap();
        assert_eq!(ModMetadata::load(&dir).unwrap(), metadata);
        let _ = std::fs::remove_dir_all(dir);
    }

    #[test]
    fn key_value_metadata_is_accepted() {
        let metadata = ModMetadata::from_slice(
            b"Matti-Mod-Id: examplemod\nMatti-Mod-Name: Example\nMatti-Mod-Version: 1.0\n",
        )
        .unwrap();
        assert_eq!(metadata.id, "examplemod");
        assert_eq!(metadata.version, "1.0");
    }

    #[test]
    fn missing_metadata_is_an_error() {
        let dir = crate::temp_dir("metadata-missing");
        assert!(ModMetadata::load(&dir).is_err());
        let _ = std::fs::remove_dir_all(dir);
    }
}
