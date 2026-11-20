use std::path::{Path, PathBuf};

use serde::{Deserialize, Serialize};
use serde_json::{Map, Value};

use crate::{Error, Result};
#[cfg(feature = "upload")]
use crate::mods;

pub const CATALOG_SCHEMA: u32 = 1;
pub const CATALOG_FILE: &str = "mods.json";
pub const DEFAULT_CATALOG_REPO: &str = "mattiasploesch-cpu/LibMatti";
pub const FALLBACK_CATALOG_REPO: &str = "mattiasploesch-cpu/mods";

#[derive(Clone, Debug, Default, Serialize, Deserialize, PartialEq, Eq)]
pub struct CatalogFile {
    #[serde(default = "default_any")]
    pub platform: String,
    #[serde(default = "default_any")]
    pub arch: String,
    #[serde(default)]
    pub file: String,
    pub url: String,
    #[serde(default)]
    pub sha256: String,
    #[serde(default)]
    pub size: u64,
}

fn default_any() -> String {
    "any".to_string()
}

#[derive(Clone, Debug, Default, Serialize, Deserialize, PartialEq, Eq)]
pub struct CatalogEntry {
    #[serde(default)]
    pub id: String,
    #[serde(default)]
    pub name: String,
    #[serde(default)]
    pub version: String,
    #[serde(default)]
    pub author: String,
    #[serde(default)]
    pub summary: String,
    #[serde(default)]
    pub description: String,
    #[serde(default)]
    pub license: String,
    #[serde(default)]
    pub homepage: String,
    #[serde(default)]
    pub icon: String,
    #[serde(default)]
    pub featured: bool,
    #[serde(default)]
    pub game_versions: Vec<String>,
    #[serde(default)]
    pub tags: Vec<String>,
    #[serde(default)]
    pub file: String,
    #[serde(default)]
    pub url: String,
    #[serde(default)]
    pub sha256: String,
    #[serde(default)]
    pub size: u64,
    #[serde(default)]
    pub downloads: Vec<CatalogFile>,
}

impl CatalogEntry {
    pub fn normalize(&mut self) {
        if self.id.trim().is_empty() {
            self.id = crate::slugify(&self.name);
        } else {
            self.id = crate::slugify(&self.id);
        }
        if self.name.trim().is_empty() {
            self.name = self.id.clone();
        }
        if self.summary.trim().is_empty() {
            self.summary = self.description.lines().next().unwrap_or("").to_string();
        }
        if self.description.trim().is_empty() {
            self.description = self.summary.clone();
        }
        self.downloads.retain(|file| !file.url.trim().is_empty());
        if self.downloads.is_empty() && !self.url.trim().is_empty() {
            self.downloads.push(CatalogFile {
                platform: "any".into(),
                arch: "any".into(),
                file: if self.file.trim().is_empty() {
                    format!("{}-{}.matti", self.id, self.version)
                } else {
                    self.file.clone()
                },
                url: self.url.clone(),
                sha256: self.sha256.clone(),
                size: self.size,
            });
        }
    }

    pub fn download_for(&self, platform: &str, arch: &str) -> Option<&CatalogFile> {
        self.downloads
            .iter()
            .find(|file| file.platform == platform && file.arch == arch)
            .or_else(|| {
                self.downloads
                    .iter()
                    .find(|file| file.platform == platform && file.arch == "any")
            })
            .or_else(|| {
                self.downloads
                    .iter()
                    .find(|file| file.platform == "any" && file.arch == arch)
            })
            .or_else(|| self.downloads.iter().find(|file| file.platform == "any" && file.arch == "any"))
    }

    pub fn compatible(&self, version: &str) -> bool {
        self.game_versions.is_empty()
            || self
                .game_versions
                .iter()
                .any(|candidate| candidate == version || candidate == "*")
    }

    pub fn matches(&self, query: &str) -> bool {
        let query = query.trim().to_ascii_lowercase();
        if query.is_empty() {
            return true;
        }
        self.id.to_ascii_lowercase().contains(&query)
            || self.name.to_ascii_lowercase().contains(&query)
            || self.summary.to_ascii_lowercase().contains(&query)
            || self
                .tags
                .iter()
                .any(|tag| tag.to_ascii_lowercase().contains(&query))
    }

