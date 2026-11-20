#!/usr/bin/env python3
"""Build the update manifest from a directory of release artifacts.

The launcher verifies the Ed25519 signature and the SHA-256 of every download.
This script maps the packaging matrix names to platform/arch/kind and writes
`update.json`; signing is a separate step:

    python3 tools/make_update_manifest.py --dist dist --version 0.2.0 \
        --channel stable --base-url https://example.invalid/releases/0.2.0
    matticraft-update-sign sign --key update.pk8 --manifest dist/update.json
"""
import argparse
import hashlib
import json
import os
import re
import sys

# kind, platform, arch matchers applied to the file name (first match wins).
RULES = [
    # Current release names: Matticraft-<version>-<platform>-<arch>.<ext>
    (re.compile(r"^Matticraft-Launcher-.+-linux-(?P<arch>x86_64|aarch64)\.tar\.zst$"), "linux", "launcher"),
    (re.compile(r"^Matticraft-Launcher-.+-windows-x64\.zip$"), "windows", "launcher"),
    (re.compile(r"^Matticraft-Launcher-.+-macos-(?P<arch>arm64|x86_64)\.zip$"), "macos", "launcher"),
    (re.compile(r"^Matticraft-.+-linux-(?P<arch>x86_64|aarch64)\.AppImage$"), "linux", "appimage"),
    (re.compile(r"^Matticraft-.+-linux-(?P<arch>x86_64|aarch64)\.deb$"), "linux", "deb"),
    (re.compile(r"^Matticraft-.+-linux-(?P<arch>x86_64|aarch64)\.rpm$"), "linux", "rpm"),
    (re.compile(r"^Matticraft-.+-linux-(?P<arch>x86_64|aarch64)\.tar\.zst$"), "linux", "tar.zst"),
    # The standalone SDK archive is intentionally not a game update artifact.
    (re.compile(r"^libmatti-.+-linux-(?P<arch>x86_64|aarch64)\.tar\.zst$"), "linux", "lib"),
    (re.compile(r"^Matticraft-.+-macos-(?P<arch>arm64|x86_64)\.dmg$"), "macos", "dmg"),
    (re.compile(r"^Matticraft-.+-macos-(?P<arch>arm64|x86_64)\.zip$"), "macos", "zip"),
    (re.compile(r"^Matticraft-.+-windows-(?P<arch>x64|x86|arm64)\.msi$"), "windows", "msi"),
    (re.compile(r"^Matticraft-.+-android\.apk$"), "android", "apk"),
    (re.compile(r"^Matticraft-.+-android\.aab$"), "android", "aab"),
    # Legacy names kept for older release directories.
    (re.compile(r"^matticraft_.+_(?P<arch>amd64|arm64|386|armhf)\.deb$"), "linux", "deb"),
    (re.compile(r"^matticraft-(?P<arch>[^.]+)\.rpm$"), "linux", "rpm"),
    (re.compile(r"^org\.matticraft\.Matticraft.*\.flatpak$"), "linux", "flatpak"),
    (re.compile(r"^matticraft-.+-linux-(?P<arch>[^.]+)\.tar\.zst$"), "linux", "tar.zst"),
    (re.compile(r"^matticraft-.+-freebsd-(?P<arch>[^.]+)\.tar\.zst$"), "freebsd", "tar.zst"),
    (re.compile(r"^Matticraft-.+-x64\.msi$"), "windows", "msi"),
    (re.compile(r"^Matticraft-.+-windows-(?P<arch>x64|x86|arm64)\.zip$"), "windows", "zip"),
    (re.compile(r"^Matticraft-.+-universal\.dmg$"), "macos", "dmg"),
    (re.compile(r"^Matticraft-.+-universal\.pkg$"), "macos", "pkg"),
    (re.compile(r"^Matticraft-.+-(?P<arch>arm64|x86_64)\.apk$"), "android", "apk"),
    (re.compile(r"^Matticraft-.+\.aab$"), "android", "aab"),
]

