//! The launcher store: profiles, the selected account and global settings.
//!
//! `launcher.json` lives in the launcher data directory; every profile also
//! writes its own `profile.json` inside its instance directory, so a profile
//! can be copied between machines without the global index.

use std::path::{Path, PathBuf};

use serde::{Deserialize, Serialize};

use crate::account::Account;
use crate::profile::{Profile, PROFILE_SCHEMA};
use crate::{Error, Result};

pub const STORE_FILE: &str = "launcher.json";
pub const STORE_SCHEMA: u32 = 1;

#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct Settings {
    #[serde(default = "default_channel")]
    pub channel: String,
    #[serde(default)]
    pub update_manifest_url: Option<String>,
    #[serde(default)]
    pub crash_upload_url: Option<String>,
/// Explicit opt-in; crash reports stay local unless this is true.
    #[serde(default)]
    pub upload_crash_reports: bool,
    #[serde(default)]
    pub game_binary: Option<PathBuf>,
    #[serde(default)]
    pub close_on_launch: bool,
    #[serde(default = "default_catalog_repo")]
    pub catalog_repo: String,
    #[serde(default = "default_mdk_repo")]
    pub mdk_repo: String,
    #[serde(default)]
    pub last_server: String,
}

fn default_catalog_repo() -> String {
    crate::catalog::DEFAULT_CATALOG_REPO.to_string()
}

fn default_mdk_repo() -> String {
    crate::studio::MDK_REPOSITORY.to_string()
}

fn default_channel() -> String {
    "stable".to_string()
}

impl Default for Settings {
    fn default() -> Self {
        Settings {
            channel: default_channel(),
            update_manifest_url: None,
            crash_upload_url: None,
            upload_crash_reports: false,
            game_binary: None,
            close_on_launch: false,
            catalog_repo: default_catalog_repo(),
            mdk_repo: default_mdk_repo(),
            last_server: String::new(),
        }
    }
}

#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct Store {
    #[serde(default = "default_schema")]
    pub schema: u32,
    #[serde(default)]
    pub active: Option<String>,
    #[serde(default)]
    pub active_account: Option<String>,
    #[serde(default)]
    pub profiles: Vec<Profile>,
    #[serde(default)]
    pub accounts: Vec<Account>,
    #[serde(default)]
    pub settings: Settings,
    #[serde(skip)]
    root: PathBuf,
}

fn default_schema() -> u32 {
    STORE_SCHEMA
}

impl Store {
/// `MATTI_LAUNCHER_HOME`, else the platform data directory.
    pub fn default_root() -> Result<PathBuf> {
        if let Some(home) = std::env::var_os("MATTI_LAUNCHER_HOME") {
            return Ok(PathBuf::from(home));
        }
        if let Some(dirs) = directories::ProjectDirs::from("org", "Matticraft", "Matticraft") {
            return Ok(dirs.data_dir().to_path_buf());
        }
        if let Some(home) = directories::BaseDirs::new() {
            return Ok(home.home_dir().join(".matticraft"));
        }
        std::env::current_dir().map_err(Error::Io)
    }

/// Load the store from `root`, creating a first profile when the file does
    /// not exist yet.
    pub fn load_or_create(root: impl AsRef<Path>) -> Result<Self> {
        let root = prepare_root(root.as_ref())?;
        let path = root.join(STORE_FILE);
        if !path.exists() {
            let mut store = Store::empty(root);
            let selection = crate::platform::default_selection();
            let profile = Profile::new(&store.instances_dir(), "Default", "1.21.11", &selection)?;
            store.active = Some(profile.id.clone());
            store.profiles.push(profile);
            store.save()?;
            return Ok(store);
        }
        Store::load(root)
    }

    pub fn load(root: impl AsRef<Path>) -> Result<Self> {
        let root = prepare_root(root.as_ref())?;
        let path = root.join(STORE_FILE);
        let data = std::fs::read(&path).map_err(|error| {
            if error.kind() == std::io::ErrorKind::NotFound {
                Error::NotFound(path.display().to_string())
            } else {
                Error::Io(error)
            }
        })?;
        let mut store: Store = serde_json::from_slice(&data)?;
        store.root = root;
        store.migrate();
        store.validate()?;
        Ok(store)
    }