    pub fn local_file(&self, platform: &str, arch: &str) -> String {
        self.download_for(platform, arch)
            .map(|file| file.file.clone())
            .filter(|file| !file.trim().is_empty())
            .unwrap_or_else(|| format!("{}-{}.matti", self.id, self.version))
    }
}

#[derive(Clone, Debug, Default, Serialize, Deserialize, PartialEq, Eq)]
pub struct ModCatalog {
    #[serde(default = "default_schema")]
    pub schema: u32,
    #[serde(default)]
    pub name: String,
    #[serde(default)]
    pub updated: String,
    #[serde(default)]
    pub repository: String,
    #[serde(default)]
    pub mods: Vec<CatalogEntry>,
}

fn default_schema() -> u32 {
    CATALOG_SCHEMA
}

impl ModCatalog {
    pub fn from_json(bytes: &[u8], repository: &str) -> Result<Self> {
        let value: Value = serde_json::from_slice(bytes)?;
        let mut catalog = if let Some(items) = value.as_array() {
            ModCatalog {
                schema: CATALOG_SCHEMA,
                name: "Matticraft mods".into(),
                repository: repository.into(),
                mods: parse_entries(items)?,
                ..ModCatalog::default()
            }
        } else {
            let object = value
                .as_object()
                .ok_or_else(|| Error::Invalid("mods.json must be an object or a list".into()))?;
            let items = object
                .get("mods")
                .and_then(Value::as_array)
                .ok_or_else(|| Error::Invalid("mods.json has no mods list".into()))?;
            ModCatalog {
                schema: object
                    .get("schema")
                    .and_then(Value::as_u64)
                    .unwrap_or(CATALOG_SCHEMA as u64) as u32,
                name: string_field(object, &["name", "title"])
                    .unwrap_or_else(|| "Matticraft mods".into()),
                updated: string_field(object, &["updated", "updated_at", "generated"]).unwrap_or_default(),
                repository: string_field(object, &["repository", "repo"]).unwrap_or_else(|| repository.into()),
                mods: parse_entries(items)?,
            }
        };
        if catalog.schema > CATALOG_SCHEMA {
            return Err(Error::Unsupported(format!(
                "mods.json schema {} is newer than {CATALOG_SCHEMA}",
                catalog.schema
            )));
        }
        catalog.mods.retain(|entry| !entry.id.is_empty() || !entry.name.is_empty());
        for entry in &mut catalog.mods {
            entry.normalize();
        }
        catalog.mods.sort_by(|left, right| {
            right
                .featured
                .cmp(&left.featured)
                .then_with(|| left.name.to_ascii_lowercase().cmp(&right.name.to_ascii_lowercase()))
        });
        Ok(catalog)
    }

    pub fn load(path: &Path, repository: &str) -> Result<Self> {
        Self::from_json(&std::fs::read(path)?, repository)
    }

    pub fn find(&self, id: &str) -> Option<&CatalogEntry> {
        self.mods.iter().find(|entry| entry.id == id)
    }

    pub fn filtered(&self, query: &str, tag: &str) -> Vec<&CatalogEntry> {
        self.mods
            .iter()
            .filter(|entry| entry.matches(query))
            .filter(|entry| tag.is_empty() || entry.tags.iter().any(|candidate| candidate == tag))
            .collect()
    }

    pub fn tags(&self) -> Vec<String> {
        let mut tags = self
            .mods
            .iter()
            .flat_map(|entry| entry.tags.iter().cloned())
            .collect::<Vec<_>>();
        tags.sort();
        tags.dedup();
        tags
    }

    pub fn repository_url(repo: &str, file: &str) -> String {
        format!("https://raw.githubusercontent.com/{repo}/main/{file}")
    }

