# GitHub Actions & Repository Automation

Every automated thing this repository does, what triggers it, and how to run
it by hand. Two trigger paths exist for everything manual: the **web UI**
(nothing to install) and the **`gh` CLI** (scriptable).

---

## 1. Triggering a workflow from the web UI

Workflows with a `workflow_dispatch` trigger can be started by hand. In this
repo that is: **Stable Release**, **Live Run**, **Label Sync**, **Stale**.

### Step by step (Stable Release as the example)

1. Open **https://github.com/mattiasploesch-cpu/LibMatti/actions**
   (the **Actions** tab at the top of the repo).
2. In the left sidebar, click the workflow name — **Stable Release**.
   (Not a run entry from the list — the *workflow name* in the sidebar.)
3. On the right, above the run list, a banner appears:
   **"This workflow has a workflow_dispatch event trigger."**
   Click the **Run workflow** button on the right side of that banner.
4. A small form drops down:
   - **Use workflow from** — leave `main` (the branch the workflow runs
     from; the build checks out exactly this branch).
   - **version** — the release version, e.g. `1.21.11-r1`. It becomes the
     tag (`v1.21.11-r1`), the artifact names and the release title.
   - **Mark as pre-release** — check only for RC/beta builds.
   - **Skip the ctest gate before packaging** — normally leave unchecked;
     tests are the quality gate for a stable release.
5. Click the green **Run workflow** button.
6. You land on the Actions list; the new run appears at the top with a
   yellow dot (running), green check (success) or red cross (failed).
   Click it to watch every step live.
7. When it is green, the release exists under
   **https://github.com/mattiasploesch-cpu/LibMatti/releases** — with the
   AppImage, the portable zip, the dev SDK, the `.matti` mods, the
   changelog and `SHA256SUMS` attached.

### CLI equivalent

```bash
gh workflow run stable-release.yml --ref main -f version=1.21.11-r1
gh run list --workflow=stable-release.yml --limit 1     # watch status
gh run watch                                            # live follow
```

---

## 2. The workflows

| # | Workflow | File | Trigger | What it does |
|---|----------|------|---------|--------------|
| 1 | **Build & Test** | `build.yml` | every push to `main`, every PR | The main CI gate: Release build, full ctest suite. |
| 2 | **PR Checks** | `pr-checks.yml` | PRs (opened/synchronize/reopened/labeled) | clang-format lint, build, client smoke test under xvfb. |
| 3 | **CodeQL** | `codeql.yml` | PRs against `main`, pushes to `main`, weekly (Wed 03:30 UTC) | GitHub's static security analysis (C/C++). Fails the PR on new high alerts. |
| 4 | **Live Run** | `live-run.yml` | push to `main`, weekly (Mon 04:00 UTC), **manual** | Boots the real client headless (xvfb), asserts the window/FML pipeline comes up. The "does the game still start" canary. |
| 5 | **Stable Release** | `stable-release.yml` | **manual only** | Full release pipeline — see section 3. |
| 6 | **Release** | `release.yml` | pushing a `v*` tag | The tag-driven release variant: build, test, pack `.matti` mods, binaries + GitHub release. |
| 7 | **Mod Validation** | `mod-validate.yml` | PRs touching mods / labeled `new_mod` | Builds the submitted mod `.so`, validates the `.matti` package (ELF manifest section, metadata), posts the result as a check. |
| 8 | **Mod Import** | `mod-import.yml` | PR **closed** (merged) with `mods/import/**` changes | Unpacks the imported `.matti`, stores sources under `mods/custom/<mod>`, links them into the mods build, produces the final `.so` in `mods/`. |
| 9 | **Auto Assign** | `auto-assign.yml` | PR opened/reopened | Assigns the author, requests a reviewer. |
| 10 | **PR Milestone Labels** | `pr-milestone.yml` | PRs | Derives milestone labels (`P4.x`, phase) from branch/title and applies them. |
| 11 | **Projects Automation** | `projects.yml` | PR lifecycle events | Adds/moves PRs as cards on the GitHub Projects board (degrades gracefully without project scope). |
| 12 | **Welcome** | `welcome.yml` | first PR/issue of a contributor | Posts a welcome comment with the contribution links. |
| 13 | **Stale** | `stale.yml` | daily 05:00 UTC, **manual** | Marks stale issues/PRs, closes them after further inactivity. |
| 14 | **Label Sync** | `label-sync.yml` | changes to `.github/labels.yml`, **manual** | Syncs the repo label set from `labels.yml` (source of truth). |
| 15 | **CI Status** | `ci-status.yml` | completion of Build/PR-Checks/Mod-Validation | Aggregates results into one rollup check + status on the commit. |