    pub fn empty(root: impl AsRef<Path>) -> Self {
        Store {
            schema: STORE_SCHEMA,
            active: None,
            active_account: None,
            profiles: Vec::new(),
            accounts: Vec::new(),
            settings: Settings::default(),
            root: root.as_ref().to_path_buf(),
        }
    }

    fn migrate(&mut self) {
        if self.schema < STORE_SCHEMA {
            self.schema = STORE_SCHEMA;
        }
        for profile in &mut self.profiles {
            if profile.schema < PROFILE_SCHEMA {
                profile.schema = PROFILE_SCHEMA;
            }
            profile.resolve_relative_paths(&self.root);
        }
    }

    pub fn validate(&self) -> Result<()> {
        let mut ids = std::collections::BTreeSet::new();
        for profile in &self.profiles {
            profile.validate()?;
            if !ids.insert(profile.id.clone()) {
                return Err(Error::Invalid(format!(
                    "duplicate profile id '{}'",
                    profile.id
                )));
            }
        }
        if let Some(active) = &self.active {
            if !self.profiles.iter().any(|profile| &profile.id == active) {
                return Err(Error::Invalid(format!(
                    "active profile '{active}' does not exist"
                )));
            }
        }
        if let Some(account) = &self.active_account {
            if !self.accounts.iter().any(|candidate| &candidate.id == account) {
                return Err(Error::Invalid(format!(
                    "active account '{account}' does not exist"
                )));
            }
        }
        Ok(())
    }

    pub fn root(&self) -> &Path {
        &self.root
    }

    pub fn instances_dir(&self) -> PathBuf {
        self.root.join("instances")
    }

    pub fn save(&mut self) -> Result<()> {
        if self.schema < STORE_SCHEMA {
            self.schema = STORE_SCHEMA;
        }
        self.validate()?;
        for profile in &self.profiles {
            profile.save()?;
        }
        crate::write_json_atomic(&self.root.join(STORE_FILE), self)
    }

    pub fn active_profile(&self) -> Option<&Profile> {
        let active = self.active.as_deref()?;
        self.profiles.iter().find(|profile| profile.id == active)
    }

    pub fn active_profile_mut(&mut self) -> Option<&mut Profile> {
        let active = self.active.clone()?;
        self.profiles.iter_mut().find(|profile| profile.id == active)
    }

    pub fn profile(&self, id: &str) -> Option<&Profile> {
        self.profiles.iter().find(|profile| profile.id == id)
    }

    pub fn set_active(&mut self, id: &str) -> Result<()> {
        if self.profile(id).is_none() {
            return Err(Error::NotFound(format!("profile '{id}'")));
        }
        self.active = Some(id.to_string());
        Ok(())
    }

/// Add a profile. Duplicate folder slugs get `-2`, `-3`, ... suffixes.
    pub fn add_profile(
        &mut self,
        name: &str,
        version: &str,
        selection: &str,
    ) -> Result<String> {
        let mut profile = Profile::new(&self.instances_dir(), name, version, selection)?;
        let base_id = profile.id.clone();
        let mut suffix = 2;
        while self.profile(&profile.id).is_some() {
            let id = format!("{base_id}-{suffix}");
            profile.id = id.clone();
            profile.instance_dir = self.instances_dir().join(&id);
            profile.game_dir = profile.instance_dir.clone();
            profile.mods_dir = profile.instance_dir.join("mods");
            profile.logs_dir = profile.instance_dir.join("logs");
            suffix += 1;
        }
        profile.ensure_dirs()?;
        let id = profile.id.clone();
        self.profiles.push(profile);
        if self.active.is_none() {
            self.active = Some(id.clone());
        }
        Ok(id)
    }

    pub fn remove_profile(&mut self, id: &str) -> Result<()> {
        if self.profiles.len() <= 1 {
            return Err(Error::Invalid("the last profile cannot be removed".into()));
        }
        let before = self.profiles.len();
        self.profiles.retain(|profile| profile.id != id);
        if self.profiles.len() == before {
            return Err(Error::NotFound(format!("profile '{id}'")));
        }
        if self.active.as_deref() == Some(id) {
            self.active = self.profiles.first().map(|profile| profile.id.clone());
        }
        Ok(())
    }

