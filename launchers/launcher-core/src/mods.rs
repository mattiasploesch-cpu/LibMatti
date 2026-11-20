//! Mods folder scanning and manifest validation.
//!
//! The game loads native mods (`*.so`, `*.dll`, `*.dylib`, `*.matti`) from the
//! profile's `mods/` directory. The launcher keeps a `mods.json` next to them
//! with the expected SHA-256 of every file; a changed or missing file is
//! reported before the game starts instead of failing inside the loader.

use std::collections::BTreeSet;
use std::io::Read;
use std::path::{Path, PathBuf};

use serde::{Deserialize, Serialize};

use crate::{Error, Result};

pub const MANIFEST_FILE: &str = "mods.json";
pub const MANIFEST_SCHEMA: u32 = 1;

const MOD_EXTENSIONS: &[&str] = &["so", "dll", "dylib", "matti", "jar"];

#[derive(Clone, Debug, Serialize, Deserialize, PartialEq, Eq)]
pub struct ModEntry {
    pub id: String,
    pub name: String,
    #[serde(default)]
    pub version: String,
    #[serde(default)]
    pub author: String,
    #[serde(default)]
    pub description: String,
    pub file: String,
    pub sha256: String,
    #[serde(default)]
    pub required: bool,
}

#[derive(Clone, Debug, Serialize, Deserialize, PartialEq, Eq)]
pub struct ModManifest {
    #[serde(default = "default_schema")]
    pub schema: u32,
    #[serde(default)]
    pub mods: Vec<ModEntry>,
}

fn default_schema() -> u32 {
    MANIFEST_SCHEMA
}

#[derive(Clone, Debug, Serialize, Deserialize, PartialEq, Eq)]
pub struct InstalledMod {
    pub file: String,
    pub size: u64,
    pub sha256: String,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct ModMismatch {
    pub file: String,
    pub expected: String,
    pub actual: String,
}

#[derive(Clone, Debug, Default, PartialEq, Eq)]
pub struct ModValidation {
    pub manifest_found: bool,
    pub installed: Vec<InstalledMod>,
    pub missing: Vec<String>,
    pub mismatched: Vec<ModMismatch>,
    pub unlisted: Vec<String>,
}

impl ModValidation {
    pub fn ok(&self) -> bool {
        self.missing.is_empty() && self.mismatched.is_empty()
    }

    pub fn summary(&self) -> String {
        if !self.manifest_found {
            return format!("{} mod file(s), no manifest", self.installed.len());
        }
        if self.ok() {
            return format!("{} mod(s) verified", self.installed.len());
        }
        format!(
            "{} missing, {} changed, {} unlisted",
            self.missing.len(),
            self.mismatched.len(),
            self.unlisted.len()
        )
    }
}

pub fn is_mod_file(path: &Path) -> bool {
    path.extension()
        .and_then(|extension| extension.to_str())
        .is_some_and(|extension| {
            MOD_EXTENSIONS
                .iter()
                .any(|known| extension.eq_ignore_ascii_case(known))
        })
}

pub fn scan(dir: &Path) -> Result<Vec<InstalledMod>> {
    std::fs::create_dir_all(dir)?;
    let mut files = Vec::new();
    for entry in std::fs::read_dir(dir)? {
        let entry = entry?;
        let path = entry.path();
        if !entry.file_type()?.is_file() || !is_mod_file(&path) {
            continue;
        }
        let metadata = entry.metadata()?;
        files.push(InstalledMod {
            file: entry.file_name().to_string_lossy().to_string(),
            size: metadata.len(),
            sha256: sha256_file(&path)?,
        });
    }
    files.sort_by(|left, right| left.file.cmp(&right.file));
    Ok(files)
}

pub fn read_manifest(dir: &Path) -> Result<Option<ModManifest>> {
    let path = dir.join(MANIFEST_FILE);
    if !path.exists() {
        return Ok(None);
    }
    let manifest: ModManifest = serde_json::from_slice(&std::fs::read(path)?)?;
    for entry in &manifest.mods {
        validate_relative_name(&entry.file)?;
    }
    Ok(Some(manifest))
}

pub fn write_manifest(dir: &Path, manifest: &ModManifest) -> Result<()> {
    for entry in &manifest.mods {
        validate_relative_name(&entry.file)?;
    }
    crate::write_json_atomic(&dir.join(MANIFEST_FILE), manifest)
}

/// Build a manifest for every mod currently in the folder.
pub fn build_manifest(dir: &Path) -> Result<ModManifest> {
    let installed = scan(dir)?;
    let mods = installed
        .into_iter()
        .map(|file| {
            let path = dir.join(&file.file);
            let metadata = crate::metadata::read_package(&path).ok();
            let fallback_id = Path::new(&file.file)
                .file_stem()
                .map(|stem| stem.to_string_lossy().to_string())
                .unwrap_or_else(|| file.file.clone());
            let id = metadata
                .as_ref()
                .map(|metadata| metadata.id.clone())
                .filter(|id| !id.is_empty())
                .unwrap_or(fallback_id);
            ModEntry {
                name: metadata
                    .as_ref()
                    .map(|metadata| metadata.name.clone())
                    .filter(|name| !name.is_empty())
                    .unwrap_or_else(|| id.clone()),
                id,
                version: metadata
                    .as_ref()
                    .map(|metadata| metadata.version.clone())
                    .unwrap_or_default(),
                author: metadata
                    .as_ref()
                    .map(|metadata| metadata.author.clone())
                    .unwrap_or_default(),
                description: metadata
                    .as_ref()
                    .map(|metadata| metadata.description.clone())
                    .unwrap_or_default(),
                file: file.file,
                sha256: file.sha256,
                required: true,
            }
        })
        .collect();
    Ok(ModManifest {
        schema: MANIFEST_SCHEMA,
        mods,
    })
}

pub fn validate(dir: &Path) -> Result<ModValidation> {
    let manifest = read_manifest(dir)?;
    let installed = scan(dir)?;
    let mut result = ModValidation {
        manifest_found: manifest.is_some(),
        installed: installed.clone(),
        ..ModValidation::default()
    };
    let Some(manifest) = manifest else {
        result.unlisted = installed.into_iter().map(|file| file.file).collect();
        return Ok(result);
    };

    let installed_by_name: std::collections::BTreeMap<_, _> = installed
        .iter()
        .map(|file| (file.file.clone(), file))
        .collect();
    let mut listed = BTreeSet::new();
    for entry in &manifest.mods {
        validate_relative_name(&entry.file)?;
        listed.insert(entry.file.clone());
        match installed_by_name.get(&entry.file) {
            None => result.missing.push(entry.file.clone()),
            Some(file) if !file.sha256.eq_ignore_ascii_case(&entry.sha256) => {
                result.mismatched.push(ModMismatch {
                    file: entry.file.clone(),
                    expected: entry.sha256.clone(),
                    actual: file.sha256.clone(),
                });
            }
            Some(_) => {}
        }
    }
    result.unlisted = installed
        .into_iter()
        .map(|file| file.file)
        .filter(|file| !listed.contains(file))
        .collect();
    Ok(result)
}

pub fn sha256_file(path: &Path) -> Result<String> {
    let mut context = ring::digest::Context::new(&ring::digest::SHA256);
    let mut file = std::fs::File::open(path)?;
    let mut buffer = [0u8; 64 * 1024];
    loop {
        let read = file.read(&mut buffer)?;
        if read == 0 {
            break;
        }
        context.update(&buffer[..read]);
    }
    Ok(hex::encode(context.finish().as_ref()))
}

fn validate_relative_name(name: &str) -> Result<()> {
    let path = Path::new(name);
    if name.is_empty()
        || path.is_absolute()
        || path.components().any(|component| {
            matches!(
                component,
                std::path::Component::ParentDir
                    | std::path::Component::RootDir
                    | std::path::Component::Prefix(_)
            )
        })
    {
        return Err(Error::Invalid(format!(
            "mod manifest path '{name}' must stay inside the mods folder"
        )));
    }
    Ok(())
}

pub fn entry_path(dir: &Path, entry: &ModEntry) -> Result<PathBuf> {
    validate_relative_name(&entry.file)?;
    Ok(dir.join(&entry.file))
}

#[cfg(test)]
mod tests {
    use super::*;

