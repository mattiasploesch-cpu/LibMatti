## Description

Add `libmatti`, `matticraft` and `matticraft-launcher` at 0.1.0.

`libmatti` is the C11 game library. The shared library is installed in the
default output and the public headers in the `dev` output (`libmatti.dev`).

`matticraft` is the native client. It embeds the Rust UI, textures and the
Monocraft font and builds against the shared `libmatti`.

`matticraft-launcher` is the Rust/egui desktop launcher containing profiles,
accounts, mods, logs and update handling.

The `stb` headers and the Monocraft font are part of the library source tree
and are compiled into the binary; the nixpkgs build needs no extra downloads.

## Notes for reviewers

- `libmatti-dev` in the upstream flake is a convenience alias for
  `libmatti.dev`; nixpkgs should use the standard `dev` output.
- The launcher is the existing Rust/egui desktop application; the Android
  Compose frontend is shipped separately in the Android package.
- The source is pinned to a codeload legacy tarball because GitHub's normal
  archive URL returns 404 for this commit.

## Validation performed

On x86_64-linux:

- `libmatti` built; `libmatti.dev` contains the public headers.
- `matticraft --help` and `vulkanClient --help` run.
- `matticraft-launcher --help` and `--version` run.
- The `matticraft-launcher` Cargo dependencies were built offline from the
  vendored lock file.

## Things done

- [x] Fits [CONTRIBUTING.md], [pkgs/README.md], [maintainers/README.md] and
      other READMEs.
- [ ] Follows the [automation/AI policy]. Submitter action required: this
      draft was prepared with Codebuff (DeepSeek V4.1 Flash Fast), so the
      submitter must manually review it and rewrite this prose in their own
      words before marking the PR ready. Nixpkgs commits produced by the
      update helper carry an `Assisted-by:` trailer.
- Built on platform:
  - [x] x86_64-linux
  - [ ] aarch64-linux (evaluated, not built)
  - [ ] aarch64-darwin (not available on the test machine)
- Tested, as applicable:
  - [x] Basic functionality of all binary files (`matticraft`,
        `vulkanClient`, `matticraft-launcher`).
  - [ ] Ran `nixpkgs-review` on this PR.
  - [ ] [Package tests] at `passthru.tests` (none added).
  - [ ] [NixOS tests] in [nixos/tests] (not applicable: no NixOS module).
  - [ ] Tests in [lib/tests] or [pkgs/test] for functions and "core"
        functionality (not applicable: no library function).
- [Nixpkgs Release Notes]
  - [ ] Package update: when the change is major or breaking (new package,
        no update).
- [NixOS Release Notes]
  - [ ] Module addition: when adding a new NixOS module (not applicable).
  - [ ] Module update: when the change is significant (not applicable).

[Package tests]: https://github.com/NixOS/nixpkgs/blob/master/pkgs/README.md#package-tests
[NixOS tests]: https://nixos.org/manual/nixos/unstable/index.html#sec-nixos-tests
[nixpkgs-review usage]: https://github.com/Mic92/nixpkgs-review#usage

[CONTRIBUTING.md]: https://github.com/NixOS/nixpkgs/blob/master/CONTRIBUTING.md
[automation/AI policy]: https://github.com/NixOS/nixpkgs/blob/master/CONTRIBUTING.md#automationai-policy
[lib/tests]: https://github.com/NixOS/nixpkgs/blob/master/lib/tests
[maintainers/README.md]: https://github.com/NixOS/nixpkgs/blob/master/maintainers/README.md
[nixos/tests]: https://github.com/NixOS/nixpkgs/tree/master/nixos/tests
[pkgs/README.md]: https://github.com/NixOS/nixpkgs/blob/master/pkgs/README.md
[pkgs/test]: https://github.com/NixOS/nixpkgs/blob/master/pkgs/test
[Nixpkgs Release Notes]: https://github.com/NixOS/nixpkgs/blob/master/doc/release-notes
[NixOS Release Notes]: https://github.com/NixOS/nixpkgs/blob/master/nixos/doc/manual/release-notes
