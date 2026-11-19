#!/usr/bin/env bash
# Builds a .pkg installer from the .app produced by dmg.sh.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
VERSION="${VERSION:-0.1.0}"
DIST="${DIST:-$ROOT/dist}"
APP="$DIST/Matticraft.app"
STAGE="$(mktemp -d)"
trap 'rm -rf "$STAGE"' EXIT

if [ ! -d "$APP" ]; then
    echo "missing $APP - run packaging/macos/dmg.sh first" >&2
    exit 1
fi

pkgbuild --root "$(dirname "$APP")" --identifier org.matticraft.Matticraft \
    --version "$VERSION" --install-location /Applications "$STAGE/Matticraft.pkg"
productbuild --package "$STAGE/Matticraft.pkg" "$DIST/Matticraft-${VERSION}-universal.pkg"
if [ -n "${APPLE_INSTALLER_IDENTITY:-}" ]; then
    productsign --sign "$APPLE_INSTALLER_IDENTITY" \
        "$DIST/Matticraft-${VERSION}-universal.pkg" \
        "$DIST/Matticraft-${VERSION}-universal-signed.pkg"
fi
echo "PKG written to $DIST"
