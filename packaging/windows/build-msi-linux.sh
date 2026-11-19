#!/usr/bin/env bash
# Builds the Windows MSI on Linux from the MinGW cross build using wixl
# (msitools). No Windows runner is required.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT/build/windows-mingw-x64}"
VERSION="${VERSION:-0.1.0}"
DIST="${DIST:-$ROOT/dist}"
STAGE="${PACKAGE_STAGE:-$(mktemp -d)}"
trap '[ -n "${PACKAGE_STAGE:-}" ] || rm -rf "$STAGE"' EXIT

if ! command -v wixl >/dev/null 2>&1; then
    echo "wixl not installed (package: msitools) - skipping MSI" >&2
    exit 0
fi
if [ ! -d "$BUILD_DIR" ]; then
    echo "missing $BUILD_DIR (configure with the MinGW toolchain)" >&2
    exit 1
fi

DESTDIR="$STAGE" cmake --install "$BUILD_DIR" --prefix /Matticraft
BIN_DIR="$STAGE/Matticraft/bin"
if [ ! -d "$BIN_DIR" ]; then
    # Multi-config generators put the binaries in a subdirectory.
    BIN_DIR="$(find "$STAGE" -name vulkancraft.exe -printf '%h' -quit)"
fi

# The MSI installs the launcher as the entry point. Native Windows builds get
# it from CMake; cross builds can pass LAUNCHER_BIN (cargo --target
# x86_64-pc-windows-gnu) or place it in launchers/target/<target>/release.
LAUNCHER_BIN="${LAUNCHER_BIN:-}"
if [ -z "$LAUNCHER_BIN" ]; then
    for candidate in \
        "$BIN_DIR/matticraft-launcher.exe" \
        "$ROOT/launchers/target/release/matticraft-launcher.exe" \
        "$ROOT/launchers/target/x86_64-pc-windows-gnu/release/matticraft-launcher.exe"; do
        if [ -f "$candidate" ]; then
            LAUNCHER_BIN="$candidate"
            break
        fi
    done
fi
if [ -z "$LAUNCHER_BIN" ] || [ ! -f "$LAUNCHER_BIN" ]; then
    echo "matticraft-launcher.exe missing - build the launcher" >&2
    echo "(cargo build --release --target x86_64-pc-windows-gnu -p launcher-desktop)" >&2
    exit 1
fi
cp "$LAUNCHER_BIN" "$BIN_DIR/matticraft-launcher.exe"
mkdir -p "$DIST"

wixl -v \
    -D "Version=$VERSION" \
    -D "SourceDir=$BIN_DIR" \
    -o "$DIST/Matticraft-${VERSION}-x64.msi" \
    "$ROOT/packaging/windows/matticraft-wixl.wxs"

( cd "$DIST" && sha256sum "Matticraft-${VERSION}-x64.msi" > SHA256SUMS.msi )
echo "MSI written to $DIST"
