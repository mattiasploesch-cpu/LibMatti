//! Release helper for the signed update manifest.
//!
//! ```text
//! matticraft-update-sign generate [--key update-signing-key.pk8] [--force]
//! matticraft-update-sign sign --key <pk8> --manifest <in.json> [--out <out.json>]
//! matticraft-update-sign verify --manifest <in.json> --public-key <hex>
//! ```
//!
//! The private key never belongs in the repository. Set the printed public key
//! as `MATTI_UPDATE_PUBLIC_KEY` when building the launcher.

use std::path::PathBuf;

use launcher_core::update::{self, UpdateManifest};

fn main() {
    std::process::exit(match run() {
        Ok(()) => 0,
        Err(error) => {
            eprintln!("matticraft-update-sign: {error}");
            2
        }
    });
}

fn run() -> Result<(), String> {
    let mut args = std::env::args().skip(1);
    let command = args.next().ok_or_else(usage)?;
    let options = Options::parse(args)?;
    match command.as_str() {
        "generate" | "keygen" => {
            if options.key.exists() && !options.force {
                return Err(format!(
                    "{} already exists (use --force to replace it)",
                    options.key.display()
                ));
            }
            let pair = update::generate_key_pair().map_err(|error| error.to_string())?;
            std::fs::write(&options.key, &pair.pkcs8)
                .map_err(|error| format!("write {}: {error}", options.key.display()))?;
            #[cfg(unix)]
            {
                use std::os::unix::fs::PermissionsExt;
                let _ = std::fs::set_permissions(
                    &options.key,
                    std::fs::Permissions::from_mode(0o600),
                );
            }
            println!("private key: {}", options.key.display());
            println!("MATTI_UPDATE_PUBLIC_KEY={}", pair.public_key_hex);
            Ok(())
        }
        "sign" => {
            let manifest_path = options
                .manifest
                .ok_or_else(|| "sign needs --manifest <in.json>".to_string())?;
            let output = options.output.unwrap_or_else(|| manifest_path.clone());
            let key = std::fs::read(&options.key)
                .map_err(|error| format!("read {}: {error}", options.key.display()))?;
            let mut manifest = read_manifest(&manifest_path)?;
            update::sign_manifest(&mut manifest, &key)
                .map_err(|error| error.to_string())?;
            let json = serde_json::to_vec_pretty(&manifest)
                .map_err(|error| format!("serialize manifest: {error}"))?;
            std::fs::write(&output, json)
                .map_err(|error| format!("write {}: {error}", output.display()))?;
            println!("signed {} -> {}", manifest_path.display(), output.display());
            Ok(())
        }
        "verify" => {
            let manifest_path = options
                .manifest
                .ok_or_else(|| "verify needs --manifest <in.json>".to_string())?;
            let key = options
                .public_key
                .or_else(|| {
                    std::env::var("MATTI_UPDATE_PUBLIC_KEY")
                        .ok()
                        .filter(|key| !key.trim().is_empty())
                })
                .ok_or_else(|| {
                    "verify needs --public-key <hex> or MATTI_UPDATE_PUBLIC_KEY".to_string()
                })?;
            let manifest = read_manifest(&manifest_path)?;
            manifest.verify(&key).map_err(|error| error.to_string())?;
            println!(
                "verified {} {} ({} artifact(s))",
                manifest.version,
                manifest.channel,
                manifest.artifacts.len()
            );
            Ok(())
        }
        "help" | "--help" | "-h" => {
            print!("{}", usage());
            Ok(())
        }
        other => Err(format!("unknown command '{other}'\n{}", usage())),
    }
}

fn read_manifest(path: &PathBuf) -> Result<UpdateManifest, String> {
    let bytes = std::fs::read(path)
        .map_err(|error| format!("read {}: {error}", path.display()))?;
    UpdateManifest::from_json(&bytes).map_err(|error| error.to_string())
}

struct Options {
    key: PathBuf,
    manifest: Option<PathBuf>,
    output: Option<PathBuf>,
    public_key: Option<String>,
    force: bool,
}

impl Options {
    fn parse(args: impl Iterator<Item = String>) -> Result<Self, String> {
        let mut options = Options {
            key: PathBuf::from("update-signing-key.pk8"),
            manifest: None,
            output: None,
            public_key: None,
            force: false,
        };
        let mut args = args.peekable();
        while let Some(argument) = args.next() {
            let (name, inline) = match argument.split_once('=') {
                Some((name, value)) => (name.to_string(), Some(value.to_string())),
                None => (argument, None),
            };
            let mut value = || {
                inline
                    .clone()
                    .or_else(|| args.next())
                    .ok_or_else(|| format!("{name} needs a value"))
            };
            match name.as_str() {
                "--key" => options.key = PathBuf::from(value()?),
                "--manifest" => options.manifest = Some(PathBuf::from(value()?)),
                "--out" | "--output" => options.output = Some(PathBuf::from(value()?)),
                "--public-key" => options.public_key = Some(value()?),
                "--force" => options.force = true,
                other => return Err(format!("unknown option '{other}'\n{}", usage())),
            }
        }
        Ok(options)
    }
}

fn usage() -> &'static str {
    "matticraft-update-sign - signed update manifests\n\
     \n\
     matticraft-update-sign generate [--key update-signing-key.pk8] [--force]\n\
     matticraft-update-sign sign --key <pk8> --manifest <in.json> [--out <out.json>]\n\
     matticraft-update-sign verify --manifest <in.json> --public-key <hex>\n"
}
