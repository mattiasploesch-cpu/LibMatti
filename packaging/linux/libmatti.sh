#!/usr/bin/env bash
# Standalone libmatti SDK: shared library, public headers, UI assets and docs.
# This is the non-AppImage artifact for NixOS and other distributions that
# package the library themselves.
#
#   BUILD_DIR=build/linux-x64 VERSION=0.1.0 packaging/linux/libmatti.sh
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT/build/linux-x64}"
VERSION="${VERSION:-0.1.0}"
ARCH="${ARCH:-$(uname -m)}"
DIST="${DIST:-$ROOT/dist}"
STAGE="$(mktemp -d)"
trap 'rm -rf "$STAGE"' EXIT

if [ ! -d "$BUILD_DIR" ]; then
    echo "missing build directory: $BUILD_DIR" >&2
    exit 1
fi

DESTDIR="$STAGE" cmake --install "$BUILD_DIR" --prefix /usr

required=(
    usr/lib/libmatti.so
    usr/include/libmatti
    usr/share/matticraft/Monocraft.ttf
)
missing=()
for path in "${required[@]}"; do
    [ -e "$STAGE/$path" ] || missing+=("$path")
done
if [ "${#missing[@]}" -ne 0 ]; then
    echo "refusing to package an incomplete libmatti install tree:" >&2
    printf '  missing %s\n' "${missing[@]}" >&2
    exit 1
fi

# The install rule copies directory skeletons for headers; drop the Rust
# build tree and empty directories so the SDK only contains the public API.
rm -rf "$STAGE/usr/include/libmatti/rs/target"
find "$STAGE/usr/include/libmatti" -type d -empty -delete

mkdir -p "$DIST"
ARTIFACT="$DIST/libmatti-${VERSION}-linux-${ARCH}.tar.zst"
if tar --help 2>/dev/null | grep -q -- '--zstd'; then
    (cd "$STAGE" && tar --zstd -cf "$ARTIFACT" \
        usr/lib/libmatti.so* usr/include/libmatti \
        usr/share/matticraft usr/share/doc/matticraft)
else
    (cd "$STAGE" && tar -cf - \
        usr/lib/libmatti.so* usr/include/libmatti \
        usr/share/matticraft usr/share/doc/matticraft) |
        zstd -19 -o "$ARTIFACT"
fi

echo "libmatti SDK: $ARTIFACT"
