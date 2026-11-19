#!/usr/bin/env bash
# Portable Linux package: one tar.zst with every client, the launcher,
# libmatti, the public headers, the desktop files and the runtime resources.
# A convenience bin/ directory points at the executables inside usr/bin.
#
#   BUILD_DIR=build/linux-x64 VERSION=0.1.0 packaging/linux/portable.sh
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT/build/linux-x64}"
VERSION="${VERSION:-0.1.0}"
ARCH="${ARCH:-$(uname -m)}"
DIST="${DIST:-$ROOT/dist}"
STAGE="$(mktemp -d)"
trap 'rm -rf "$STAGE"' EXIT

if [ ! -d "$BUILD_DIR" ]; then
    echo "missing build directory: $BUILD_DIR (configure with --preset linux-x64)" >&2
    exit 1
fi

DESTDIR="$STAGE" cmake --install "$BUILD_DIR" --prefix /usr

required=(
    usr/bin/matticraft-launcher
    usr/bin/matticraft
    usr/bin/runClient
    usr/bin/vulkanClient
    usr/bin/vulkancraft
    usr/lib/libmatti.so
    usr/share/applications/org.matticraft.Matticraft.desktop
    usr/share/icons/hicolor/scalable/apps/org.matticraft.Matticraft.svg
    usr/share/matticraft/Monocraft.ttf
)
missing=()
for path in "${required[@]}"; do
    [ -e "$STAGE/$path" ] || missing+=("$path")
done
if [ "${#missing[@]}" -ne 0 ]; then
    echo "refusing to package an incomplete Linux install tree:" >&2
    printf '  missing %s\n' "${missing[@]}" >&2
    echo "rebuild with the launcher enabled (MATTI_BUILD_LAUNCHER=ON) and install it first" >&2
    exit 1
fi

# The portable archive is also the standalone-binary archive: expose every
# executable at bin/ without duplicating the files.
mkdir -p "$STAGE/bin" "$STAGE/usr/share/doc/matticraft"
for executable in matticraft matticraft-launcher runClient vulkanClient vulkancraft; do
    ln -s "../usr/bin/$executable" "$STAGE/bin/$executable"
done
cp "$ROOT/README.md" "$STAGE/usr/share/doc/matticraft/" 2>/dev/null || true
cp "$ROOT/planported.MD" "$STAGE/usr/share/doc/matticraft/" 2>/dev/null || true

mkdir -p "$DIST"
ARTIFACT="$DIST/matticraft-${VERSION}-linux-${ARCH}.tar.zst"
if tar --help 2>/dev/null | grep -q -- '--zstd'; then
    tar --zstd -cf "$ARTIFACT" -C "$STAGE" .
else
    tar -cf - -C "$STAGE" . | zstd -19 -o "$ARTIFACT"
fi

( cd "$DIST" && sha256sum "$(basename "$ARTIFACT")" > SHA256SUMS )
echo "portable package: $ARTIFACT"