    #[cfg(feature = "upload")]
    pub fn fetch(repo: &str) -> Result<Self> {
        let repo = repo.trim().trim_matches('/');
        if repo.is_empty() {
            return Err(Error::Invalid("catalog repository is empty".into()));
        }
        let mut repositories = vec![repo.to_string()];
        if repo != DEFAULT_CATALOG_REPO {
            repositories.push(DEFAULT_CATALOG_REPO.to_string());
        }
        if repo != FALLBACK_CATALOG_REPO {
            repositories.push(FALLBACK_CATALOG_REPO.to_string());
        }
        let mut errors = Vec::new();
        for candidate in repositories {
            for file in [CATALOG_FILE, "catalog/mods.json", "mods/mods.json"] {
                let url = Self::repository_url(&candidate, file);
                match crate::update::fetch_bytes(&url) {
                    Ok(bytes) => return Self::from_json(&bytes, &candidate),
                    Err(error) => errors.push(format!("{url}: {error}")),
                }
            }
        }
        Err(Error::NotFound(format!(
            "mods.json in {repo} ({})",
            errors.join("; ")
        )))
    }

    #[cfg(not(feature = "upload"))]
    pub fn fetch(repo: &str) -> Result<Self> {
        let _ = repo;
        Err(Error::Unsupported(
            "this launcher was built without network support".into(),
        ))
    }

    #[cfg(feature = "upload")]
    pub fn install(
        &self,
        entry: &CatalogEntry,
        mods_dir: &Path,
        platform: &str,
        arch: &str,
    ) -> Result<PathBuf> {
        let download = entry
            .download_for(platform, arch)
            .ok_or_else(|| Error::NotFound(format!("download for {} on {platform} {arch}", entry.id)))?;
        std::fs::create_dir_all(mods_dir)?;
        let file_name = sanitize_file_name(&download.file, &entry.id, &entry.version);
        let target = mods_dir.join(file_name);
        let temp = target.with_extension("matti.part");
        crate::update::download_to_file(&download.url, &temp)?;
        if !download.sha256.trim().is_empty() {
            crate::update::verify_file_sha256(&temp, &download.sha256, &entry.id)?;
        }
        if target.exists() {
            std::fs::remove_file(&target)?;
        }
        std::fs::rename(&temp, &target)?;
        let manifest = mods::build_manifest(mods_dir)?;
        mods::write_manifest(mods_dir, &manifest)?;
        Ok(target)
    }

    #[cfg(not(feature = "upload"))]
    pub fn install(
        &self,
        _entry: &CatalogEntry,
        _mods_dir: &Path,
        _platform: &str,
        _arch: &str,
    ) -> Result<PathBuf> {
        Err(Error::Unsupported(
            "this launcher was built without network support".into(),
        ))
    }
}

fn parse_entries(items: &[Value]) -> Result<Vec<CatalogEntry>> {
    let mut entries = Vec::new();
    for item in items {
        let object = item
            .as_object()
            .ok_or_else(|| Error::Invalid("every catalog entry must be an object".into()))?;
        entries.push(parse_entry(object)?);
    }
    Ok(entries)
}