ARCH_ALIASES = {
    "amd64": "x86_64",
    "x64": "x86_64",
    "x86_64": "x86_64",
    "i386": "x86",
    "386": "x86",
    "x86": "x86",
    "arm64": "arm64",
    "aarch64": "arm64",
    "armhf": "arm",
    "universal": "any",
}

SKIP = {"SHA256SUMS", "update.json", "latest.json"}


def normalize_arch(arch):
    return ARCH_ALIASES.get(arch, arch or "any")


def sha256(path):
    digest = hashlib.sha256()
    with open(path, "rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def classify(name):
    for pattern, platform, kind in RULES:
        match = pattern.match(name)
        if match:
            arch = normalize_arch(match.groupdict().get("arch"))
            if kind == "msi":
                arch = "x86_64"
            return platform, arch, kind
    return None


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dist", default="dist")
    parser.add_argument("--version", required=True)
    parser.add_argument("--channel", default="stable")
    parser.add_argument("--base-url", default="")
    parser.add_argument("--notes", default="")
    parser.add_argument("--patch", default="", help="MATTICRAFTPATCH1 file for the game binary")
    parser.add_argument("--patch-platform", default="linux")
    parser.add_argument("--patch-arch", default="x86_64")
    parser.add_argument("--out", default="")
    args = parser.parse_args()

    if not os.path.isdir(args.dist):
        print(f"missing dist directory: {args.dist}", file=sys.stderr)
        return 1

    artifacts = []
    for name in sorted(os.listdir(args.dist)):
        if name in SKIP or name.startswith("."):
            continue
        path = os.path.join(args.dist, name)
        if not os.path.isfile(path):
            continue
        classified = classify(name)
        if not classified:
            continue
        platform, arch, kind = classified
        artifacts.append({
            "platform": platform,
            "arch": arch,
            "kind": kind,
            "name": name,
            "url": (args.base_url.rstrip("/") + "/" + name) if args.base_url else name,
            "sha256": sha256(path),
            "size": os.path.getsize(path),
        })

    patch = None
    if args.patch:
        with open(args.patch, "rb") as handle:
            magic = handle.read(16)
            if magic != b"MATTICRAFTPATCH1":
                print(f"not a MATTICRAFTPATCH1 file: {args.patch}", file=sys.stderr)
                return 1
            length = int.from_bytes(handle.read(4), "little")
            header = json.loads(handle.read(length).decode("utf-8"))
        patch_name = os.path.basename(args.patch)
        patch = {
            "platform": args.patch_platform,
            "arch": normalize_arch(args.patch_arch),
            "name": patch_name,
            "fromVersion": header.get("fromVersion", ""),
            "toVersion": header.get("toVersion", args.version.lstrip("v")),
            "url": (args.base_url.rstrip("/") + "/" + patch_name) if args.base_url else patch_name,
            "sha256": sha256(args.patch),
            "size": os.path.getsize(args.patch),
            "fromSha256": header["fromSha256"],
            "toSha256": header["toSha256"],
            "toSize": header["toSize"],
        }

    manifest = {
        "schema": 1,
        "version": args.version.lstrip("v"),
        "channel": args.channel,
        "released_at": os.environ.get("MATTI_RELEASED_AT", ""),
        "notes": args.notes,
        "artifacts": artifacts,
        "patch": patch,
        "signature": "",
    }

    output = args.out or os.path.join(args.dist, "update.json")
    with open(output, "w", encoding="utf-8") as handle:
        json.dump(manifest, handle, indent=2, sort_keys=False)
        handle.write("\n")
    print(f"wrote {output}: {len(artifacts)} artifact(s)")
    for artifact in artifacts:
        print(f"  {artifact['platform']}/{artifact['arch']}/{artifact['kind']}  {artifact['name']}")
    if patch:
        print(f"  patch {patch['fromVersion']} -> {patch['toVersion']}  {patch['name']}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
