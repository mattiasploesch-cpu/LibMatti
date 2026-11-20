#!/usr/bin/env bash
# Builds Matticraft.app and a DMG from the universal CMake build.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT/build/macos-universal}"
VERSION="${VERSION:-0.1.0}"
ARCH="${ARCH:-$(uname -m)}"
case "$ARCH" in
    aarch64) ARCH=arm64 ;;
    amd64) ARCH=x86_64 ;;
esac
DIST="${DIST:-$ROOT/dist}"
APP="$DIST/Matticraft.app"
STAGE="$(mktemp -d)"
trap 'rm -rf "$STAGE"' EXIT

DESTDIR="$STAGE" cmake --install "$BUILD_DIR" --prefix /usr

rm -rf "$APP"
mkdir -p "$APP/Contents/MacOS" "$APP/Contents/Frameworks" "$APP/Contents/Resources"
if [ -x "$STAGE/usr/bin/matticraft-launcher" ]; then
    cp "$STAGE/usr/bin/matticraft-launcher" "$APP/Contents/MacOS/Matticraft"
    cp "$STAGE/usr/bin/vulkancraft" "$APP/Contents/MacOS/vulkancraft"
else
    # Cross build without the Rust launcher: the game is the bundle executable.
    cp "$STAGE/usr/bin/vulkancraft" "$APP/Contents/MacOS/Matticraft"
fi
cp "$STAGE/usr/lib/libmatti."*.dylib "$APP/Contents/Frameworks/" 2>/dev/null || true
mkdir -p "$APP/Contents/Resources"
cp -R "$STAGE/usr/share/matticraft/assets" "$APP/Contents/Resources/assets"
cp "$STAGE/usr/share/matticraft/Monocraft.ttf" "$APP/Contents/Resources/Monocraft.ttf"
sed "s/__VERSION__/$VERSION/g" "$ROOT/packaging/macos/Info.plist" > "$APP/Contents/Info.plist"

# The shared lib is found next to the executable (Contents/Frameworks).
install_name_tool -add_rpath "@executable_path/../Frameworks" "$APP/Contents/MacOS/Matticraft" 2>/dev/null || true
for _binary in "$APP/Contents/MacOS/Matticraft" "$APP/Contents/MacOS/vulkancraft"; do
    [ -f "$_binary" ] && install_name_tool -add_rpath "@executable_path/../Frameworks" "$_binary" 2>/dev/null || true
done

if [ -n "${APPLE_SIGNING_IDENTITY:-}" ]; then
    codesign --force --deep --options runtime --timestamp \
        --sign "$APPLE_SIGNING_IDENTITY" "$APP"
    if [ -n "${APPLE_ID:-}" ] && [ -n "${APPLE_TEAM_ID:-}" ] && [ -n "${APPLE_APP_PASSWORD:-}" ]; then
        ditto -c -k --keepParent "$APP" "$STAGE/Matticraft.zip"
        xcrun notarytool submit "$STAGE/Matticraft.zip" \
            --apple-id "$APPLE_ID" --team-id "$APPLE_TEAM_ID" \
            --password "$APPLE_APP_PASSWORD" --wait
        xcrun stapler staple "$APP"
    fi
fi

DMG="$DIST/Matticraft-${VERSION}-macos-${ARCH}.dmg"
if command -v create-dmg >/dev/null 2>&1; then
    create-dmg --volname "Matticraft" --app-drop-link 450 200 "$DMG" "$APP"
else
    hdiutil create -volname "Matticraft" -srcfolder "$APP" -ov -format UDZO "$DMG"
fi
echo "DMG written to $DIST"
echo "App bundle left at $APP for the ZIP step"
echo "Architecture: $ARCH"