fn parse_entry(object: &Map<String, Value>) -> Result<CatalogEntry> {
    let id = string_field(object, &["id", "mod_id", "modId", "slug"]).unwrap_or_default();
    let name = string_field(object, &["name", "title", "display_name", "displayName"]).unwrap_or_default();
    let version = string_field(object, &["version", "latest_version", "latestVersion"]).unwrap_or_default();
    let author = match object.get("author") {
        Some(Value::String(text)) => text.clone(),
        Some(Value::Object(value)) => string_field(value, &["name", "username"]).unwrap_or_default(),
        _ => string_field(object, &["authors"]).unwrap_or_default(),
    };
    let summary = string_field(object, &["summary", "tagline"]).unwrap_or_default();
    let description = string_field(object, &["description", "body", "readme"]).unwrap_or_default();
    let mut downloads = Vec::new();
    if let Some(items) = object.get("downloads").and_then(Value::as_array) {
        for item in items {
            if let Some(file) = item.as_object() {
                if let Some(url) = string_field(file, &["url", "download_url", "browser_download_url"]) {
                    downloads.push(CatalogFile {
                        platform: string_field(file, &["platform", "os"]).unwrap_or_else(default_any),
                        arch: string_field(file, &["arch", "architecture"]).unwrap_or_else(default_any),
                        file: string_field(file, &["file", "name", "filename"]).unwrap_or_default(),
                        url,
                        sha256: string_field(file, &["sha256", "digest", "checksum"]).unwrap_or_default(),
                        size: file.get("size").and_then(Value::as_u64).unwrap_or(0),
                    });
                }
            }
        }
    }
    let mut game_versions = string_list(object, &["game_versions", "gameVersions", "versions"]);
    if game_versions.is_empty() {
        if let Some(version) = string_field(object, &["game_version", "gameVersion"]) {
            game_versions.push(version);
        }
    }
    let entry = CatalogEntry {
        id,
        name,
        version,
        author,
        summary,
        description,
        license: string_field(object, &["license"]).unwrap_or_default(),
        homepage: string_field(object, &["homepage", "website", "url"]).unwrap_or_default(),
        icon: string_field(object, &["icon", "icon_url", "iconUrl"]).unwrap_or_default(),
        featured: object.get("featured").and_then(Value::as_bool).unwrap_or(false),
        game_versions,
        tags: string_list(object, &["tags", "categories"]),
        file: string_field(object, &["file", "filename", "asset"]).unwrap_or_default(),
        url: string_field(object, &["download", "download_url", "downloadUrl", "url"]).unwrap_or_default(),
        sha256: string_field(object, &["sha256", "checksum", "digest"]).unwrap_or_default(),
        size: object.get("size").and_then(Value::as_u64).unwrap_or(0),
        downloads,
    };
    Ok(entry)
}

fn string_field(object: &Map<String, Value>, keys: &[&str]) -> Option<String> {
    keys.iter()
        .find_map(|key| object.get(*key).and_then(Value::as_str))
        .map(|value| value.trim().to_string())
        .filter(|value| !value.is_empty())
}

fn string_list(object: &Map<String, Value>, keys: &[&str]) -> Vec<String> {
    keys.iter()
        .find_map(|key| object.get(*key).and_then(Value::as_array))
        .map(|values| {
            values
                .iter()
                .filter_map(Value::as_str)
                .map(ToString::to_string)
                .collect()
        })
        .unwrap_or_default()
}

#[cfg(feature = "upload")]
fn sanitize_file_name(file: &str, id: &str, version: &str) -> String {
    let name = Path::new(file)
        .file_name()
        .and_then(|name| name.to_str())
        .unwrap_or("");
    if !name.is_empty()
        && name
            .chars()
            .all(|ch| ch.is_ascii_alphanumeric() || matches!(ch, '.' | '-' | '_'))
    {
        return name.to_string();
    }
    format!("{}-{}.matti", crate::slugify(id), crate::slugify(version))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn parses_flat_catalog() {
        let json = br#"[
            {"id":"worldtools","name":"World Tools","version":"1.0","summary":"Tools",
             "download":"https://example.invalid/worldtools.matti","sha256":"abc"},
            {"name":"Render Tools","version":"2.0","url":"https://example.invalid/render.matti"}
        ]"#;
        let catalog = ModCatalog::from_json(json, "example/mods").unwrap();
        assert_eq!(catalog.mods.len(), 2);
        assert_eq!(catalog.mods[0].id, "render-tools");
        assert_eq!(catalog.mods[1].downloads.len(), 1);
        assert!(catalog.find("worldtools").is_some());
    }

    #[test]
    fn selects_platform_download() {
        let json = br#"{"mods":[{"id":"x","name":"X","version":"1","downloads":[
            {"platform":"linux","arch":"x86_64","url":"https://example.invalid/linux.matti","file":"x-linux.matti"},
            {"platform":"windows","arch":"x86_64","url":"https://example.invalid/win.matti","file":"x-win.matti"}
        ]}]}"#;
        let catalog = ModCatalog::from_json(json, "example/mods").unwrap();
        let entry = catalog.find("x").unwrap();
        assert_eq!(entry.download_for("linux", "x86_64").unwrap().file, "x-linux.matti");
        assert_eq!(entry.download_for("windows", "x86_64").unwrap().file, "x-win.matti");
    }

    #[test]
    fn rejects_invalid_root() {
        assert!(ModCatalog::from_json(b"{}", "example/mods").is_err());
    }
}
