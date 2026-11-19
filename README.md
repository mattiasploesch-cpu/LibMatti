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

Requirements: CMake ≥ 3.20, a C11 compiler, a Rust toolchain (`cargo`, builds
the Rust part under `library/src/libmatti/rs`), zlib (development headers and
library), a GL driver (runtime only, loaded via dlopen) and GLFW/wayland or X11
dev libraries at runtime for the window.

```bash
cmake -S . -B cmake-build-debug
cmake --build cmake-build-debug -j
```

That single build produces the library, the client, every test harness and the
demo mods.

The configure step checks the host like `./configure` does: it prints a summary
of the detected headers, functions and libraries (`dlopen`, `pthread_create`,
`inotify_init1`, `getrandom`, `strndup`, `mmap`, zlib, …) and writes the result
to `config.h` in the build directory.

### NixOS (Home Manager, no nix-shell)

CMake detects NixOS through `/etc/os-release` and uses GCC from
`/run/current-system/sw/bin` automatically — `/usr/bin/gcc` does not exist on
NixOS. NixOS does not install development headers globally, so use the modular
Home Manager module in `nix/home.nix`. It provides CMake, GCC, binutils,
pkg-config and zlib (headers from the `dev` output plus the shared library).

Copy the module into your NixOS configuration (flake evaluation cannot import
absolute paths outside the flake):

```bash
sudo cp /path/to/LibMatti/nix/home.nix /etc/nixos/matticraft-dev.nix
```

Then add it to the Home Manager modules of your flake, without touching the
rest of your configuration:

```nix
# /etc/nixos/flake.nix
homeConfigurations.administrator.modules = [
  ./home.nix
  ./matticraft-dev.nix
  # ... your other modules
];
```

For a classic `home.nix`, the equivalent is:

```nix
imports = [
  ./matticraft-dev.nix
];
```

Apply it and build normally — no `nix-shell`:

```bash
home-manager switch --flake /etc/nixos#administrator
cmake -S . -B cmake-build-debug
cmake --build cmake-build-debug -j
```

CMake adds the Home Manager profile (`~/.nix-profile`) to `CMAKE_PREFIX_PATH`
itself, so `find_package(ZLIB)` and the configure checks find the headers and
library installed by the module.

### NixOS flake (no AppImage needed)

The flake exposes the standalone library and the client directly from source:

```bash
nix build .#matticraft        # client binaries in ./result/bin
nix build .#libmatti          # libmatti.so* in ./result/lib
nix build .#libmatti-dev      # public headers in ./result/include

nix profile install .#matticraft
nix profile install .#libmatti

nix develop                   # CMake, Rust and the runtime libraries
```

`search.nix.org` indexes nixpkgs, not arbitrary flakes. The flake packages work
immediately; the nixpkgs names are `matticraft`, `libmatti` and the `dev` output
of `libmatti`. Making them appear on `search.nix.org` therefore requires a
merged nixpkgs pull request.

`libmatti` is also published as a standalone tarball
(`libmatti-<version>-linux-<arch>.tar.zst`) next to the AppImage, deb and rpm.
Install it into `/usr/local` with:

```bash
tar --zstd -xf libmatti-0.1.0-linux-x86_64.tar.zst -C /usr/local
```

### Run configurations (CLion)

The project ships ready-made run configurations (`.idea/runConfigurations/`):

- **runClient** — the plain client boot (library + FML + mods → game hand-off),
  no test harnesses.
- **matticraft** — the full selftest run: every harness, then the launcher.
- **build mods** — builds the separate `mods/` project into `mods/*.so`.

Command-line equivalents are documented in
[`.github/RUN_COMMANDS.md`](.github/RUN_COMMANDS.md).

## Rendering backends

Drawing goes through one CPU-side interface in
`library/src/libmatti/matti/render/render.h`: a framebuffer of `0xAARRGGBB`
pixels (top-down) with `LIBMATTI_Render_Clear`, `DrawPixel`, `GetPixel`,
`FillRect`, `DrawLine`, `Present`, resize and event polling. A backend only
creates the window, pumps events and uploads/presents that framebuffer, so the
same drawing code runs on every backend:

| Backend | Source | Notes |
| --- | --- | --- |
| OpenGL | `matti/render/client_opengl.c` | GLFW + OpenGL 3.2 core; streaming ARGB texture |
| Vulkan | `matti/render/client_vulkan.c` | GLFW `NO_API` window, swapchain, staging + blit present |
| SDL3 | `matti/render/client_sdl.c` | SDL window/renderer, driver picked from the API |

Select the layer and API at runtime. The platform default is detected by
`LibMatti` itself; `auto` lets the library pick the first usable backend:

```bash
./build/runClient --layer native --api vulkan
./build/runClient --layer native --api opengl
./build/runClient --layer sdl    --api vulkan
./build/runClient --layer sdl    --api opengl
./build/runClient --layer sdl    --api directx   # Windows only
./build/runClient --layer sdl    --api metal     # macOS only
./build/runClient --layer sdl    --api auto      # SDL chooses
./build/runClient --layer auto   --api auto      # first usable backend

MATTI_LAYER=sdl MATTI_API=vulkan ./build/runClient   # environment
LIBMATTI_VK=TRUE ./build/runClient                    # legacy alias for vulkan
```

`--layer` and `--api` are the only selection switches; there is no
`--renderer` shorthand. `MATTI_RENDERER=layer:api` still works as an
environment fallback. The options screen edits the same selection and restarts
the window in place.

The SDL3 and Vulkan backends are optional at build time. CMake enables them
when the `sdl3` / `vulkan` pkg-config modules (or the Vulkan CMake package) are
available; otherwise they simply report themselves as unavailable.

## Launcher

`matticraft-launcher` is the entry point of every package. It is a Rust
workspace under `launchers/`:

| Crate | Target | Notes |
| --- | --- | --- |
| `launcher-core` | all frontends | profiles, accounts, mods, logs, update manifests |
| `launcher-desktop` | Linux/Windows | egui UI, `--check`, `--launch`, `--ui-once` |
| `launcher-macos` | macOS `.app` | thin wrapper around the desktop crate |

The launcher stores `profile.json` per instance and asks `libmatti`
(`LIBMATTI_Platform_DefaultRenderer`) for the renderer preset when the library
is installed next to it. Profiles start the game with `--layer`/`--api` and
`MATTI_LAYER`/`MATTI_API`.

```bash
cmake --build build --target launcher      # cargo build + install rules
launchers/target/debug/matticraft-launcher --check
launchers/target/debug/matticraft-launcher --launch
```

Android ships the same features as a Kotlin/Compose frontend inside
`mobile_client`; it calls the shared `libmatti` through JNI and uses the same
`profile.json` layout.

Update manifests are signed with Ed25519. Generate a key once per release
channel and pass the public key when building the launcher:

```bash
launchers/target/debug/matticraft-update-sign generate --key update.pk8
cmake -S . -B build -DMATTI_UPDATE_PUBLIC_KEY=<hex public key>
```

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
library/src/mobile     the Android library and Compose launcher frontend
client/                the client entry (selftests + launcher start)
launchers/             Rust launcher workspace (core, desktop, macOS)
cmake/                 platform, native-bundling, Rust and launcher modules
packaging/             deb/rpm/AppImage/Flatpak, MSI/ZIP, DMG/PKG, APK/AAB, FreeBSD
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
