#!/usr/bin/env bash
# Installs Matticraft from the latest (or a tagged) GitHub release.
#
#   ./install.sh                # latest release, user space (~/.local)
#   ./install.sh v1.21.11-r1    # a specific tag, user space
#   ./install.sh --root         # latest release, system wide (/usr/local)
#   ./install.sh --root v1.21.11-r1
#
# What you get:
#   <prefix>/share/matticraft/game/     the game (binary, launcher, mods/)
#   <prefix>/share/matticraft/Matticraft.AppImage
#   <prefix>/bin/matticraft             the launcher on PATH
#   ~/.local/share/applications/matticraft.desktop   (menu entry)
#
# The launcher always passes --launchTarget neoforge and a stable game dir
# (~/.matticraft); more arguments go through to the game. User space needs
# no root; --root installs to /usr/local (sudo when not running as root).

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
            sed -n '2,20p' "$0" | sed 's/^# \{0,1\}//'
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
# The asset names carry the version (Matticraft-<version>-portable-linux-x64.zip).
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
GAME_DIR="${INSTALL_DIR}/game"
echo "==> installing into ${INSTALL_DIR}"
$SUDO mkdir -p "${INSTALL_DIR}" "${PREFIX}/bin"
$SUDO unzip -oq "$TMP/matticraft.zip" -d "$TMP/unpack"

# The zip root carries the versioned game folder (Matticraft-<version>).
TOPDIR=$(find "$TMP/unpack" -mindepth 1 -maxdepth 2 -type d -name 'Matticraft-*' | head -1)
if [ -z "$TOPDIR" ]; then
    echo "ERROR: unexpected zip layout (no Matticraft-* folder)" >&2
    exit 1
fi
$SUDO rm -rf "${GAME_DIR}"
$SUDO mv "$TOPDIR" "${GAME_DIR}"

BIN="${GAME_DIR}/matticraft"
LAUNCHER="${GAME_DIR}/run-matticraft.sh"
if [ ! -f "$BIN" ]; then
    echo "ERROR: the zip did not contain the matticraft binary" >&2
    exit 1
fi
$SUDO chmod +x "$BIN" "$LAUNCHER"

if [ -f "$TMP/Matticraft.AppImage" ]; then
    $SUDO mv "$TMP/Matticraft.AppImage" "${INSTALL_DIR}/Matticraft.AppImage"
    $SUDO chmod +x "${INSTALL_DIR}/Matticraft.AppImage"
fi

# ---------------------------------------------------------------------------
# Launcher on PATH - always neoforge, stable game dir, extra args pass through
# ---------------------------------------------------------------------------
$SUDO tee "${PREFIX}/bin/matticraft" > /dev/null <<EOF
#!/bin/sh
# Installed by Matticraft install.sh - the stable way to start the game.
GAME_DIR="\${MATTICRAFT_GAME_DIR:-\${HOME}/.matticraft}"
mkdir -p "\$GAME_DIR/mods"
exec "${GAME_DIR}/matticraft" --launchTarget neoforge --gameDir "\$GAME_DIR" "\$@"
EOF
$SUDO chmod +x "${PREFIX}/bin/matticraft"

# ---------------------------------------------------------------------------
# Menu entry (.desktop) - user and root installs both get one
# ---------------------------------------------------------------------------
if [ "$USE_ROOT" = 1 ]; then
    DESKTOP_DIR="/usr/local/share/applications"
    ICON_DIR="/usr/local/share/pixmaps"
else
    DESKTOP_DIR="${HOME}/.local/share/applications"
    ICON_DIR="${HOME}/.local/share/pixmaps"
fi
$SUDO mkdir -p "$DESKTOP_DIR" "$ICON_DIR"
if [ -f "${GAME_DIR}/matticraft.png" ]; then
    $SUDO cp "${GAME_DIR}/matticraft.png" "${ICON_DIR}/matticraft.png"
else
    # No icon asset in the zip - the desktop entry ships without one.
    :
fi
$SUDO tee "${DESKTOP_DIR}/matticraft.desktop" > /dev/null <<EOF
[Desktop Entry]
Type=Application
Name=Matticraft
Comment=A from-scratch C port of the Minecraft + NeoForge toolchain
Exec=${PREFIX}/bin/matticraft
Icon=matticraft
Categories=Game;
Terminal=true
EOF

# ---------------------------------------------------------------------------
# Done - tell the user what happened and what to check
# ---------------------------------------------------------------------------
echo
echo "==> installed Matticraft ${VERSION}"
echo "    game:         ${GAME_DIR}"
[ -f "${INSTALL_DIR}/Matticraft.AppImage" ] && echo "    appimage:     ${INSTALL_DIR}/Matticraft.AppImage"
echo "    launcher:     ${PREFIX}/bin/matticraft"
echo "    menu entry:   ${DESKTOP_DIR}/matticraft.desktop"
echo "    game dir:     \${HOME}/.matticraft (override with MATTICRAFT_GAME_DIR)"
echo
case ":${PATH}:" in
    *":${PREFIX}/bin:"*) ;;
    *)
        echo "    NOTE: ${PREFIX}/bin is not in your PATH. Fix with:"
        echo "        echo 'export PATH=\"${PREFIX}/bin:\$PATH\"' >> ~/.bashrc && source ~/.bashrc"
        echo "    (or run the game directly: ${LAUNCHER})"
        ;;
esac
