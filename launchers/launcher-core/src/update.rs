//! Update manifests: signed JSON, SHA-256 per artifact.
//!
//! The signature covers a canonical JSON document without the `signature`
//! field. The release workflow sets `MATTI_UPDATE_PUBLIC_KEY`, the launcher
//! embeds that public key and refuses a manifest that does not verify. Each
//! artifact additionally carries the SHA-256 of the exact download.

use std::path::Path;

use ring::signature::{Ed25519KeyPair, KeyPair, UnparsedPublicKey, ED25519};
use serde::{Deserialize, Serialize};

use crate::{Error, Result};

pub const UPDATE_SCHEMA: u32 = 1;

#[derive(Clone, Debug, Serialize, Deserialize, PartialEq, Eq)]
pub struct Artifact {
/// `linux`, `windows`, `macos`, `android`, `freebsd` or `any`.
    pub platform: String,
/// `x86_64`, `x86`, `arm64`, `arm` or `any`.
    pub arch: String,
/// `tar.zst`, `deb`, `rpm`, `appimage`, `flatpak`, `msi`, `zip`, `dmg`,
    /// `pkg`, `apk` or `aab`.
    pub kind: String,
    pub name: String,
    pub url: String,
    pub sha256: String,
    #[serde(default)]
    pub size: u64,
}

#[derive(Clone, Debug, Serialize, Deserialize, PartialEq, Eq)]
pub struct UpdateManifest {
    #[serde(default = "default_schema")]
    pub schema: u32,
    pub version: String,
    pub channel: String,
    #[serde(default)]
    pub released_at: String,
    #[serde(default)]
    pub notes: String,
    #[serde(default)]
    pub artifacts: Vec<Artifact>,
    #[serde(default)]
    pub signature: String,
}

fn default_schema() -> u32 {
    UPDATE_SCHEMA
}

#[derive(Serialize)]
struct Unsigned<'a> {
    schema: u32,
    version: &'a str,
    channel: &'a str,
    released_at: &'a str,
    notes: &'a str,
    artifacts: &'a [Artifact],
}

impl UpdateManifest {
    pub fn from_json(bytes: &[u8]) -> Result<Self> {
        let manifest: UpdateManifest = serde_json::from_slice(bytes)?;
        if manifest.schema > UPDATE_SCHEMA {
            return Err(Error::Unsupported(format!(
                "update manifest schema {} is newer than {UPDATE_SCHEMA}",
                manifest.schema
            )));
        }
        if manifest.version.trim().is_empty() {
            return Err(Error::Invalid("update manifest has no version".into()));
        }
        for artifact in &manifest.artifacts {
            if artifact.sha256.len() != 64
                || !artifact.sha256.chars().all(|ch| ch.is_ascii_hexdigit())
            {
                return Err(Error::Invalid(format!(
                    "artifact '{}' has an invalid SHA-256",
                    artifact.name
                )));
            }
        }
        Ok(manifest)
    }

/// Canonical bytes the signature is computed over.
    pub fn unsigned_bytes(&self) -> Vec<u8> {
        serde_json::to_vec(&Unsigned {
            schema: self.schema,
            version: &self.version,
            channel: &self.channel,
            released_at: &self.released_at,
            notes: &self.notes,
            artifacts: &self.artifacts,
        })
        .unwrap_or_default()
    }

    pub fn sign(&mut self, key_pair: &Ed25519KeyPair) {
        self.signature = hex::encode(key_pair.sign(&self.unsigned_bytes()).as_ref());
    }

    pub fn verify(&self, public_key_hex: &str) -> Result<()> {
        let key = hex::decode(public_key_hex.trim())
            .map_err(|_| Error::Signature("public key is not valid hex".into()))?;
        if key.len() != 32 {
            return Err(Error::Signature(format!(
                "ed25519 public key must be 32 bytes, got {}",
                key.len()
            )));
        }
        let signature = hex::decode(self.signature.trim())
            .map_err(|_| Error::Signature("signature is not valid hex".into()))?;
        UnparsedPublicKey::new(&ED25519, key)
            .verify(&self.unsigned_bytes(), &signature)
            .map_err(|_| Error::Signature("update manifest signature does not verify".into()))
    }

/// Verify with the public key compiled into the launcher.
    pub fn verify_release(&self) -> Result<()> {
        let key = release_public_key().ok_or_else(|| {
            Error::Signature(
                "no MATTI_UPDATE_PUBLIC_KEY was compiled into this launcher".into(),
            )
        })?;
        self.verify(&key)
    }

    pub fn artifacts_for(&self, platform: &str, arch: &str) -> Vec<&Artifact> {
        self.artifacts
            .iter()
            .filter(|artifact| {
                (artifact.platform == platform || artifact.platform == "any")
                    && (artifact.arch == arch || artifact.arch == "any")
            })
            .collect()
    }

    pub fn artifact(&self, platform: &str, arch: &str, kind: &str) -> Option<&Artifact> {
        self.artifacts_for(platform, arch)
            .into_iter()
            .find(|artifact| artifact.kind == kind)
    }

    pub fn verify_artifact(&self, path: &Path, artifact: &Artifact) -> Result<()> {
        let metadata = std::fs::metadata(path)?;
        if artifact.size != 0 && metadata.len() != artifact.size {
            return Err(Error::Invalid(format!(
                "artifact '{}' has {} bytes, manifest says {}",
                artifact.name,
                metadata.len(),
                artifact.size
            )));
        }
        let actual = crate::mods::sha256_file(path)?;
        if !actual.eq_ignore_ascii_case(&artifact.sha256) {
            return Err(Error::Invalid(format!(
                "artifact '{}' SHA-256 mismatch (expected {}, got {})",
                artifact.name, artifact.sha256, actual
            )));
        }
        Ok(())
    }
}