    pub fn duplicate_profile(&mut self, id: &str) -> Result<String> {
        let source = self
            .profile(id)
            .cloned()
            .ok_or_else(|| Error::NotFound(format!("profile '{id}'")))?;
        self.add_profile(
            &format!("{} copy", source.name),
            &source.game_version,
            &source.selection,
        )
    }

    pub fn active_account(&self) -> Option<&Account> {
        let active = self.active_account.as_deref()?;
        self.accounts.iter().find(|account| account.id == active)
    }

    pub fn set_active_account(&mut self, id: &str) -> Result<()> {
        if !self.accounts.iter().any(|account| account.id == id) {
            return Err(Error::NotFound(format!("account '{id}'")));
        }
        self.active_account = Some(id.to_string());
        Ok(())
    }

    pub fn add_account(&mut self, mut account: Account) -> Result<String> {
        account.validate()?;
        let base_id = account.id.clone();
        let mut suffix = 2;
        while self.accounts.iter().any(|candidate| candidate.id == account.id) {
            account.id = format!("{base_id}-{suffix}");
            suffix += 1;
        }
        let id = account.id.clone();
        self.accounts.push(account);
        if self.active_account.is_none() {
            self.active_account = Some(id.clone());
        }
        Ok(id)
    }

    pub fn remove_account(&mut self, id: &str) -> Result<()> {
        let before = self.accounts.len();
        self.accounts.retain(|account| account.id != id);
        if self.accounts.len() == before {
            return Err(Error::NotFound(format!("account '{id}'")));
        }
        if self.active_account.as_deref() == Some(id) {
            self.active_account = self.accounts.first().map(|account| account.id.clone());
        }
        Ok(())
    }
}

fn prepare_root(root: &Path) -> Result<PathBuf> {
    std::fs::create_dir_all(root)?;
    Ok(std::fs::canonicalize(root).unwrap_or_else(|_| root.to_path_buf()))
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::account::Account;

    #[test]
    fn first_start_creates_a_default_profile() {
        let root = crate::temp_dir("store-first");
        let store = Store::load_or_create(&root).unwrap();
        assert_eq!(store.profiles.len(), 1);
        assert_eq!(store.active_profile().unwrap().name, "Default");
        assert!(store.root().join(STORE_FILE).exists());
        assert!(store
            .active_profile()
            .unwrap()
            .instance_dir
            .join("profile.json")
            .exists());
        let _ = std::fs::remove_dir_all(root);
    }

    #[test]
    fn profiles_and_accounts_persist() {
        let root = crate::temp_dir("store-persist");
        let mut store = Store::load_or_create(&root).unwrap();
        let id = store.add_profile("Second", "1.21.11", "sdl:auto").unwrap();
        store.set_active(&id).unwrap();
        let account = store.add_account(Account::offline("Steve").unwrap()).unwrap();
        store.set_active_account(&account).unwrap();
        store.settings.channel = "beta".into();
        store.save().unwrap();

        let loaded = Store::load(&root).unwrap();
        assert_eq!(loaded.profiles.len(), 2);
        assert_eq!(loaded.active.as_deref(), Some(id.as_str()));
        assert_eq!(loaded.active_account.as_deref(), Some(account.as_str()));
        assert_eq!(loaded.settings.channel, "beta");
        let _ = std::fs::remove_dir_all(root);
    }

    #[test]
    fn duplicate_slugs_get_numeric_suffixes() {
        let root = crate::temp_dir("store-duplicate");
        let mut store = Store::load_or_create(&root).unwrap();
        let first = store.add_profile("Same", "1.21.11", "auto").unwrap();
        let second = store.add_profile("Same", "1.21.11", "auto").unwrap();
        assert_eq!(first, "same");
        assert_eq!(second, "same-2");
        let _ = std::fs::remove_dir_all(root);
    }

    #[test]
    fn last_profile_cannot_be_removed() {
        let root = crate::temp_dir("store-remove");
        let mut store = Store::load_or_create(&root).unwrap();
        let id = store.active.clone().unwrap();
        assert!(store.remove_profile(&id).is_err());
        let _ = std::fs::remove_dir_all(root);
    }
}
