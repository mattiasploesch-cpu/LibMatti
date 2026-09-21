# tests - the harnesses

The harnesses are part of the root project (no second build tree, no /tmp).
They link the `matti` target like the client does, build into the normal build
folder and are executed by the client itself: the matticraft binary has a
selftest phase that runs every harness before the launcher starts.

    tests/
      modscan/            the class-file scan harness (ModScanHarness.c)
                          and the .so scan harness (SoScanHarness.c)
      mixinfacade/        the MixinFacade harness + fixture mods
      mixin/m4proof/      the M4 hook table + M2 GOT patch proof (non-PIE)

The fixture mods are built with the same `matti_add_mod` helper the real mods
use (shared through `mods/cmake/MattiMod.cmake`), so a test that passes here
exercises the exact mechanics the game sees.

## Run configurations (CLion, .idea/runConfigurations)

| Run configuration | What it does |
|---|---|
| **matticraft** | The full configuration: builds the library, the client, every harness with its fixture mods and the mods project, then runs the client. The selftest phase executes all four harnesses (same commands as ctest); only when all pass does the launcher start. `--no-tests` skips the phase. |
| **runClient** | The plain client: builds the library, the client binary and the mods project - no test harnesses - and starts directly (the selftest phase is compiled out via `MATTICRAFT_CLIENT_ONLY`). Same launcher arguments as matticraft. |
| **build mods** | Builds only the mods project (`cmake --build cmake-build-debug --target build_mods`). Output: `mods/*.so`. |

## Commands behind the configurations

The matticraft configuration is exactly:

    cmake --build cmake-build-debug --target matticraft
    cd <repo root> && ./cmake-build-debug/client/matticraft \
        --launchTarget neoforge --gameDir . --version 1.21.11 \
        --fml.neoForgeVersion 21.11.45 --fml.mcVersion 1.21.11 \
        --fml.neoFormVersion 20251209.172050

The runClient configuration is exactly:

    cmake --build cmake-build-debug --target runClient
    cd <repo root> && ./cmake-build-debug/runClient \
        --launchTarget neoforge --gameDir . --version 1.21.11 \
        --fml.neoForgeVersion 21.11.45 --fml.mcVersion 1.21.11 \
        --fml.neoFormVersion 20251209.172050

with `libraryDirectory` pointing at the Prism libraries and `legacyClassPath`
at the demo mods (both set in the run configurations).

The build mods configuration is exactly:

    cmake -S mods -B mods/build && cmake --build mods/build

## The harnesses

| Harness | What it checks |
|---|---|
| **modscan** | Scans real javac output (`tests/modscan/out/**/*.class`) with the port's ASM + `ModClassVisitor` and checks every value FML reads later: `@Mod`, `@EventBusSubscriber`, `@SubscribeEvent` (method/field), the nested-annotation rendering and the ClassData entries. |
| **soscan** | Reads the fixture mod `classmod-fixture.so` through `JarContents` and runs the same class scan the discovery `Scanner` runs - proving ELF-section entries reach the scan as `.class` entries (the fixture's `example/ExampleMod.class` is appended with `objcopy`, exactly how real mods carry compiled classes). |
| **mixinfacade** | Builds a `LoadingModList` from two real fixture mods and runs `MixinFacade.finishInitialization`: the valid config registers, the four broken ones produce exactly one `missing_config` / `duplicate_config` / `requested_behavior_too_new` / `requested_behavior_too_old` loading issue each. |
| **m4proof** | The mixin backends end to end, non-PIE on purpose: collects the executable's descriptors (2) and the fixture's hook (1), resolves the chain, dispatches (hook runs, body continues), proves HEAD cancellation, then patches the executable's own GOT (`puts`) and calls through the PLT to prove the M2 rewrite with original re-invocation. |

## ctest

The same four harnesses are registered with ctest, so CI-style runs work too:

    cmake -S . -B cmake-build-debug
    ctest --test-dir cmake-build-debug --output-on-failure

Rebuilding the checked-in class fixtures (only needed if the sources change):
needs a JDK on the PATH, then from `tests/modscan`:

    javac -parameters -d out $(find src -name '*.java')
