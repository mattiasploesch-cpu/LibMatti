#!/usr/bin/env bash
# One entry point for the Linux-first release build.
#
#   VERSION=0.1.0 packaging/build-all.sh
#
# Linux packages always build. Windows (MinGW + wixl), macOS (osxcross) and
# Android (SDK/NDK + Gradle) build when their cross toolchains are installed;
# missing tools are reported instead of failing the whole run.
set -uo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
VERSION="${VERSION:-0.1.0}"
JOBS="${JOBS:-$(nproc 2>/dev/null || echo 4)}"
# REQUIRE_ALL=1 turns skipped cross targets into a failure (release gating).
REQUIRE_ALL="${REQUIRE_ALL:-0}"
FAILED=0

step() { printf '\n== %s ==\n' "$*"; }
warn() { printf 'WARN: %s\n' "$*" >&2; }

step "Linux (native)"
if cmake --preset linux-x64 && cmake --build --preset linux-x64 -j "$JOBS"; then
    BUILD_DIR="$ROOT/build/linux-x64" VERSION="$VERSION" "$ROOT/packaging/linux/portable.sh" || FAILED=1
    BUILD_DIR="$ROOT/build/linux-x64" VERSION="$VERSION" "$ROOT/packaging/linux/nfpm.sh" || {
        warn "deb/rpm packaging failed (nfpm missing or install tree incomplete)"
        FAILED=1
    }
    BUILD_DIR="$ROOT/build/linux-x64" VERSION="$VERSION" "$ROOT/packaging/linux/appimage.sh" || {
        warn "AppImage packaging failed (appimagetool missing or install tree incomplete)"
        FAILED=1
    }
else
    warn "Linux build failed"
    FAILED=1
fi

step "Windows (MinGW-w64 cross, MSI via wixl)"
if command -v x86_64-w64-mingw32-gcc >/dev/null 2>&1; then
    if cmake -S "$ROOT" -B "$ROOT/build/windows-mingw-x64" \
            -DCMAKE_TOOLCHAIN_FILE="$ROOT/cmake/toolchains/mingw-w64.cmake" \
            -DCMAKE_BUILD_TYPE=Release \
            -DMATTI_BUILD_LAUNCHER=OFF \
        && cmake --build "$ROOT/build/windows-mingw-x64" --target matticraft vulkancraft runClient vulkanClient matti_shared -j "$JOBS"; then
        # The launcher has to be cross-built by cargo; only pass it on when the
        # rustup GNU target is installed.
        LAUNCHER_BIN=""
        WIN_LAUNCHER="$ROOT/launchers/target/x86_64-pc-windows-gnu/release/matticraft-launcher.exe"
        if [ ! -f "$WIN_LAUNCHER" ] && command -v rustup >/dev/null 2>&1 \
                && rustup target list --installed 2>/dev/null | grep -qx x86_64-pc-windows-gnu; then
            ( cd "$ROOT/launchers" && cargo build --release --target x86_64-pc-windows-gnu -p launcher-desktop ) || true
        fi
        [ -f "$WIN_LAUNCHER" ] && LAUNCHER_BIN="$WIN_LAUNCHER"
        BUILD_DIR="$ROOT/build/windows-mingw-x64" VERSION="$VERSION" \
            LAUNCHER_BIN="$LAUNCHER_BIN" \
            "$ROOT/packaging/windows/build-msi-linux.sh" || FAILED=1
    else
        warn "Windows cross build failed (platform backends may still be missing)"
        FAILED=1
    fi
else
    warn "x86_64-w64-mingw32-gcc missing - Windows artifacts skipped (apt install mingw-w64)"
    [ "$REQUIRE_ALL" = 1 ] && FAILED=1
fi

step "macOS (osxcross cross)"
if [ -n "${OSXCROSS_ROOT:-}" ]; then
    if cmake -S "$ROOT" -B "$ROOT/build/macos-osxcross" \
            -DCMAKE_TOOLCHAIN_FILE="$ROOT/cmake/toolchains/osxcross.cmake" \
            -DCMAKE_BUILD_TYPE=Release \
            -DMATTI_BUILD_LAUNCHER=OFF \
        && cmake --build "$ROOT/build/macos-osxcross" --target vulkancraft matti_shared -j "$JOBS"; then
        BUILD_DIR="$ROOT/build/macos-osxcross" VERSION="$VERSION" \
            "$ROOT/packaging/macos/dmg-linux.sh" || FAILED=1
    else
        warn "macOS cross build failed (Apple SDK/platform backends may be missing)"
        FAILED=1
    fi
else
    warn "OSXCROSS_ROOT missing - macOS artifacts skipped (unsigned only; signing needs Apple hardware)"
    [ "$REQUIRE_ALL" = 1 ] && FAILED=1
fi

step "FreeBSD (native)"
if [ "$(uname -s)" = "FreeBSD" ]; then
    if cmake --preset freebsd-x64 && cmake --build --preset freebsd-x64 -j "$JOBS"; then
        BUILD_DIR="$ROOT/build/freebsd-x64" VERSION="$VERSION" \
            "$ROOT/packaging/freebsd/portable.sh" || FAILED=1
    else
        warn "FreeBSD build failed"
        FAILED=1
    fi
else
    warn "not on FreeBSD - FreeBSD artifacts are built by the FreeBSD CI job"
fi

step "Android (SDK/NDK + Gradle)"
if command -v gradle >/dev/null 2>&1 && [ -n "${ANDROID_HOME:-}${ANDROID_SDK_ROOT:-}" ]; then
    if gradle -p "$ROOT/library/src/mobile" :mobile_client:assembleRelease \
        && gradle -p "$ROOT/library/src/mobile" :mobile_client:bundleRelease; then
        mkdir -p "$ROOT/dist/android"
        cp "$ROOT"/library/src/mobile/mobile_client/build/outputs/apk/release/*.apk "$ROOT/dist/android/" 2>/dev/null || true
        cp "$ROOT"/library/src/mobile/mobile_client/build/outputs/bundle/release/*.aab "$ROOT/dist/android/" 2>/dev/null || true
    else
        warn "Android build failed"
        FAILED=1
    fi
else
    warn "gradle or Android SDK missing - APK/AAB skipped"
    [ "$REQUIRE_ALL" = 1 ] && FAILED=1
fi

step "Result"
ls -lh "$ROOT/dist" 2>/dev/null || warn "no dist/ directory"
exit "$FAILED"
