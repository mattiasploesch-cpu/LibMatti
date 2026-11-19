#!/usr/bin/env bash
# Builds an (unsigned) macOS .app and .dmg on Linux from the osxcross build.
# Code signing/notarization still requires an Apple machine, so the artifacts
# from this script are explicitly unsigned.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT/build/macos-osxcross}"
VERSION="${VERSION:-0.1.0}"
ARCH="${ARCH:-$(uname -m)}"
DIST="${DIST:-$ROOT/dist}"
STAGE="$(mktemp -d)"
trap 'rm -rf "$STAGE"' EXIT

if [ ! -d "$BUILD_DIR" ]; then
    echo "missing $BUILD_DIR - run the osxcross build first" >&2
    exit 1
fi

DESTDIR="$STAGE" cmake --install "$BUILD_DIR" --prefix /usr
APP="$DIST/Matticraft.app"
rm -rf "$APP"
mkdir -p "$APP/Contents/MacOS" "$APP/Contents/Frameworks" "$APP/Contents/Resources"
if [ -x "$STAGE/usr/bin/matticraft-launcher" ]; then
    cp "$STAGE/usr/bin/matticraft-launcher" "$APP/Contents/MacOS/Matticraft"
    cp "$STAGE/usr/bin/vulkancraft" "$APP/Contents/MacOS/vulkancraft"
else
    cp "$STAGE/usr/bin/vulkancraft" "$APP/Contents/MacOS/Matticraft"
fi
cp "$STAGE"/usr/lib/libmatti*.dylib "$APP/Contents/Frameworks/" 2>/dev/null || true
mkdir -p "$APP/Contents/Resources"
cp -R "$STAGE/usr/share/matticraft/assets" "$APP/Contents/Resources/assets"
cp "$STAGE/usr/share/matticraft/Monocraft.ttf" "$APP/Contents/Resources/Monocraft.ttf"
sed "s/__VERSION__/$VERSION/g" "$ROOT/packaging/macos/Info.plist" > "$APP/Contents/Info.plist"
mkdir -p "$DIST"

DMG="$DIST/Matticraft-${VERSION}-macos-${ARCH}.dmg"
if command -v genisoimage >/dev/null 2>&1; then
    genisoimage -quiet -V "Matticraft" -D -R -apple -no-pad -o "$DMG" "$DIST"
elif command -v mkisofs >/dev/null 2>&1; then
    mkisofs -quiet -V "Matticraft" -D -R -apple -no-pad -o "$DMG" "$DIST"
else
    echo "genisoimage/mkisofs not installed - .app is available, DMG skipped" >&2
    exit 0
fi

( cd "$DIST" && sha256sum "$(basename "$DMG")" > SHA256SUMS.dmg )
echo "unsigned DMG written to $DIST"
