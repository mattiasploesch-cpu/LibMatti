#!/usr/bin/env bash
# Launcher-only Linux archive. The game is deliberately not included: the
# launcher downloads the selected Matticraft version in its Updates tab.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT/build/linux-x64}"
VERSION="${VERSION:-0.1.0}"
ARCH="${ARCH:-$(uname -m)}"
DIST="${DIST:-$ROOT/dist}"
STAGE="$(mktemp -d)"
trap 'rm -rf "$STAGE"' EXIT

LAUNCHER_BIN="${LAUNCHER_BIN:-}"
if [ -z "$LAUNCHER_BIN" ]; then
    for candidate in \
        "$BUILD_DIR/launchers/matticraft-launcher" \
        "$ROOT/launchers/target/release/matticraft-launcher" \
        "$ROOT/launchers/target/debug/matticraft-launcher"; do
        if [ -x "$candidate" ]; then
            LAUNCHER_BIN="$candidate"
            break
        fi
    done
fi
if [ -z "$LAUNCHER_BIN" ] || [ ! -x "$LAUNCHER_BIN" ]; then
    echo "matticraft-launcher missing (build the launcher first)" >&2
    exit 1
fi

mkdir -p "$STAGE/usr/bin"
cp "$LAUNCHER_BIN" "$STAGE/usr/bin/matticraft-launcher"
mkdir -p "$STAGE/usr/share/applications" "$STAGE/usr/share/icons/hicolor/scalable/apps"
cp "$ROOT/packaging/linux/matticraft.desktop" "$STAGE/usr/share/applications/org.matticraft.Matticraft.desktop"
cp "$ROOT/packaging/linux/matticraft.svg" \
    "$STAGE/usr/share/icons/hicolor/scalable/apps/org.matticraft.Matticraft.svg"
mkdir -p "$DIST"
ARTIFACT="$DIST/Matticraft-Launcher-${VERSION}-linux-${ARCH}.tar.zst"
if tar --help 2>/dev/null | grep -q -- '--zstd'; then
    tar --zstd -cf "$ARTIFACT" -C "$STAGE" .
else
    tar -cf - -C "$STAGE" . | zstd -19 -o "$ARTIFACT"
fi
echo "launcher package: $ARTIFACT"
