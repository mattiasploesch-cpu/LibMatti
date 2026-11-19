## Description

Add `libmatti` and `matticraft` at 0.1.0.

`libmatti` is the C11 game library. The shared library is installed in the
default output and the public headers in the `dev` output (`libmatti.dev`).

`matticraft` is the native client. It embeds the Rust UI, textures and the
Monocraft font and builds against the shared `libmatti`.

The project's git submodules contain the `stb` headers and the Monocraft font.
They are fetched as pinned files so the nixpkgs build does not depend on
submodule checkout state.

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
- [ ] aarch64-linux build
