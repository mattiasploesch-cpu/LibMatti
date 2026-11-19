# Matticraft Mobile

Android project for the one `libmatti` library.

- `matti-kotlin/` — Android library (`dev.matticraft.libmatti`) with the JNI
  bridge (`src/main/cpp/libmatti_jni.c`). It loads the same C sources the
  desktop builds; `matti/Platform.c` detects Android and the pointer width.
- `mobile_client/` — Compose launcher that starts the client through
  `LibMatti.run(...)`.

Status: scaffold for milestone M2 (see `planported.MD`). The Gradle build is
not wired into the desktop CMake build yet; `libmatti_jni.c` and
`MainActivity.kt` compile once the NDK/Compose toolchains are installed.

Build (after installing the Android SDK/NDK):

```
cd library/src/mobile
./gradlew :mobile_client:assembleDebug
```
