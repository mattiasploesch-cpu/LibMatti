#!/usr/bin/env bash
# Imports a .matti mod package into the main repository.
#
# The package is the mod .so with a ".matti_pack" metadata section (see
# matti-pack.sh). The MDK embeds the mod's sources and CMake file as ELF
# sections ("matti_src/<path>" / "matti_cmake"), so the import restores:
#
#   mods/custom/<mod_id>/CMakeLists.txt   (the mod's build file)
#   mods/custom/<mod_id>/src/...          (the mod's sources)
#   mods/custom/<mod_id>/resources/...    (embedded resources, when present)
#
# and copies the runnable .so into mods/ as <modid>-<version>.so (FML loads
# it from there on the next runClient start).
#
# Usage: matti-import.sh <file.matti> [repo-root]
set -euo pipefail

if [ $# -lt 1 ]; then
    echo "usage: $0 <file.matti> [repo-root]" >&2
    exit 2
fi

file="$1"
root="${2:-.}"

tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT

# ---- read the pack metadata -----------------------------------------------
objcopy -O binary --only-section=.matti_pack "$file" "$tmp/pack" 2>/dev/null || true
if [ ! -s "$tmp/pack" ]; then
    echo "error: $file has no .matti_pack section (is it a packaged mod?)" >&2
    exit 1
fi

mod_id="$(sed -n 's/^Matti-Mod-Id: //p' "$tmp/pack")"
mod_version="$(sed -n 's/^Matti-Mod-Version: //p' "$tmp/pack")"
if [ -z "$mod_id" ] || [ -z "$mod_version" ]; then
    echo "error: $file pack metadata is incomplete" >&2
    exit 1
fi

# ---- restore the source tree ----------------------------------------------
list_file="$tmp/list"
objcopy -O binary --only-section=.matti_src_list "$file" "$list_file" 2>/dev/null || true

target="$root/mods/custom/$mod_id"
mkdir -p "$target/src"

if [ -s "$list_file" ]; then
    while IFS= read -r entry; do
        [ -n "$entry" ] || continue
        # The MDK embeds every source file under its own section name.
        objcopy -O binary --only-section="matti_src/$entry" "$file" "$tmp/src" 2>/dev/null || true
        if [ -s "$tmp/src" ]; then
            mkdir -p "$(dirname "$target/$entry")"
            cp "$tmp/src" "$target/$entry"
            rm -f "$tmp/src"
        fi
    done < "$list_file"
fi

# The CMake file (fallback: a generated one when the MDK did not embed it).
objcopy -O binary --only-section=matti_cmake "$file" "$tmp/cmake" 2>/dev/null || true
if [ -s "$tmp/cmake" ]; then
    mkdir -p "$target"
    cp "$tmp/cmake" "$target/CMakeLists.txt"
else
    cat > "$target/CMakeLists.txt" <<EOF
# Restored from the $mod_id-$mod_version.matti import (generated fallback).

matti_add_mod($mod_id
        MOD_ID $mod_id
        MOD_VERSION $mod_version
        SOURCES src/${mod_id}.c
        RESOURCES resources
)
EOF
fi

# ---- the runnable binary ---------------------------------------------------
so_name="$mod_id-$mod_version.so"
cp "$file" "$root/mods/$so_name"

echo "imported $file:"
echo "  source: mods/custom/$mod_id/"
echo "  binary: mods/$so_name"
