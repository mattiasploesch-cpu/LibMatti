# Run / build commands

The exact commands behind the CLion run configurations and the CI workflows -
nothing is done outside the repository directory.

## Configure + build

```bash
cmake -S . -B cmake-build-debug
cmake --build cmake-build-debug -j
```

## Tests (all harnesses, correct working dirs)

```bash
ctest --test-dir cmake-build-debug --output-on-failure
```

## runClient

The plain client boot - library + FML + demo mods, hand-over to the game class
(what CLion's `runClient` configuration runs):

```bash
cmake --build cmake-build-debug --target runClient
./cmake-build-debug/runClient \
  --launchTarget neoforge --gameDir . \
  --version 1.21.11 --fml.neoForgeVersion 21.11.45 \
  --fml.mcVersion 1.21.11 --fml.neoFormVersion 20251209.172050 \
  --no-tests
```

Environment (as set in the run configuration):

```bash
export libraryDirectory="$HOME/.local/share/PrismLauncher/libraries"
export legacyClassPath="$PWD/client/demo-mods/demo-app-1.0.so:$PWD/client/demo-mods/demo-lib-1.0.so"
```

`--no-tests` skips the in-client selftest phase (runClient does not link the
harness binaries).

## matticraft (full selftest run)

```bash
cmake --build cmake-build-debug --target matticraft
./cmake-build-debug/client/matticraft \
  --launchTarget neoforge --gameDir . --no-tests
```

## build mods

The mods sub-project builds standalone; its output lands in `mods/*.so` where
`--gameDir .` finds it:

```bash
cmake --build cmake-build-debug --target build_mods
# equivalent standalone form:
cmake -S mods -B mods/build
cmake --build mods/build
```

## Headless client run (CI)

The GL/window paths degrade gracefully without a display; for a real window
CI uses xvfb:

```bash
xvfb-run -a ./cmake-build-debug/runClient --launchTarget neoforge \
  --gameDir . --version 1.21.11 --fml.neoForgeVersion 21.11.45 \
  --fml.mcVersion 1.21.11 --fml.neoFormVersion 20251209.172050 --no-tests
```
