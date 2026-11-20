# Matticraft Mobile

Android project for the one `libmatti` library.

- `matti-kotlin/` — Android library (`dev.matticraft.libmatti`) with the JNI
  bridge (`src/main/cpp/libmatti_jni.c`). It loads the same C sources the
  desktop builds; `matti/Platform.c` detects Android and the pointer width.
- `mobile_client/` — Compose launcher that starts the client through
  `LibMatti.run(...)`.

Status: the launcher is portrait-first; Play starts `GameActivity`, which owns a
landscape `SurfaceView`. `libmatti_jni.c` hands that surface to the Android EGL/
GLES3 renderer, and the launcher's Material 3 settings are stored in the same
`profile.json` shape as the desktop launcher. The Gradle build is not wired into
the desktop CMake build yet; the NDK/Compose toolchains are supplied by CI.

Build (after installing the Android SDK/NDK):

```
cd library/src/mobile
./gradlew :mobile_client:assembleDebug
```
