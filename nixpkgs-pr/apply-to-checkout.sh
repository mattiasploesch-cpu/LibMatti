#!/usr/bin/env bash
# Copy the prepared nixpkgs files into an existing nixpkgs checkout.
# This script deliberately does not commit, push or open a PR.
#
#   nixpkgs-pr/apply-to-checkout.sh /path/to/nixpkgs
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
NIXPKGS="${1:-}"

if [ -z "$NIXPKGS" ] || [ ! -d "$NIXPKGS/pkgs/by-name" ]; then
    echo "usage: $0 /path/to/nixpkgs" >&2
    exit 1
fi

cp -a "$ROOT/pkgs/." "$NIXPKGS/pkgs/"
echo "copied libmatti and matticraft into $NIXPKGS/pkgs/by-name"
echo "next: cd $NIXPKGS && git status"
echo "review, build, commit and submit manually; this script does not do that."
