<div align="center">

# LibMatti / Matticraft

**A from-scratch C port of the Minecraft 1.21.11 + NeoForge toolchain.**

The JVM class-loading pipeline — bootstrap, modules, FML, ModLauncher, mixins,
registries, rendering — rebuilt as a single C11 library (`libmatti`), driven by
a thin client, with native `.so` mods instead of `.jar` mods.

</div>

## What is this?

Minecraft's modding stack is a tower of Java machinery: `BootstrapLauncher`
turns the classpath into modules, SecureJarHandler wraps jars, ModLauncher
plugs transformation services in, FML discovers and loads mods, mixins weave
hooks into bytecode, and the registries bring blocks and items to life.

LibMatti ports that tower **one layer at a time, 1:1 against the upstream
sources** (vendored under `vendor/`), but in C:

| Layer | Java | Port |
|---|---|---|
| Bootstrap | `cpw.mods.bootstraplauncher` | `libmatti/bsl/BootstrapLauncher` |
| SecureJarHandler | `cpw.mods.jarhandling`, `cpw.mods.cl` | `libmatti/bsl/sjh` |
| ModLauncher | `cpw.mods.modlauncher` | `libmatti/cpw/modlauncher` |
| FML | `net.neoforged.fml.*` | `libmatti/net/neoforged/fml` |
| Mixins | sponge-mixin (M2/M4 backends) | `libmatti/matti/mixin` |
| Registry | `net.minecraft.core.*` | `libmatti/net/minecraft/core` |
| Render | blaze3d + GL/GLFW via dlopen | `libmatti/com/mojang/blaze3d` |

There is no JVM. Mods are **native shared objects** that export a small
entry-point surface; the FML port scans `mods/` for `.so` files, reads their
embedded metadata and drives them through the same lifecycle events as Java
FML (construct, common setup, load complete, …).

## Status

- ✅ Full FML boot sequence: discovery → language providers → containers →
  EventBus → lifecycle events → config tracker → hand-over to the game class
- ✅ Mixin backends M4 (hook table in ELF sections) and M2 (GOT patching)
- ✅ Vanilla registry bootstrap (1142 blocks, items, …) with `RegisterEvent`
- ✅ Client skeleton: GLFW window, GL render loop, texture atlas, font
  rendering (Monocraft via stb_truetype), early-display framebuffer pipeline
- 🚧 World/render port in progress (chunk meshing, models — see `plan.md`)

The full roadmap lives in [`plan.md`](plan.md); it tracks what is ported,
what is verified and what is next.

## Building

Requirements: CMake ≥ 3.20, a C11 compiler, zlib (optional, for java.util.zip),
a GL driver (runtime only, loaded via dlopen) and GLFW/wayland or X11 dev
libraries at runtime for the window.

```bash
cmake -S . -B cmake-build-debug
cmake --build cmake-build-debug -j
```

That single build produces the library, the client, every test harness and the
demo mods.

### Run configurations (CLion)

The project ships ready-made run configurations (`.idea/runConfigurations/`):

- **runClient** — the plain client boot (library + FML + mods → game hand-off),
  no test harnesses.
- **matticraft** — the full selftest run: every harness, then the launcher.
- **build mods** — builds the separate `mods/` project into `mods/*.so`.

Command-line equivalents are documented in
[`.github/RUN_COMMANDS.md`](.github/RUN_COMMANDS.md).

## Tests

Every harness is registered with CTest:

```bash
ctest --test-dir cmake-build-debug --output-on-failure
```

The harnesses live under `tests/`; the client's `--no-tests` flag skips the
in-client selftest phase (used by `runClient`).

## Making a mod

Mods are standalone CMake projects under `mods/`. Create a folder with a
`CMakeLists.txt` that includes `mods/cmake/MattiMod.cmake`, drop in an entry
point and metadata, and `cmake --build mods/build` produces
`mods/<name>-1.0.so`. The FML port picks it up on the next `runClient` start.
See `mods/example-mod/` for the minimal working example.

## Repository layout

```
library/src/libmatti   the port (bsl, sjh, modlauncher, fml, minecraft, java, …)
client/                the client entry (selftests + launcher start)
tests/                 the harnesses (CTest)
mods/                  the mods sub-project (built separately)
vendor/                upstream Java sources used as the 1:1 reference
tools/                 helper scripts (vendor sync, generation)
```

## License

[MIT](LICENSE) — this repository's own code. The upstream sources under
`vendor/` remain under their original licenses and are used as reference
material only.

<div align="center">

*Not affiliated with Mojang or Microsoft. "Minecraft" is a trademark of Mojang Synergies AB.*

</div>
