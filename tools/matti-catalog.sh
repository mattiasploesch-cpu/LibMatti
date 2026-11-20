#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT="${1:-$ROOT/mods/mods.json}"
TMP="$(mktemp)"
trap 'rm -f "$TMP"' EXIT

printf '{\n  "schema": 1,\n  "name": "Matticraft mods",\n  "repository": "mattiasploesch-cpu/LibMatti",\n  "updated": "%s",\n  "mods": [\n' "$(date -u +%Y-%m-%dT%H:%M:%SZ)" > "$TMP"

first=1
for metadata in "$ROOT"/mods/*/metadata.matti "$ROOT"/mods/custom/*/metadata.matti; do
    [ -f "$metadata" ] || continue
    case "$metadata" in
        "$ROOT"/mods/custom/*/metadata.matti) ;;
        "$ROOT"/mods/custom/*) continue ;;
    esac
    [ "$first" -eq 1 ] || printf ',\n' >> "$TMP"
    first=0
    python3 - "$metadata" >> "$TMP" <<'PY'
import json
import os
import sys

path = sys.argv[1]
with open(path, encoding="utf-8") as handle:
    data = json.load(handle)
mod_id = data["id"]
version = data["version"]
entry = {
    "id": mod_id,
    "name": data.get("name", mod_id),
    "version": version,
    "author": data.get("author", ""),
    "summary": data.get("description", ""),
    "description": data.get("description", ""),
    "license": data.get("license", ""),
    "homepage": data.get("homepage", ""),
    "game_versions": data.get("game_versions", []),
    "tags": data.get("tags", []),
    "file": f"{mod_id}-{version}.matti",
    "url": f"https://github.com/mattiasploesch-cpu/LibMatti/releases/latest/download/{mod_id}-{version}.matti",
    "downloads": [
        {
            "platform": "any",
            "arch": "any",
            "file": f"{mod_id}-{version}.matti",
            "url": f"https://github.com/mattiasploesch-cpu/LibMatti/releases/latest/download/{mod_id}-{version}.matti",
        }
    ],
}
print(json.dumps(entry, ensure_ascii=False, indent=2), end="")
PY
done

printf '\n  ]\n}\n' >> "$TMP"
python3 -m json.tool "$TMP" > "$OUT"
echo "catalog: $OUT"
