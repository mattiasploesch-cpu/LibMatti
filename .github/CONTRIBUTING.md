# Contributing

## Ground rules

1. **Port 1:1 or nothing.** Every file maps to an upstream Java file under
   `vendor/`. No inventions, no "improvements" over the original semantics.
   The header comment names the Java source.
2. **Everything stays in the repo.** No files outside the project directory
   (builds, logs, tests - see `.github/RUN_COMMANDS.md`).
3. **Small PRs, one concern each.** A PR that ports a class is one PR; a PR
   that fixes a crash is one PR.

## The PR flow

1. Branch from `main` (`feat/p4-chunk-meshing`, `fix/unique-mod-list-uaf`, ...).
2. Build + test locally:
   ```bash
   cmake -S . -B cmake-build-debug
   cmake --build cmake-build-debug
   ctest --test-dir cmake-build-debug --output-on-failure
   ```
3. Open the PR - the default template applies. The checks run automatically:
   - **Build & Test** (build + ctest, required)
   - **Client smoke test** (headless boot, required)
   - **Lint** (clang-format, non-blocking)
   - **CodeQL** (security scan)
   - @coderabbitai reviews the diff.
4. Merge = squash. The commit message is the PR title.

## Importing a mod (new_mod)

Mod sources live in the [MDK](https://github.com/mattiasploesch-cpu/matti-mdk).
The import path is:

1. Build the mod in the MDK and pack it (the MDK run configuration does this)
   into `yourmod-1.0.matti`.
2. Open a PR here that adds **exactly one** file: `mods/import/yourmod-1.0.matti`,
   using the **new_mod template** (mandatory).
3. The `new_mod` label is required - the validation workflow runs only with it.
4. The `mod-validate` workflow parses the manifest, dumps the embedded source
   into the PR (so @coderabbitai can review it), rebuilds and boots the client
   with the mod.
5. After the merge, `mod-import` unpacks the package into
   `mods/custom/<mod_id>/`, links it into the mods build and commits the
   result. The next `runClient` loads the mod from `mods/`.

## Run configurations

| Configuration | What it does |
|---|---|
| `runClient` | The plain client boot (no test harnesses) |
| `matticraft` | Full selftest run (every harness, then the launcher) |
| `build mods` | Builds the `mods/` sub-project into `mods/*.so` |

Command-line equivalents: `.github/RUN_COMMANDS.md`.

## Commits

Conventional, short, imperative: `port: net.minecraft.core.SectionPos`,
`fix: VisGraph flood-fill queue wraparound`, `ci: add mod-import workflow`.
