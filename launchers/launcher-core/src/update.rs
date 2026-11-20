//! Update manifests: signed JSON, SHA-256 per artifact.
//!
//! The signature covers a canonical JSON document without the `signature`
//! field. The release workflow sets `MATTI_UPDATE_PUBLIC_KEY`, the launcher
//! embeds that public key and refuses a manifest that does not verify. Each
//! artifact additionally carries the SHA-256 of the exact download.

use std::fs::File;
use std::io::{Read, Seek, SeekFrom, Write};
use std::path::Path;

use flate2::read::ZlibDecoder;
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
#[serde(rename_all = "camelCase")]
pub struct Patch {
    pub platform: String,
    pub arch: String,
    #[serde(default)]
    pub name: String,
    pub from_version: String,
    pub to_version: String,
    pub url: String,
    pub sha256: String,
    #[serde(default)]
    pub size: u64,
    pub from_sha256: String,
    pub to_sha256: String,
    pub to_size: u64,
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
    pub patch: Option<Patch>,
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
    patch: &'a Option<Patch>,
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
            if !is_sha256(&artifact.sha256) {
                return Err(Error::Invalid(format!(
                    "artifact '{}' has an invalid SHA-256",
                    artifact.name
                )));
            }
        }
        if let Some(patch) = &manifest.patch {
            if !is_sha256(&patch.sha256)
                || !is_sha256(&patch.from_sha256)
                || !is_sha256(&patch.to_sha256)
            {
                return Err(Error::Invalid(
                    "update patch has an invalid SHA-256".into(),
                ));
            }
            if patch.to_size == 0 || patch.url.trim().is_empty() {
                return Err(Error::Invalid("update patch is incomplete".into()));
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
            patch: &self.patch,
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

    pub fn patch_for(&self, platform: &str, arch: &str) -> Option<&Patch> {
        self.patch.as_ref().filter(|patch| {
            (patch.platform == platform || patch.platform == "any")
                && (patch.arch == arch || patch.arch == "any")
        })
    }

    pub fn verify_patch(&self, path: &Path, patch: &Patch) -> Result<()> {
        if patch.size != 0 && std::fs::metadata(path)?.len() != patch.size {
            return Err(Error::Invalid(format!(
                "patch has {} bytes, manifest says {}",
                std::fs::metadata(path)?.len(),
                patch.size
            )));
        }
        verify_file_sha256(path, &patch.sha256, "update patch")
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
    fetch_bytes(location)
}

/// Read any update resource (manifest, patch or artifact) from disk or HTTP(S).
#[cfg(feature = "upload")]
pub fn fetch_bytes(location: &str) -> Result<Vec<u8>> {
    if let Some(path) = location.strip_prefix("file://") {
        return std::fs::read(path).map_err(Error::Io);
    }
    if !location.starts_with("http://") && !location.starts_with("https://") {
        return std::fs::read(location).map_err(Error::Io);
    }
    let response = ureq::get(location)
        .timeout(std::time::Duration::from_secs(60))
        .call()
        .map_err(|error| Error::Invalid(format!("update download failed: {error}")))?;
    let mut bytes = Vec::new();
    response
        .into_reader()
        .read_to_end(&mut bytes)
        .map_err(Error::Io)?;
    Ok(bytes)
}

/// Stream a download to a file. Used for game archives, which are much larger
/// than the in-memory manifest/patch path.
#[cfg(feature = "upload")]
pub fn download_to_file(location: &str, target: &Path) -> Result<()> {
    if let Some(parent) = target.parent() {
        std::fs::create_dir_all(parent)?;
    }
    if let Some(path) = location.strip_prefix("file://") {
        std::fs::copy(path, target)?;
        return Ok(());
    }
    if !location.starts_with("http://") && !location.starts_with("https://") {
        std::fs::copy(location, target)?;
        return Ok(());
    }
    let temp = target.with_extension("part");
    let _ = std::fs::remove_file(&temp);
    let response = ureq::get(location)
        .timeout(std::time::Duration::from_secs(300))
        .call()
        .map_err(|error| Error::Invalid(format!("game download failed: {error}")))?;
    let mut reader = response.into_reader();
    let mut file = File::create(&temp)?;
    std::io::copy(&mut reader, &mut file)?;
    file.flush()?;
    std::fs::rename(&temp, target)?;
    Ok(())
}

pub const PATCH_MAGIC: &[u8; 16] = b"MATTICRAFTPATCH1";

#[derive(Deserialize)]
#[serde(rename_all = "camelCase")]
struct PatchHeader {
    from_version: String,
    to_version: String,
    from_sha256: String,
    to_sha256: String,
    to_size: u64,
}

pub fn is_sha256(value: &str) -> bool {
    value.len() == 64 && value.chars().all(|ch| ch.is_ascii_hexdigit())
}

pub fn verify_file_sha256(path: &Path, expected: &str, label: &str) -> Result<()> {
    let actual = crate::mods::sha256_file(path)?;
    if !actual.eq_ignore_ascii_case(expected) {
        return Err(Error::Invalid(format!(
            "{label} SHA-256 mismatch (expected {expected}, got {actual})"
        )));
    }
    Ok(())
}

/// Apply a `MATTICRAFTPATCH1` delta to `base`, writing the result to `output`.
///
/// The operation stream is zlib-compressed. COPY instructions refer to byte
/// offsets in the base binary; ADD instructions carry new bytes. The base and
/// result are both checked against the SHA-256 values in the patch header.
pub fn apply_file_patch(base: &Path, patch_path: &Path, output: &Path) -> Result<()> {
    let base_len = std::fs::metadata(base)?.len();
    let base_hash = crate::mods::sha256_file(base)?;

    let mut patch_file = std::io::BufReader::new(File::open(patch_path)?);
    let mut magic = [0u8; 16];
    patch_file.read_exact(&mut magic)?;
    if &magic != PATCH_MAGIC {
        return Err(Error::Invalid("unknown update patch format".into()));
    }
    let mut length_bytes = [0u8; 4];
    patch_file.read_exact(&mut length_bytes)?;
    let header_len = u32::from_le_bytes(length_bytes) as usize;
    if header_len == 0 || header_len > 64 * 1024 {
        return Err(Error::Invalid("invalid update patch header".into()));
    }
    let mut header_bytes = vec![0u8; header_len];
    patch_file.read_exact(&mut header_bytes)?;
    let header: PatchHeader = serde_json::from_slice(&header_bytes)?;
    if !header.from_sha256.eq_ignore_ascii_case(&base_hash) {
        return Err(Error::Invalid(format!(
            "installed game does not match patch {} → {} (expected {}, got {})",
            header.from_version, header.to_version, header.from_sha256, base_hash
        )));
    }
    if header.to_size == 0 || !is_sha256(&header.to_sha256) {
        return Err(Error::Invalid("update patch header is incomplete".into()));
    }

    if let Some(parent) = output.parent() {
        std::fs::create_dir_all(parent)?;
    }
    let _ = std::fs::remove_file(output);
    let mut out = File::create(output)?;
    let mut base_file = File::open(base)?;
    let mut ops = ZlibDecoder::new(patch_file);

    loop {
        let mut opcode = [0u8; 1];
        match ops.read(&mut opcode) {
            Ok(0) => break,
            Ok(_) => {}
            Err(error) => {
                let _ = std::fs::remove_file(output);
                return Err(Error::Io(error));
            }
        }
        let result = match opcode[0] {
            0x01 => {
                let offset = read_u64(&mut ops)?;
                let length = read_u64(&mut ops)?;
                if offset.checked_add(length).map_or(true, |end| end > base_len) {
                    Err(Error::Invalid("patch COPY lies outside the installed game".into()))
                } else {
                    base_file.seek(SeekFrom::Start(offset))?;
                    let copied = std::io::copy(&mut Read::by_ref(&mut base_file).take(length), &mut out)?;
                    if copied == length {
                        Ok(())
                    } else {
                        Err(Error::Invalid("patch COPY ended too early".into()))
                    }
                }
            }
            0x02 => {
                let length = read_u64(&mut ops)?;
                let copied = std::io::copy(&mut Read::by_ref(&mut ops).take(length), &mut out)?;
                if copied == length {
                    Ok(())
                } else {
                    Err(Error::Invalid("patch ADD ended too early".into()))
                }
            }
            other => Err(Error::Invalid(format!("unknown patch opcode {other}"))),
        };
        if let Err(error) = result {
            let _ = std::fs::remove_file(output);
            return Err(error);
        }
    }
    out.flush()?;
    drop(out);

    let size = std::fs::metadata(output)?.len();
    if size != header.to_size {
        let _ = std::fs::remove_file(output);
        return Err(Error::Invalid(format!(
            "patched game has {size} bytes, expected {}",
            header.to_size
        )));
    }
    if let Err(error) = verify_file_sha256(output, &header.to_sha256, "patched game") {
        let _ = std::fs::remove_file(output);
        return Err(error);
    }
    Ok(())
}

fn read_u64(input: &mut impl Read) -> Result<u64> {
    let mut bytes = [0u8; 8];
    input.read_exact(&mut bytes)?;
    Ok(u64::from_le_bytes(bytes))
}

/// Apply a patch to an installed executable atomically. The old binary is kept
/// as `<name>.matti-backup` until the verified replacement is in place.
pub fn apply_patch_to_file(base: &Path, patch_path: &Path) -> Result<()> {
    let temp = base.with_extension("matti-new");
    apply_file_patch(base, patch_path, &temp)?;
    if let Ok(metadata) = std::fs::metadata(base) {
        let _ = std::fs::set_permissions(&temp, metadata.permissions());
    }
    let backup = base.with_extension("matti-backup");
    let _ = std::fs::remove_file(&backup);
    std::fs::rename(base, &backup)?;
    if let Err(error) = std::fs::rename(&temp, base) {
        let _ = std::fs::rename(&backup, base);
        let _ = std::fs::remove_file(&temp);
        return Err(Error::Io(error));
    }
    Ok(())
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
            patch: None,
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
    fn binary_patch_applies_and_rejects_a_different_base() {
        use flate2::write::ZlibEncoder;
        use flate2::Compression;

        let dir = crate::temp_dir("binary-patch");
        let base = dir.join("matticraft");
        let other = dir.join("other");
        let patch = dir.join("update.patch");
        let output = dir.join("matticraft.new");
        let old_bytes = b"hello old";
        let new_bytes = b"hello new";
        std::fs::write(&base, old_bytes).unwrap();
        std::fs::write(&other, b"different").unwrap();

        let header = serde_json::json!({
            "fromVersion": "1.0.0",
            "toVersion": "1.0.1",
            "fromSha256": crate::mods::sha256_file(&base).unwrap(),
            "toSha256": hex::encode(ring::digest::digest(&ring::digest::SHA256, new_bytes)),
            "toSize": new_bytes.len(),
        });
        let header = serde_json::to_vec(&header).unwrap();

        let mut encoder = ZlibEncoder::new(Vec::new(), Compression::best());
        encoder.write_all(&[0x01]).unwrap();
        encoder.write_all(&0u64.to_le_bytes()).unwrap();
        encoder.write_all(&5u64.to_le_bytes()).unwrap();
        encoder.write_all(&[0x02]).unwrap();
        encoder.write_all(&4u64.to_le_bytes()).unwrap();
        encoder.write_all(b" new").unwrap();
        let encoded = encoder.finish().unwrap();

        let mut bytes = Vec::new();
        bytes.extend_from_slice(PATCH_MAGIC);
        bytes.extend_from_slice(&(header.len() as u32).to_le_bytes());
        bytes.extend_from_slice(&header);
        bytes.extend_from_slice(&encoded);
        std::fs::write(&patch, bytes).unwrap();

        apply_file_patch(&base, &patch, &output).unwrap();
        assert_eq!(std::fs::read(&output).unwrap(), new_bytes);
        assert!(apply_file_patch(&other, &patch, &output).is_err());
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
