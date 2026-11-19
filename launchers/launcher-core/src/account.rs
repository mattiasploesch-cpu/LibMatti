//! Accounts stored next to the profiles.
//!
//! Offline accounts carry only a player name; the C client derives the same
//! offline UUID from it as Java does. Microsoft accounts keep the token fields
//! the launcher received from its auth flow. Tokens are never written to the
//! game log.

use serde::{Deserialize, Serialize};

use crate::{slugify, Error, Result};

#[derive(Clone, Copy, Debug, Default, Eq, PartialEq, Serialize, Deserialize)]
#[serde(rename_all = "snake_case")]
pub enum AccountKind {
    #[default]
    Offline,
    Microsoft,
}

#[derive(Clone, Eq, PartialEq, Serialize, Deserialize)]
pub struct Account {
    #[serde(default)]
    pub id: String,
    pub username: String,
    #[serde(default)]
    pub kind: AccountKind,
    #[serde(default)]
    pub uuid: Option<String>,
/// Kept out of `Debug` output so a crash report cannot leak it.
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub access_token: Option<String>,
}

impl std::fmt::Debug for Account {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("Account")
            .field("id", &self.id)
            .field("username", &self.username)
            .field("kind", &self.kind)
            .field("uuid", &self.uuid)
            .field("access_token", &self.access_token.as_ref().map(|_| "<redacted>"))
            .finish()
    }
}

impl Account {
    pub fn offline(username: &str) -> Result<Self> {
        let username = username.trim();
        validate_username(username)?;
        Ok(Account {
            id: slugify(username),
            username: username.to_string(),
            kind: AccountKind::Offline,
            uuid: None,
            access_token: None,
        })
    }

    pub fn microsoft(
        username: &str,
        uuid: Option<String>,
        access_token: Option<String>,
    ) -> Result<Self> {
        let username = username.trim();
        validate_username(username)?;
        Ok(Account {
            id: format!("{}-msa", slugify(username)),
            username: username.to_string(),
            kind: AccountKind::Microsoft,
            uuid,
            access_token,
        })
    }

    pub fn validate(&self) -> Result<()> {
        validate_username(self.username.trim())?;
        if self.kind == AccountKind::Microsoft && self.access_token.is_none() {
            return Err(Error::Invalid(format!(
                "Microsoft account '{}' has no access token",
                self.username
            )));
        }
        Ok(())
    }

/// Arguments handed to the C client. The game derives an offline UUID when
    /// `--uuid` is absent.
    pub fn game_arguments(&self) -> Vec<String> {
        let mut args = vec![
            "--username".to_string(),
            self.username.clone(),
            "--accessToken".to_string(),
            self.access_token.clone().unwrap_or_else(|| "0".to_string()),
        ];
        if let Some(uuid) = &self.uuid {
            args.push("--uuid".to_string());
            args.push(uuid.clone());
        }
        args
    }
}

pub fn validate_username(username: &str) -> Result<()> {
    if !(3..=16).contains(&username.chars().count()) {
        return Err(Error::Invalid(
            "player name must be 3 to 16 characters long".into(),
        ));
    }
    if !username
        .chars()
        .all(|ch| ch.is_ascii_alphanumeric() || ch == '_')
    {
        return Err(Error::Invalid(
            "player name may only contain letters, digits and '_'".into(),
        ));
    }
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn offline_account_roundtrip() {
        let account = Account::offline("Steve").unwrap();
        assert_eq!(account.kind, AccountKind::Offline);
        assert_eq!(account.id, "steve");
        let json = serde_json::to_string(&account).unwrap();
        let loaded: Account = serde_json::from_str(&json).unwrap();
        assert_eq!(loaded, account);
        assert_eq!(
            account.game_arguments(),
            vec!["--username", "Steve", "--accessToken", "0"]
        );
    }

    #[test]
    fn invalid_names_are_rejected() {
        assert!(Account::offline("ab").is_err());
        assert!(Account::offline("this-name-is-too-long").is_err());
        assert!(Account::offline("bad name").is_err());
        assert!(Account::offline("Steve_1").is_ok());
    }

    #[test]
    fn token_is_redacted_from_debug() {
        let account = Account::microsoft("Alex", None, Some("secret".into())).unwrap();
        let debug = format!("{account:?}");
        assert!(debug.contains("<redacted>"));
        assert!(!debug.contains("secret"));
    }
}
