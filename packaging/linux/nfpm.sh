#!/usr/bin/env bash
# Builds .deb and .rpm from the staged install tree using nfpm.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT/build/linux-x64}"
VERSION="${VERSION:-0.1.0}"
ARCH="${ARCH:-$(uname -m)}"
# nfpm/Debian/RPM use amd64/arm64/386, not the uname spellings.
case "$ARCH" in
    x86_64|amd64) ARCH=amd64; LABEL=x86_64 ;;
    aarch64|arm64) ARCH=arm64; LABEL=arm64 ;;
    i386|i486|i586|i686|x86) ARCH=386; LABEL=i386 ;;
    armv7l|armhf) ARCH=armhf; LABEL=armhf ;;
    *) LABEL="$ARCH" ;;
esac
DIST="${DIST:-$ROOT/dist}"
STAGE="${PACKAGE_STAGE:-$(mktemp -d)}"
trap '[ -n "${PACKAGE_STAGE:-}" ] || rm -rf "$STAGE"' EXIT

if ! command -v nfpm >/dev/null 2>&1; then
    echo "nfpm not installed - cannot build deb/rpm (install: go install github.com/goreleaser/nfpm/v2/cmd/nfpm@latest)" >&2
    exit 1
fi
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
    usr/share/matticraft/Monocraft.ttf
)
missing=()
for path in "${required[@]}"; do
    [ -e "$STAGE/$path" ] || missing+=("$path")
done
if [ "${#missing[@]}" -ne 0 ]; then
    echo "refusing to package an incomplete Linux install tree:" >&2
    printf '  missing %s\n' "${missing[@]}" >&2
    exit 1
fi

mkdir -p "$DIST"

# nfpm does not expand environment variables inside contents.src. Render the
# staging path into a temporary config so the tree points at the install tree.
CONFIG="$STAGE/nfpm.yaml"
sed -e "s|\${PACKAGE_STAGE}|$STAGE|g" \
    -e "s|\${ARCH}|$ARCH|g" \
    -e "s|\${VERSION}|$VERSION|g" \
    "$ROOT/packaging/linux/nfpm.yaml" > "$CONFIG"

DEB="$DIST/Matticraft-${VERSION}-linux-${LABEL}.deb"
RPM="$DIST/Matticraft-${VERSION}-linux-${LABEL}.rpm"
nfpm package --config "$CONFIG" --packager deb --target "$DEB"
nfpm package --config "$CONFIG" --packager rpm --target "$RPM"

( cd "$DIST" && sha256sum "$(basename "$DEB")" "$(basename "$RPM")" > SHA256SUMS.packages )
echo "packages written to $DIST"
