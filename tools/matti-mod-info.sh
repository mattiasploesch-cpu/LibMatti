#!/usr/bin/env bash
# Reads the manifest of a packaged .matti mod (an ELF shared object with the
# ".matti_manifest" section, see mods/cmake/MattiMod.cmake).
#
# Usage: matti-mod-info.sh <file.matti>
# Prints KEY=VALUE lines: MOD_ID, MOD_VERSION, MOD_TYPE
set -euo pipefail

if [ $# -ne 1 ]; then
    echo "usage: $0 <file.matti>" >&2
    exit 2
fi

file="$1"
if [ ! -f "$file" ]; then
    echo "error: $file does not exist" >&2
    exit 2
fi

# The file must be an ELF shared object (the mod format is a plain .so).
if ! head -c 4 "$file" | grep -q $'\x7fELF'; then
    echo "error: $file is not an ELF object" >&2
    exit 1
fi

tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT

# objcopy dumps the manifest section; the section carries the same keys a
# META-INF/MANIFEST.MF would (Automatic-Module-Name, Implementation-Version,
# FMLModType).
if ! objcopy -O binary --only-section=.matti_manifest "$file" "$tmp/manifest" 2>/dev/null; then
    echo "error: $file has no .matti_manifest section" >&2
    exit 1
fi

if [ ! -s "$tmp/manifest" ]; then
    echo "error: $file has an empty .matti_manifest section" >&2
    exit 1
fi

mod_id=""
mod_version=""
mod_type="MOD"
while IFS= read -r line; do
    case "$line" in
        Automatic-Module-Name:*) mod_id="${line#Automatic-Module-Name: }" ;;
        Implementation-Version:*) mod_version="${line#Implementation-Version: }" ;;
        FMLModType:*) mod_type="${line#FMLModType: }" ;;
    esac
done < "$tmp/manifest"

if [ -z "$mod_id" ]; then
    echo "error: $file has no Automatic-Module-Name in its manifest" >&2
    exit 1
fi

echo "MOD_ID=$mod_id"
echo "MOD_VERSION=$mod_version"
echo "MOD_TYPE=$mod_type"
