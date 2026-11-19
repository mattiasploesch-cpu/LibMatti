#!/usr/bin/env bash
# Update the Matticraft packages in a nixpkgs checkout.
#
# Local use (does not push):
#   tools/update-nixpkgs.sh \
#     --nixpkgs /mnt/niklas/nixpkgs \
#     --version 0.1.0 \
#     --rev "$(git rev-parse HEAD)"
#
# Release use:
#   tools/update-nixpkgs.sh --nixpkgs nixpkgs --version "$VERSION" \
#     --rev "$GITHUB_SHA" --branch "matticraft-$VERSION" \
#     --fork "$NIXPKGS_FORK" --push --pr
#
# The script updates only the package files; it never rewrites nixpkgs itself.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
NIXPKGS=""
VERSION=""
REV="$(git -C "$ROOT" rev-parse HEAD 2>/dev/null || true)"
SOURCE_URL=""
HASH=""
BRANCH=""
FORK=""
REPO="mattiasploesch-cpu/LibMatti"
REMOTE="origin"
PUSH=0
PR=0

usage() {
  sed -n '2,18p' "$0" | sed 's/^# \{0,1\}//'
}

while [ "$#" -gt 0 ]; do
  case "$1" in
    --nixpkgs) NIXPKGS="${2:?}"; shift 2 ;;
    --version) VERSION="${2:?}"; shift 2 ;;
    --rev) REV="${2:?}"; shift 2 ;;
    --source-url) SOURCE_URL="${2:?}"; shift 2 ;;
    --hash) HASH="${2:?}"; shift 2 ;;
    --branch) BRANCH="${2:?}"; shift 2 ;;
    --fork) FORK="${2:?}"; shift 2 ;;
    --repo) REPO="${2:?}"; shift 2 ;;
    --remote) REMOTE="${2:?}"; shift 2 ;;
    --push) PUSH=1; shift ;;
    --pr) PR=1; shift ;;
    -h|--help) usage; exit 0 ;;
    *) echo "unknown option: $1" >&2; usage >&2; exit 2 ;;
  esac
done

if [ -z "$NIXPKGS" ] || [ ! -d "$NIXPKGS/pkgs/by-name" ]; then
  echo "missing --nixpkgs <nixpkgs checkout>" >&2
  exit 2
fi
if [ -z "$VERSION" ]; then
  echo "missing --version" >&2
  exit 2
fi
VERSION="${VERSION#v}"
if [ -z "$REV" ]; then
  echo "missing --rev" >&2
  exit 2
fi
if [ -z "$SOURCE_URL" ]; then
  SOURCE_URL="https://codeload.github.com/${REPO}/legacy.tar.gz/${REV}"
fi
if [ -z "$HASH" ]; then
  if ! command -v nix-prefetch-url >/dev/null 2>&1; then
    echo "missing --hash (or install nix-prefetch-url)" >&2
    exit 2
  fi
  HASH="$(nix hash convert --hash-algo sha256 --to sri \
    "$(nix-prefetch-url "$SOURCE_URL")")"
fi
if [ "$PR" -eq 1 ]; then
  PUSH=1
fi
if [ "$PUSH" -eq 1 ]; then
  if ! git -C "$NIXPKGS" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    echo "$NIXPKGS is not a git checkout" >&2
    exit 2
  fi
  if [ -z "$BRANCH" ]; then
    BRANCH="matticraft-${VERSION}"
  fi
  git -C "$NIXPKGS" fetch "$REMOTE" --prune
  if git -C "$NIXPKGS" show-ref --verify --quiet "refs/remotes/$REMOTE/$BRANCH"; then
    git -C "$NIXPKGS" checkout -B "$BRANCH" "$REMOTE/$BRANCH"
  else
    BASE="$(git -C "$NIXPKGS" symbolic-ref --short "refs/remotes/$REMOTE/HEAD" 2>/dev/null || true)"
    if [ -z "$BASE" ]; then
      BASE="$REMOTE/master"
    fi
    git -C "$NIXPKGS" checkout -B "$BRANCH" "$BASE"
  fi
fi

python3 - "$ROOT/nixpkgs-pr/pkgs" "$NIXPKGS/pkgs" "$VERSION" "$SOURCE_URL" "$HASH" <<'PY'
import pathlib
import re
import shutil
import sys

template, target, version, source_url, source_hash = sys.argv[1:]
template = pathlib.Path(template)
target = pathlib.Path(target)

packages = [
    ("by-name/li/libmatti", True),
    ("by-name/ma/matticraft", False),
    ("by-name/ma/matticraft-launcher", False),
]
for relative, update_source in packages:
    source_dir = template / relative
    target_dir = target / relative
    target_dir.mkdir(parents=True, exist_ok=True)
    for item in source_dir.iterdir():
        if not item.is_file():
            continue
        destination = target_dir / item.name
        shutil.copy2(item, destination)
        if destination.name != "package.nix":
            continue
        text = destination.read_text()
        text = re.sub(r'version = "[^"]+";',
                      f'version = "{version}";', text, count=1)
        if update_source:
            text = re.sub(r'url = "https://codeload\.github\.com/[^"]+";',
                          f'url = "{source_url}";', text, count=1)
            text = re.sub(r'hash = "sha256-[^"]+";',
                          f'hash = "{source_hash}";', text, count=1)
        destination.write_text(text)
PY

FILES=(
  "pkgs/by-name/li/libmatti/package.nix"
  "pkgs/by-name/ma/matticraft/package.nix"
  "pkgs/by-name/ma/matticraft-launcher/package.nix"
  "pkgs/by-name/ma/matticraft-launcher/Cargo.lock"
)
printf 'updated %s\n' "$NIXPKGS"
git -C "$NIXPKGS" status --short -- "${FILES[@]}"

if [ "$PUSH" -eq 0 ]; then
  printf '\nNo push requested. Review the files, then run with --push (and --pr) when ready.\n'
  exit 0
fi

git -C "$NIXPKGS" add "${FILES[@]}"
if git -C "$NIXPKGS" diff --cached --quiet; then
  echo "no package changes to commit"
else
  git -C "$NIXPKGS" commit -m "libmatti, matticraft, matticraft-launcher: update to ${VERSION}"
fi
git -C "$NIXPKGS" push -u "$REMOTE" "$BRANCH"

if [ "$PR" -eq 1 ]; then
  if [ -z "$FORK" ]; then
    echo "--pr needs --fork <owner/nixpkgs>" >&2
    exit 2
  fi
  OWNER="${FORK%%/*}"
  existing="$(gh pr list --repo NixOS/nixpkgs --head "$OWNER:$BRANCH" \
    --state open --json number --jq '.[0].number // empty')"
  if [ -n "$existing" ]; then
    echo "existing nixpkgs PR #$existing updated on $OWNER:$BRANCH"
  else
    gh pr create --repo NixOS/nixpkgs --base master \
      --head "$OWNER:$BRANCH" \
      --title "libmatti, matticraft, matticraft-launcher: update to ${VERSION}" \
      --body-file "$ROOT/nixpkgs-pr/PR_BODY.md"
  fi
fi
