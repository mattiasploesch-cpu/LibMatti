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
- The launcher frontend is not included in this PR; it has additional Rust
  dependencies and will follow separately.
- Tested locally with `nix-build -A libmatti`, `nix-build -A matticraft` and
  `nix-build -A libmatti.dev`.

## Checklist

- [x] Built on x86_64-linux
- [x] `libmatti.dev` contains the public headers
- [x] `matticraft --help` starts
- [x] `matticraft-launcher --version` builds
- [ ] aarch64-linux build
