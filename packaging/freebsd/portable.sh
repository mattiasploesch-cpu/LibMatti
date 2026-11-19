#!/usr/bin/env sh
# Portable FreeBSD package: one tar.zst with the launcher, the clients,
# libmatti and the public headers.
#
#   BUILD_DIR=build/freebsd-x64 VERSION=0.1.0 packaging/freebsd/portable.sh
#
# FreeBSD ships libGL/libvulkan in the base system; SDL3 and GLFW come from
# packages (pkg install sdl3 glfw). The executable embeds what it can through
# .matti_lib_* sections, the package additionally documents the pkg deps.
set -eu

ROOT="$(cd "$(dirname "${0}")/../.." && pwd)"
BUILD_DIR="${BUILD_DIR:-${ROOT}/build/freebsd-x64}"
VERSION="${VERSION:-0.1.0}"
ARCH="${ARCH:-$(uname -m)}"
DIST="${DIST:-${ROOT}/dist}"
STAGE="$(mktemp -d)"
trap 'rm -rf "${STAGE}"' EXIT

if [ ! -d "${BUILD_DIR}" ]; then
    echo "missing build directory: ${BUILD_DIR}" >&2
    exit 1
fi

DESTDIR="${STAGE}" cmake --install "${BUILD_DIR}" --prefix /usr/local
mkdir -p "${STAGE}/usr/local/share/doc/matticraft"
cp "${ROOT}/README.md" "${STAGE}/usr/local/share/doc/matticraft/" 2>/dev/null || true
cp "${ROOT}/planported.MD" "${STAGE}/usr/local/share/doc/matticraft/" 2>/dev/null || true

mkdir -p "${DIST}"
ARTIFACT="${DIST}/matticraft-${VERSION}-freebsd-${ARCH}.tar.zst"
if tar --help 2>/dev/null | grep -q -- '--zstd'; then
    tar --zstd -cf "${ARTIFACT}" -C "${STAGE}" .
else
    tar -cf - -C "${STAGE}" . | zstd -19 -o "${ARTIFACT}"
fi

(cd "${DIST}" && sha256sum "$(basename "${ARTIFACT}")" > SHA256SUMS.freebsd)
echo "FreeBSD package: ${ARTIFACT}"
