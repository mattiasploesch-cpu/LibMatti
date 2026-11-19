#!/usr/bin/env bash
# Builds a single-file AppImage (the Linux "one file" artifact).
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT/build/linux-x64}"
VERSION="${VERSION:-0.1.0}"
ARCH="${ARCH:-x86_64}"
DIST="${DIST:-$ROOT/dist}"
APPDIR="$(mktemp -d)/Matticraft.AppDir"
trap 'rm -rf "$(dirname "$APPDIR")"' EXIT

if ! command -v appimagetool >/dev/null 2>&1 && ! command -v linuxdeploy >/dev/null 2>&1; then
    echo "appimagetool/linuxdeploy not installed - skipping AppImage" >&2
    exit 0
fi

mkdir -p "$APPDIR/usr"
DESTDIR="$APPDIR" cmake --install "$BUILD_DIR" --prefix /usr
cp "$ROOT/packaging/linux/matticraft.desktop" "$APPDIR/matticraft.desktop"
cp "$ROOT/packaging/linux/matticraft.svg" "$APPDIR/matticraft.svg"
ln -sf usr/bin/matticraft-launcher "$APPDIR/AppRun"
chmod +x "$APPDIR/AppRun"

mkdir -p "$DIST"
if command -v appimagetool >/dev/null 2>&1; then
    ARCH="$ARCH" appimagetool "$APPDIR" "$DIST/Matticraft-${VERSION}-${ARCH}.AppImage"
else
    ARCH="$ARCH" linuxdeploy --appdir "$APPDIR" \
        --output appimage --desktop-file "$APPDIR/matticraft.desktop"
    mv ./*.AppImage "$DIST/" 2>/dev/null || true
fi

( cd "$DIST" && sha256sum ./*.AppImage > SHA256SUMS.appimage )
echo "AppImage written to $DIST"
