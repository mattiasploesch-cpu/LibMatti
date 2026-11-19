#!/usr/bin/env bash
# Builds a single-file AppImage (the Linux "one file" artifact).
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT/build/linux-x64}"
VERSION="${VERSION:-0.1.0}"
ARCH="${ARCH:-$(uname -m)}"
DIST="${DIST:-$ROOT/dist}"
APPDIR="$(mktemp -d)/Matticraft.AppDir"
trap 'rm -rf "$(dirname "$APPDIR")"' EXIT

if [ ! -d "$BUILD_DIR" ]; then
    echo "missing build directory: $BUILD_DIR (configure with --preset linux-x64)" >&2
    exit 1
fi

APPIMAGETOOL="${APPIMAGETOOL:-}"
if [ -z "$APPIMAGETOOL" ]; then
    APPIMAGETOOL="$(command -v appimagetool || true)"
fi
if [ -z "$APPIMAGETOOL" ]; then
    echo "appimagetool not installed - cannot build the AppImage" >&2
    echo "set APPIMAGETOOL=/path/to/appimagetool or install appimagetool/linuxdeploy" >&2
    exit 1
fi

mkdir -p "$APPDIR/usr"
DESTDIR="$APPDIR" cmake --install "$BUILD_DIR" --prefix /usr

required=(
    usr/bin/matticraft-launcher
    usr/bin/matticraft
    usr/bin/runClient
    usr/bin/vulkanClient
    usr/bin/vulkancraft
    usr/share/matticraft/Monocraft.ttf
)
missing=()
for path in "${required[@]}"; do
    [ -e "$APPDIR/$path" ] || missing+=("$path")
done
if [ "${#missing[@]}" -ne 0 ]; then
    echo "refusing to build an incomplete AppImage:" >&2
    printf '  missing %s\n' "${missing[@]}" >&2
    exit 1
fi

cp "$ROOT/packaging/linux/matticraft.desktop" "$APPDIR/org.matticraft.Matticraft.desktop"
cp "$ROOT/packaging/linux/matticraft.svg" "$APPDIR/org.matticraft.Matticraft.svg"
ln -sf org.matticraft.Matticraft.svg "$APPDIR/.DirIcon"
ln -sf usr/bin/matticraft-launcher "$APPDIR/AppRun"
chmod +x "$APPDIR/AppRun"

mkdir -p "$DIST"
ARCH="$ARCH" "$APPIMAGETOOL" "$APPDIR" "$DIST/Matticraft-${VERSION}-${ARCH}.AppImage"

( cd "$DIST" && sha256sum ./*.AppImage > SHA256SUMS.appimage )
echo "AppImage written to $DIST"