    fn write(path: &Path, bytes: &[u8]) {
        std::fs::write(path, bytes).unwrap();
    }

    #[test]
    fn manifest_roundtrip_and_validation() {
        let dir = crate::temp_dir("mods");
        write(&dir.join("examplemod-1.0.so"), b"first");
        write(&dir.join("other.dll"), b"second");
        write(&dir.join("readme.txt"), b"ignored");

        let manifest = build_manifest(&dir).unwrap();
        assert_eq!(manifest.mods.len(), 2);
        write_manifest(&dir, &manifest).unwrap();

        let result = validate(&dir).unwrap();
        assert!(result.manifest_found);
        assert!(result.ok());
        assert!(result.unlisted.is_empty());

        write(&dir.join("examplemod-1.0.so"), b"tampered");
        let result = validate(&dir).unwrap();
        assert!(!result.ok());
        assert_eq!(result.mismatched.len(), 1);

        std::fs::remove_file(dir.join("other.dll")).unwrap();
        let result = validate(&dir).unwrap();
        assert_eq!(result.missing, vec!["other.dll"]);

        let _ = std::fs::remove_dir_all(dir);
    }

    #[test]
    fn manifest_rejects_escaping_paths() {
        let dir = crate::temp_dir("mods-escape");
        let manifest = ModManifest {
            schema: 1,
            mods: vec![ModEntry {
                id: "bad".into(),
                name: "bad".into(),
                version: String::new(),
                author: String::new(),
                description: String::new(),
                file: "../evil.so".into(),
                sha256: String::new(),
                required: true,
            }],
        };
        assert!(write_manifest(&dir, &manifest).is_err());
        let _ = std::fs::remove_dir_all(dir);
    }

    #[test]
    fn empty_directory_is_valid() {
        let dir = crate::temp_dir("mods-empty");
        let result = validate(&dir).unwrap();
        assert!(!result.manifest_found);
        assert!(result.ok());
        assert!(result.installed.is_empty());
        let _ = std::fs::remove_dir_all(dir);
    }

    #[test]
    fn sha256_matches_known_value() {
        let dir = crate::temp_dir("mods-hash");
        let path = dir.join("file.so");
        write(&path, b"abc");
        assert_eq!(
            sha256_file(&path).unwrap(),
            "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"
        );
        let _ = std::fs::remove_dir_all(dir);
    }
}
