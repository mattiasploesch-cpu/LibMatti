#!/usr/bin/env bash
# Packs a built mod .so into the distributable .matti package.
#
# A .matti file is the mod shared object plus a ".matti_pack" ELF section that
# carries the package metadata. The validate/import workflows and the import
# tool in the MDK read it. The ELF .so itself is byte-identical to what FML
# loads - unpacking on the main repo side means stripping nothing, the file
# *is* the mod.
#
# Usage: matti-pack.sh <mod.so> <out.matti> <mod-id> <version>
set -euo pipefail

if [ $# -ne 4 ]; then
    echo "usage: $0 <mod.so> <out.matti> <mod-id> <version>" >&2
    exit 2
fi

mod_so="$1"
out="$2"
mod_id="$3"
version="$4"

if [ ! -f "$mod_so" ]; then
    echo "error: $mod_so does not exist" >&2
    exit 1
fi

tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT

printf 'Matti-Pack-Format: 1\nMatti-Mod-Id: %s\nMatti-Mod-Version: %s\n\n' \
    "$mod_id" "$version" > "$tmp/pack"

cp "$mod_so" "$out"
objcopy --add-section ".matti_pack=$tmp/pack" \
        --set-section-flags ".matti_pack=contents,readonly" \
        "$out"

echo "packed $out ($(stat -c%s "$out") bytes, mod: $mod_id $version)"
