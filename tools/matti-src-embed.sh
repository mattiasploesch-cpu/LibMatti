#!/usr/bin/env bash
# Embeds a mod project's sources into the built .so as ELF sections so the
# main repository's import (tools/matti-import.sh) can restore the tree.
#
# Section layout:
#   matti_src_list          newline-separated relative paths of every source file
#   matti_src/<relpath>     the file content, one section per file
#   matti_cmake             the mod's CMakeLists.txt
#
# Usage: matti-src-embed.sh <mod.so> <mod-src-dir> <cmake-file>
set -euo pipefail

if [ $# -ne 3 ]; then
    echo "usage: $0 <mod.so> <mod-src-dir> <cmake-file>" >&2
    exit 2
fi

so="$1"
src_dir="$2"
cmake_file="$3"

tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT

cd "$src_dir"
find src resources -type f 2>/dev/null | sort > "$tmp/list"

objcopy --add-section "matti_src_list=$tmp/list" \
        --set-section-flags "matti_src_list=contents,readonly" \
        "$so"

while IFS= read -r entry; do
    [ -n "$entry" ] || continue
    objcopy --add-section "matti_src/$entry=$entry" \
            --set-section-flags "matti_src/$entry=contents,readonly" \
            "$so"
done < "$tmp/list"

objcopy --add-section "matti_cmake=$cmake_file" \
        --set-section-flags "matti_cmake=contents,readonly" \
        "$so"

echo "embedded $(wc -l < "$tmp/list") source file(s) + CMakeLists.txt into $so"
