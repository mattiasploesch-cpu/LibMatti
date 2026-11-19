#!/usr/bin/env bash
# Portable Linux package: one tar.zst with the clients, libmatti and headers.
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
mkdir -p "$STAGE/usr/share/doc/matticraft"
cp "$ROOT/README.md" "$STAGE/usr/share/doc/matticraft/" 2>/dev/null || true

mkdir -p "$DIST"
ARTIFACT="$DIST/matticraft-${VERSION}-linux-${ARCH}.tar.zst"
if tar --help 2>/dev/null | grep -q -- '--zstd'; then
    tar --zstd -cf "$ARTIFACT" -C "$STAGE" .
else
    tar -cf - -C "$STAGE" . | zstd -19 -o "$ARTIFACT"
fi

( cd "$DIST" && sha256sum "$(basename "$ARTIFACT")" > SHA256SUMS )
echo "portable package: $ARTIFACT"
