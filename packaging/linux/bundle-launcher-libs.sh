#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BINARY="${1:-$ROOT/build/bin/matticraft-launcher}"
LIBDIR="${2:-$(dirname "$BINARY")/lib}"

if [ ! -x "$BINARY" ]; then
    echo "bundle-launcher-libs: missing launcher binary $BINARY" >&2
    exit 1
fi

mkdir -p "$LIBDIR"

source_dir="${MATTI_LAUNCHER_LIB_DIR:-}"
if [ -z "$source_dir" ] && command -v nix >/dev/null 2>&1; then
    source_dir="$(nix build --no-link --print-out-paths \
        nixpkgs#xorg.libX11 \
        nixpkgs#xorg.libXcursor \
        nixpkgs#xorg.libXrandr \
        nixpkgs#xorg.libXi \
        nixpkgs#xorg.libXext \
        nixpkgs#xorg.libXau \
        nixpkgs#xorg.libXdmcp \
        nixpkgs#xorg.libXrender \
        nixpkgs#xorg.libXfixes \
        nixpkgs#xorg.libxcb \
        nixpkgs#libxkbcommon \
        nixpkgs#wayland \
        nixpkgs#libGL 2>/dev/null | tr '\n' ':' || true)"
fi

if [ -n "$source_dir" ]; then
    IFS=':' read -r -a roots <<< "$source_dir"
    for root in "${roots[@]}"; do
        [ -d "$root" ] || continue
        if [ -d "$root/lib" ]; then
            cp -L                "$root"/lib/libX11.so* "$root"/lib/libX11-xcb.so* \
                "$root"/lib/libXcursor.so* "$root"/lib/libXrandr.so* \
                "$root"/lib/libXi.so* "$root"/lib/libXext.so* \
                "$root"/lib/libXau.so* "$root"/lib/libXdmcp.so* \
                "$root"/lib/libXrender.so* "$root"/lib/libXfixes.so* \
                "$root"/lib/libxcb.so* "$root"/lib/libxcb-render.so* \
                "$root"/lib/libxcb-shm.so* "$root"/lib/libxcb-util.so* \
                "$root"/lib/libxkbcommon.so* "$root"/lib/libxkbcommon-x11.so* \
                "$root"/lib/libwayland-client.so* \
                "$root"/lib/libwayland-cursor.so* "$root"/lib/libwayland-egl.so* \
                "$root"/lib/libGL.so* "$root"/lib/libEGL.so* \
                "$LIBDIR/" 2>/dev/null || true
        fi
    done
fi

for name in libX11.so.6 libX11-xcb.so.1 libXcursor.so.1 libXrandr.so.2 libXi.so.6 \
    libXext.so.6 libXau.so.6 libXdmcp.so.6 libXrender.so.1 libXfixes.so.3 \
    libxcb.so.1 libxcb-render.so.0 libxcb-shm.so.0 libxkbcommon.so.0 \
    libxkbcommon-x11.so.0 libwayland-client.so.0 libwayland-cursor.so.0 libwayland-egl.so.1 \
    libGL.so.1 libEGL.so.1; do
    [ -e "$LIBDIR/$name" ] || echo "bundle-launcher-libs: warning: $name missing" >&2
done

if command -v patchelf >/dev/null 2>&1; then
    patchelf --set-rpath '$ORIGIN/lib:$ORIGIN/../lib' "$BINARY"
fi

echo "launcher runtime libraries: $LIBDIR"
