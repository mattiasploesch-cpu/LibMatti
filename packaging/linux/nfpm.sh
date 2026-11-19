#!/usr/bin/env bash
# Builds .deb and .rpm from the staged install tree using nfpm.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT/build/linux-x64}"
VERSION="${VERSION:-0.1.0}"
ARCH="${ARCH:-$(uname -m)}"
# nfpm/Debian/RPM use amd64/arm64/386, not the uname spellings.
case "$ARCH" in
    x86_64|amd64) ARCH=amd64 ;;
    aarch64|arm64) ARCH=arm64 ;;
    i386|i486|i586|i686|x86) ARCH=386 ;;
    armv7l|armhf) ARCH=armhf ;;
esac
DIST="${DIST:-$ROOT/dist}"
STAGE="${PACKAGE_STAGE:-$(mktemp -d)}"
trap '[ -n "${PACKAGE_STAGE:-}" ] || rm -rf "$STAGE"' EXIT

if ! command -v nfpm >/dev/null 2>&1; then
    echo "nfpm not installed - skipping deb/rpm (install: go install github.com/goreleaser/nfpm/v2/cmd/nfpm@latest)" >&2
    exit 0
fi

DESTDIR="$STAGE" cmake --install "$BUILD_DIR" --prefix /usr
mkdir -p "$DIST"

# nfpm does not expand environment variables inside contents.src. Render the
# staging path into a temporary config so the globs point at the install tree.
CONFIG="$STAGE/nfpm.yaml"
sed -e "s|\${PACKAGE_STAGE}|$STAGE|g" \
    -e "s|\${ARCH}|$ARCH|g" \
    -e "s|\${VERSION}|$VERSION|g" \
    "$ROOT/packaging/linux/nfpm.yaml" > "$CONFIG"

nfpm package --config "$CONFIG" --packager deb \
    --target "$DIST/matticraft_${VERSION}_${ARCH}.deb"
nfpm package --config "$CONFIG" --packager rpm \
    --target "$DIST/matticraft-${VERSION}.${ARCH}.rpm"

( cd "$DIST" && sha256sum matticraft_"${VERSION}"_"${ARCH}".deb matticraft-"${VERSION}"."${ARCH}".rpm > SHA256SUMS.packages )
echo "packages written to $DIST"
