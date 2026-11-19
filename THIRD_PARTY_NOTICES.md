# Third-party notices

This repository contains its own MIT-licensed code and vendors upstream
projects as reference material.

## Horizon

- Path: `vendor/horizon`
- Upstream: <https://github.com/horizon-rust/horizon>
- License: LGPL-3.0-or-later (see `vendor/horizon/LICENSE`)
- Use here: pinned reference for the launcher/window look and behavior. The
  submodule is not linked into `libmatti`; it is checked out for inspection and
  comparison, just like the Java upstream sources under `vendor/`.

Do not copy Horizon source into the MIT-licensed C sources without preserving
the LGPL notices and following the LGPL-3.0 terms.