/// The public key embedded at build time. Release builds set
/// `MATTI_UPDATE_PUBLIC_KEY` to the hex-encoded Ed25519 public key.
pub fn release_public_key() -> Option<String> {
    option_env!("MATTI_UPDATE_PUBLIC_KEY")
        .map(str::trim)
        .filter(|key| !key.is_empty())
        .map(ToString::to_string)
}

/// Read an update manifest from a local path, `file://` URL or HTTP(S) URL.
#[cfg(feature = "upload")]
pub fn fetch_manifest(location: &str) -> Result<Vec<u8>> {
    if let Some(path) = location.strip_prefix("file://") {
        return std::fs::read(path).map_err(Error::Io);
    }
    if !location.starts_with("http://") && !location.starts_with("https://") {
        return std::fs::read(location).map_err(Error::Io);
    }
    let response = ureq::get(location)
        .timeout(std::time::Duration::from_secs(30))
        .call()
        .map_err(|error| Error::Invalid(format!("update check failed: {error}")))?;
    let mut bytes = Vec::new();
    use std::io::Read;
    response
        .into_reader()
        .read_to_end(&mut bytes)
        .map_err(Error::Io)?;
    Ok(bytes)
}

pub struct GeneratedKeyPair {
/// PKCS#8 DER, the format `ring::signature::Ed25519KeyPair::from_pkcs8`
    /// expects. Keep this file private.
    pub pkcs8: Vec<u8>,
    pub public_key_hex: String,
}

pub fn generate_key_pair() -> Result<GeneratedKeyPair> {
    let rng = ring::rand::SystemRandom::new();
    let pkcs8 = Ed25519KeyPair::generate_pkcs8(&rng)
        .map_err(|_| Error::Signature("could not generate Ed25519 key".into()))?;
    let pair = Ed25519KeyPair::from_pkcs8(pkcs8.as_ref())
        .map_err(|_| Error::Signature("could not read generated Ed25519 key".into()))?;
    Ok(GeneratedKeyPair {
        pkcs8: pkcs8.as_ref().to_vec(),
        public_key_hex: hex::encode(pair.public_key().as_ref()),
    })
}

pub fn sign_manifest(manifest: &mut UpdateManifest, pkcs8: &[u8]) -> Result<()> {
    let pair = Ed25519KeyPair::from_pkcs8(pkcs8)
        .map_err(|_| Error::Signature("update signing key is not a PKCS#8 Ed25519 key".into()))?;
    manifest.sign(&pair);
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    fn manifest() -> UpdateManifest {
        UpdateManifest {
            schema: UPDATE_SCHEMA,
            version: "0.2.0".into(),
            channel: "stable".into(),
            released_at: "2026-11-19T00:00:00Z".into(),
            notes: "release".into(),
            artifacts: vec![Artifact {
                platform: "linux".into(),
                arch: "x86_64".into(),
                kind: "tar.zst".into(),
                name: "matticraft-0.2.0-linux-x86_64.tar.zst".into(),
                url: "https://example.invalid/file".into(),
                sha256: "a".repeat(64),
                size: 0,
            }],
            signature: String::new(),
        }
    }

    #[test]
    fn signed_manifest_verifies_and_tampering_fails() {
        let keys = generate_key_pair().unwrap();
        let mut manifest = manifest();
        manifest.sign_manifest(&keys.pkcs8).unwrap();
        manifest.verify(&keys.public_key_hex).unwrap();

        let mut tampered = manifest.clone();
        tampered.version = "9.9.9".into();
        assert!(tampered.verify(&keys.public_key_hex).is_err());

        let mut bad_signature = manifest.clone();
        bad_signature.signature = "00".repeat(64);
        assert!(bad_signature.verify(&keys.public_key_hex).is_err());
    }

    #[test]
    fn artifact_selection_matches_arch_and_any() {
        let mut manifest = manifest();
        manifest.artifacts.push(Artifact {
            platform: "linux".into(),
            arch: "any".into(),
            kind: "flatpak".into(),
            name: "flatpak".into(),
            url: "https://example.invalid/flatpak".into(),
            sha256: "b".repeat(64),
            size: 0,
        });
        assert_eq!(manifest.artifacts_for("linux", "x86_64").len(), 2);
        assert_eq!(
            manifest.artifact("linux", "x86_64", "tar.zst").unwrap().name,
            "matticraft-0.2.0-linux-x86_64.tar.zst"
        );
        assert!(manifest.artifact("windows", "x86_64", "msi").is_none());
    }

    #[test]
    fn artifact_hash_is_checked() {
        let dir = crate::temp_dir("update");
        let path = dir.join("artifact");
        std::fs::write(&path, b"abc").unwrap();
        let mut artifact = manifest().artifacts.remove(0);
        artifact.sha256 =
            "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad".into();
        artifact.size = 3;
        let manifest = UpdateManifest {
            artifacts: vec![artifact.clone()],
            ..manifest()
        };
        manifest.verify_artifact(&path, &artifact).unwrap();
        artifact.size = 4;
        assert!(manifest.verify_artifact(&path, &artifact).is_err());
        let _ = std::fs::remove_dir_all(dir);
    }

    #[test]
    fn json_roundtrip_is_stable() {
        let mut manifest = manifest();
        manifest.signature = "ab".repeat(64);
        let bytes = serde_json::to_vec(&manifest).unwrap();
        let parsed = UpdateManifest::from_json(&bytes).unwrap();
        assert_eq!(parsed, manifest);
    }
}



impl UpdateManifest {
    pub fn sign_manifest(&mut self, pkcs8: &[u8]) -> Result<()> {
        sign_manifest(self, pkcs8)
    }
}