### Trigger types in one line each

- `push` / `pull_request` — automatic, nothing to do.
- `workflow_dispatch` — the manual button (web UI: **Actions → workflow →
  Run workflow**; CLI: `gh workflow run <file> -f key=value`).
- `schedule` (cron) — runs on GitHub's clock, in UTC.
- `workflow_run` — reacts to *another* workflow finishing.
- `pull_request_target` — like `pull_request`, but with write access on
  the base repo (used by Welcome; safe here because it only comments).

---

## 3. What the Stable Release pipeline produces

Input: `version` (e.g. `1.21.11-r1`). Every artifact carries the version.

| Artifact | Content |
|----------|---------|
| `Matticraft-<v>-x86_64.AppImage` | The game as a single AppImage. Double-click in a desktop environment, or `chmod +x` + run. Contains the binary, AppRun, desktop entry, icon, README and an empty `mods/` folder. |
| `Matticraft-<v>-portable-linux-x64.zip` | Unpack anywhere, `./run-matticraft.sh` — the launcher points `--gameDir` at the unpacked folder. Same layout as the AppDir without the AppImage plumbing. |
| `libmatti-dev-linux-x64.tar.gz` | For mod developers: `libmatti.a` (Release), the full `include/libmatti` header tree, `libmatti-config.cmake`. |
| `mods/*.matti` | The demo mods packed with `tools/matti-pack.sh` (ELF `.so` + `.matti_pack` metadata section). |
| `CHANGELOG-<v>.md` | Every commit since the previous `v*` tag (author + hash). |
| `SHA256SUMS` | Checksums over all artifacts above. |

The pipeline: Release build → ctest gate (skippable) → stage portable
layout → linuxdeploy AppImage → zip → dev SDK → mod packages → changelog
→ checksums → create `v<version>` tag on the built commit → GitHub
Release with everything attached.

The **single binary** claim: `libmatti` is a static library and the client
links it with `WHOLE_ARCHIVE`, so the shipped `matticraft` executable
*is* the library-included build — mods resolve libmatti symbols against
the host process at `dlopen` time. No external library install beyond
the system OpenGL/GLFW drivers.

---

## 4. GitHub infrastructure around the code

| Thing | File(s) | Purpose |
|-------|---------|---------|
| **Security policy** | `SECURITY.md` | How to report vulnerabilities privately. |
| **Code owners** | `CODEOWNERS` | Who is auto-requested as reviewer per path. |
| **Contributing guide** | `CONTRIBUTING.md` | Branch/commit/PR conventions. |
| **Issue templates** | `ISSUE_TEMPLATE/bug_report.md`, `mod_request.md`, `config.yml` | Structured bug reports and mod requests; `config.yml` adds the "blank issue" toggle and contact links. |
| **PR templates** | `pull_request_template.md`, `PULL_REQUEST_TEMPLATE/new_mod.md` | The default PR body and the special template for `new_mod` PRs (used with the Mod Validation/Import pipeline). |
| **Label set** | `labels.yml` + Label Sync | The canonical labels; synced on change or by hand. |
| **Auto assign config** | `auto-assign.yml` | Reviewer/assignee rules for the Auto Assign action. |
| **Dependabot** | `dependabot.yml` | Keeps GitHub Actions versions current (weekly PRs). |
| **Funding** | `FUNDING.yml` | The "Sponsor this project" button. |
| **Repo setup** | `setup-repo.sh` + `RUN_COMMANDS.md` | One-time repo configuration (labels, settings, projects, branch protection) and the exact commands used. |
| **Releases** | GitHub Releases | Versioned artifacts + changelog (see section 3). |
| **Security tab** | Code scanning alerts | CodeQL findings per branch: *Security → Code scanning*. |
| **Insights** | Repo → Insights | Dependency graph, commit activity, CI timing. |

---

## 5. Local equivalents (the same things by hand)

```bash
# Build & test (what build.yml does)
cmake -S . -B cmake-build-debug && cmake --build cmake-build-debug
ctest --test-dir cmake-build-debug --output-on-failure

# The live run (what live-run.yml does)
xvfb-run -a ./cmake-build-debug/client/matticraft \
  --launchTarget neoforge --gameDir .

# Build the mods (the "build mods" run configuration)
cmake --build cmake-build-debug --target build_mods

# Pack a mod
./tools/matti-pack.sh mods/examplemod-1.0.so dist/examplemod-1.0.matti examplemod 1.0
```

Everything runs inside the repository — no artifacts outside the project
directory, no external test sources.
