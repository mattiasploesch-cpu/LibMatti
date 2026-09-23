#!/usr/bin/env bash
# Installs Matticraft from the latest (or a tagged) GitHub release.
#
#   ./install.sh                # latest release, user space (~/.local)
#   ./install.sh v1.21.11-r1    # a specific tag, user space
#   ./install.sh --root         # latest release, system wide (/usr/local)
#   ./install.sh --root v1.21.11-r1
#
# The installer downloads the portable zip from the GitHub release, unpacks
# it into the target prefix, installs the AppImage alongside and symlinks
# the launcher into PATH. User space needs no root; --root installs to
# /usr/local (sudo when not running as root).

set -euo pipefail

REPO="mattiasploesch-cpu/LibMatti"
USER_PREFIX="${HOME}/.local"
ROOT_PREFIX="/usr/local"

TAG=""
USE_ROOT=0

for arg in "$@"; do
    case "$arg" in
        --root) USE_ROOT=1 ;;
        -h|--help)
            sed -n '2,12p' "$0" | sed 's/^# \{0,1\}//'
            exit 0
            ;;
        v*) TAG="$arg" ;;
        *)  echo "unknown argument: $arg (use --root and/or a vX.Y.Z tag)" >&2; exit 2 ;;
    esac
done

if [ "$USE_ROOT" = 1 ]; then
    PREFIX="$ROOT_PREFIX"
    if [ "$(id -u)" != 0 ] && command -v sudo >/dev/null 2>&1; then
        SUDO="sudo"
    else
        SUDO=""
    fi
else
    PREFIX="$USER_PREFIX"
    SUDO=""
fi

command -v curl >/dev/null 2>&1 || { echo "curl is required" >&2; exit 1; }
command -v unzip >/dev/null 2>&1 || { echo "unzip is required" >&2; exit 1; }

# ---------------------------------------------------------------------------
# Resolve the release
# ---------------------------------------------------------------------------
if [ -n "$TAG" ]; then
    API="releases/tags/${TAG}"
else
    API="releases/latest"
fi

echo "==> resolving release ${TAG:-latest} from ${REPO}"
META=$(curl -fsSL "https://api.github.com/repos/${REPO}/${API}") || {
    echo "ERROR: cannot reach the GitHub API (rate limit? wrong tag?)" >&2
    exit 1
}

VERSION=$(printf '%s' "$META" | grep -o '"tag_name": *"[^"]*"' | head -1 | sed 's/.*"v\?\([^"]*\)"$/\1/')
ZIP_URL=$(printf '%s' "$META" | grep -o '"browser_download_url": *"[^"]*portable-linux-x64\.zip"' | head -1 | sed 's/.*"\(https[^"]*\)"$/\1/')
APPIMAGE_URL=$(printf '%s' "$META" | grep -o '"browser_download_url": *"[^"]*x86_64\.AppImage"' | head -1 | sed 's/.*"\(https[^"]*\)"$/\1/')

if [ -z "$ZIP_URL" ]; then
    echo "ERROR: release has no portable zip asset" >&2
    exit 1
fi

echo "==> version ${VERSION}"

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

# ---------------------------------------------------------------------------
# Download
# ---------------------------------------------------------------------------
echo "==> downloading the portable zip"
curl -fL --progress-bar -o "$TMP/matticraft.zip" "$ZIP_URL"

if [ -n "$APPIMAGE_URL" ]; then
    echo "==> downloading the AppImage"
    curl -fL --progress-bar -o "$TMP/Matticraft.AppImage" "$APPIMAGE_URL"
fi

# ---------------------------------------------------------------------------
# Install into the prefix
# ---------------------------------------------------------------------------
INSTALL_DIR="${PREFIX}/share/matticraft"
echo "==> installing into ${INSTALL_DIR}"
$SUDO mkdir -p "${INSTALL_DIR}" "${PREFIX}/bin"
$SUDO unzip -oq "$TMP/matticraft.zip" -d "$TMP/unpack"

# The zip carries a single top-level folder (Matticraft-<version>); move its
# contents into the install dir so the path stays stable across versions.
TOPDIR=$(find "$TMP/unpack" -mindepth 1 -maxdepth 1 -type d | head -1)
$SUDO rm -rf "${INSTALL_DIR}/game"
$SUDO mv "$TOPDIR" "${INSTALL_DIR}/game"
$SUDO chmod +x "${INSTALL_DIR}/game/matticraft" "${INSTALL_DIR}/game/run-matticraft.sh"

if [ -f "$TMP/Matticraft.AppImage" ]; then
    $SUDO mv "$TMP/Matticraft.AppImage" "${INSTALL_DIR}/Matticraft.AppImage"
    $SUDO chmod +x "${INSTALL_DIR}/Matticraft.AppImage"
fi

# ---------------------------------------------------------------------------
# Launcher on PATH
# ---------------------------------------------------------------------------
$SUDO tee "${PREFIX}/bin/matticraft" > /dev/null <<EOF
#!/bin/sh
# Installed by Matticraft install.sh - runs the game with a stable game dir.
GAME_DIR="\${MATTICRAFT_GAME_DIR:-\${HOME}/.matticraft}"
mkdir -p "\$GAME_DIR"
exec "${INSTALL_DIR}/game/matticraft" --launchTarget neoforge --gameDir "\$GAME_DIR" "\$@"
EOF
$SUDO chmod +x "${PREFIX}/bin/matticraft"

if [ "$USE_ROOT" = 1 ]; then
    # A desktop entry makes the AppImage findable in menus.
    $SUDO mkdir -p /usr/local/share/applications 2>/dev/null || true
    if [ -f "${INSTALL_DIR}/game/matticraft.desktop" ]; then
        $SUDO cp "${INSTALL_DIR}/game/matticraft.desktop" /usr/local/share/applications/ 2>/dev/null || true
    fi
fi

echo
echo "==> installed Matticraft ${VERSION}"
echo "    run:          matticraft"
echo "    game dir:     \${HOME}/.matticraft (override with MATTICRAFT_GAME_DIR)"
[ -f "${INSTALL_DIR}/Matticraft.AppImage" ] && echo "    appimage:     ${INSTALL_DIR}/Matticraft.AppImage"
[ "$USE_ROOT" = 0 ] && [ -d "${PREFIX}/bin" ] && case ":${PATH}:" in
    *":${PREFIX}/bin:"*) ;;
    *) echo "    note: add ${PREFIX}/bin to your PATH to use the launcher" ;;
esac
echo "    uninstall:    rm -rf ${INSTALL_DIR} ${PREFIX}/bin/matticraft"
