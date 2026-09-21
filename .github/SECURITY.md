# Security Policy

## Supported versions

The `main` branch is the only supported line. Security fixes land as regular
commits; tagged releases (`v*`) pick them up.

## Reporting a vulnerability

**Please do not open a public issue for security problems.**

Use GitHub's private vulnerability reporting:
**Security tab → Report a vulnerability** — or contact the owner directly
(@mattiasploesch-cpu) if private reporting is unavailable.

Include: what is affected (loader, mixin backends, render, mods pipeline),
how to reproduce, and the log output.

## Scope and posture

This project is a **development/learning port**, run locally from source:

- The client runs with the user's local file access; the FML port loads `.so`
  mods from the `mods/` folder of the game directory. Treat `.matti`/`.so`
  files from strangers like executables — because that is what they are.
- The new_mod import pipeline validates packages in CI, but **the merge of a
  mod PR is a trust decision by the maintainer**, not a sandbox guarantee.
- No network code ships yet; when Netty-based networking lands, its advisories
  will be tracked here.

## Automated scanning

- Dependency/manifest alerts: dependabot (see `.github/dependabot.yml`)
- Code scanning: CodeQL (see `.github/workflows/codeql.yml`)
