# Nixpkgs PR preparation

`search.nix.org` indexes nixpkgs. These files are the local PR content and are
not submitted by this repository.

## Files

- `pkgs/by-name/li/libmatti/package.nix` — library in `out`, headers in `dev`
- `pkgs/by-name/ma/matticraft/package.nix` — client binary
- `pkgs/by-name/ma/matticraft-launcher/package.nix` — Rust/egui launcher

The development headers are the standard nixpkgs `dev` output of `libmatti`:
`libmatti.dev`. Nixpkgs does not normally publish a second package named
`libmatti-dev`; that would duplicate `libmatti.dev`. The flake keeps the
convenience alias `.#libmatti-dev`.

## Apply locally

```bash
git clone --depth=1 https://github.com/NixOS/nixpkgs.git
cd nixpkgs
git checkout -b matticraft-0.1.0

cp -a /path/to/LibMatti/nixpkgs-pr/pkgs/. pkgs/
git add pkgs/by-name/li/libmatti pkgs/by-name/ma/matticraft pkgs/by-name/ma/matticraft-launcher
git commit -m "libmatti, matticraft: init at 0.1.0"
```

Build before submitting:

```bash
nix-build -A libmatti
nix-build -A matticraft
nix-build -A matticraft-launcher
nix-build -A libmatti.dev
```

Then push the branch to your nixpkgs fork and open the PR manually with the
text in `PR_BODY.md`. Do not run `gh pr create` until you have reviewed the
package and the build results.

## Release automation

`Stable Release` has an `update_nixpkgs` checkbox. To use it, set the repository
variable `NIXPKGS_FORK=<owner>/nixpkgs` and the secret `NIXPKGS_TOKEN` with
write access to that fork. The release job updates the branch
`matticraft-<version>` and creates the PR on the first run; later releases push
to the same branch and update the existing PR.

After the first successful push, set the repository variable
`NIXPKGS_AUTO_UPDATE=true` to run the update on every release without selecting
the checkbox. The local `/mnt/niklas/nixpkgs` checkout is supported directly:

```bash
tools/update-nixpkgs.sh --nixpkgs /mnt/niklas/nixpkgs \
  --version 0.1.0 --rev "$(git rev-parse HEAD)"
```

Without `--push` it only updates the package files; `--push --fork
<owner>/nixpkgs --pr` performs the GitHub update.
