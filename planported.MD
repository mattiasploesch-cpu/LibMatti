
# Planported — Matticraft/LibMatti: eine Lib, alle Plattformen

Version 1.0 · 18.11.2026 · Branch `vulkan_test` · Ziel: eine C-Bibliothek, die ihre Plattform und Bitness selbst erkennt, alle Renderer enthält und auf Linux, Windows, macOS und Android mit derselben Codebasis läuft.

Dieses Dokument ist die vollständige Arbeits-, Architektur- und Abnahme-spezifikation. Es beschreibt die grobe Infrastruktur bis zu den feinen Details: Renderer-Layer, API-Matrix, Plattformerkennung, eine einzige Bibliotheksdatei, Kotlin-Bindung, Mobile-Client, Launcher, Build-Presets, Packaging, GitHub-Releases und Tests.

Statuslegende: ✅ umgesetzt · 🟡 Gerüst · ⬜ geplant. Jede Zeile der Arbeitsliste ist eine überprüfbare Aufgabe; die Abnahmekriterien am Ende sind nummeriert und werden in der PR-Beschreibung referenziert.


## Inhaltsverzeichnis

- 1. Vision und Grundsätze
- 2. Ist-Zustand
- 3. Eine Datei: was technisch geht und was nicht
- 4. Architektur der einen Bibliothek
- 5. Renderer: Layer zuerst, dann API
- 6. Plattform- und Bitnesserkennung
- 7. Repository-Struktur
- 8. C-API-Referenz
- 9. Rust-Fenster
- 10. Android: Kotlin-Lib und Mobile-Client
- 11. Launcher für alle Plattformen
- 12. Build-System und Presets
- 13. Packaging-Matrix
- 14. GitHub Actions und Releases
- 15. Tests und Qualitätssicherung
- 16. Sicherheit und Signierung
- 17. Meilensteine
- 18. Datei-für-Datei-Arbeitsliste
- 19. Risiken und offene Entscheidungen
- 20. Anhänge: CLI, Umgebungsvariablen, Formate
- 21. Nummerierte Abnahmekriterien


## 1. Vision und Grundsätze

Matticraft ist ein C-Port der Minecraft/NeoForge-Toolchain. C bleibt die API und besitzt Rendering, Texturen, Blöcke, Menüs und Welt. Rust besitzt ausschließlich das echte Fenster. Kotlin bindet dieselbe C-Bibliothek auf Android an. Es gibt keine zweite UI-Implementierung.

- Eine Codebasis: `library/src/libmatti` wird für jedes Ziel kompiliert.
- Eine Bibliotheksdatei pro Ziel: `libmatti.so`, `matti.dll` oder `libmatti.dylib`; alle Renderer sind enthalten.
- Keine Renderer-Sonderbauten: Layer und API sind Laufzeitentscheidungen.
- Plattform und Bitness werden in der Bibliothek erkannt, nicht vom Aufrufer gesetzt.
- Ein Dateiname, ein Ladepfad, ein API-Satz; die Datei darf groß werden.
- Kein stiller Fallback bei expliziter Wahl; Fehler nennen Layer, API, Plattform und Alternativen.
- Alle Assets (Panorama, Logo, Widgets, Dirt, Font) reisen in der Datei.
- Alle Pakete entstehen aus derselben Version und demselben Commit.


## 2. Ist-Zustand

| Bereich | Status | Ort |
|---|---|---|
| C-UI mit Vanilla-Look | ✅ | library/src/libmatti/matti/ui/menu.c |
| Textur-Registry + objcopy-Assets | ✅ | matti/ui/texture.c, cmake/MattiNative.cmake |
| Rust nur Fenster | ✅ | library/src/libmatti/rs/src/ui/{mod,ffi}.rs |
| Layer/API-Matrix | ✅ | matti/render/render.c |
| Eine Shared Library (libmatti.so) | ✅ | build/library/libmatti.so (alle Backends + Assets) |
| CMake-Presets | ✅ | CMakePresets.json |
| Plattform-/Bitnesserkennung | ✅ | matti/Platform.c |
| SDL-Driverwahl nach API | ✅ | matti/render/client_sdl.c |
| Options-Screen wechselt Selection | ✅ | matti/ui/menu.c |
| Vendor Horizon | ✅ | vendor/horizon (gitignored) |
| Mobile-Gradle-Projekt | 🟡 | library/src/mobile |
| Launcher | ⬜ | launchers |
| Packaging-Rezepte | 🟡 | packaging/linux, packaging/windows, packaging/macos |
| Release-Workflow | 🟡 | .github/workflows/portable-release.yml |

Verifiziert auf diesem Host: Linux x86_64, Vulkan/OpenGL/SDL3 vorhanden, Titel-/Options-/Pause-Screens per `--ui-once` gerendert, voller Loop läuft.


## 3. Eine Datei: was technisch geht und was nicht

Ein natives Programm kann nicht als dieselbe Binärdatei auf Linux, Windows, macOS und Android laufen: ELF, PE/COFF und Mach-O sind verschiedene Formate, Android braucht eine andere C-Laufzeit und eine andere Einstiegsschnittstelle. „Immer die gleiche Datei“ wird deshalb so umgesetzt, wie es technisch korrekt möglich ist:

1. Ein Quellbaum und eine Bibliotheksdatei pro Zielplattform, immer unter demselben Namen `libmatti` (bzw. `matti` auf Windows).
2. In jeder dieser Dateien stecken alle Layer und APIs; die Auswahl passiert zur Laufzeit.
3. Für macOS wird eine Universal-Datei gebaut (arm64 + x86_64 in einem Mach-O).
4. Für die Auslieferung entsteht zusätzlich genau eine Portable-Datei pro OS: AppImage (Linux), MSI/ZIP (Windows), DMG (macOS), APK (Android).
5. Optional entsteht ein selbstentpackendes `.matti`-Bundle, das die Payloads aller Ziele in einer Datei trägt und beim Start OS/Arch wählt; das ist ein Container, keine ausführbare Universal-Binärdatei.

Damit gilt: derselbe Dateiname, derselbe Code, dieselbe API, dieselbe Selection — aber ein Container pro Ziel-OS. Ein echtes Cross-OS-Executable wird nicht versprochen, weil es das nicht gibt.


## 4. Architektur der einen Bibliothek

Die Bibliothek besteht aus Schichten, die alle in derselben Datei landen:

| Schicht | Aufgabe | Plattformabhängig? |
|---|---|---|
| core | Speicher, Strings, Logging | nein |
| registry | Blöcke, Items, Ressourcen-Keys | nein |
| assets | PNG-Decode, Pack-Suche, eingebettete Sections | nein |
| ui | Screens, Widgets, Panorama, Text | nur Fenster/Input |
| render | CPU-Framebuffer + Backend-Ops | ja, per Backend |
| platform | OS/Arch/Bits, APIs, Defaults | ja |
| bridge | C-API für Rust/Kotlin | nein |
| rust | Fensterloop, Restart | ja, pro Target |

Der Aufrufer sieht nur die öffentlichen Header und eine Selection. Alles andere ist Implementierung derselben Datei.

- Kein `#ifdef` in Fachcode außer in Plattform- und Backend-Dateien.
- Optionale Laufzeitbibliotheken werden per `dlopen`/`LoadLibrary` geladen.
- Fehlt eine Bibliothek, nennt der Fehler den gesuchten Namen und die Pfade.
- Kein Feature-Flag für Renderer im Build; nur Laufzeitwahl.
- Die Bibliothek protokolliert beim Start Plattform, Arch, Bitness und Backend.


## 5. Renderer: Layer zuerst, dann API

Der Spieler wählt zuerst den Layer, dann die API. Der Layer bestimmt, wem das Fenster gehört: der Port selbst (native) oder SDL3 (sdl). Die API bestimmt darunter OpenGL, Vulkan oder DirectX.

| Layer | API | Fenster | Umsetzung | Plattformen |
|---|---|---|---|---|
| native | opengl | GLFW | GLFW + GL 3.2 core | linux/windows/macos |
| native | vulkan | GLFW | GLFW NO_API + Vulkan swapchain | linux/windows/macos |
| sdl | opengl | SDL3 | SDL3 renderer driver opengl | linux/windows/macos/android |
| sdl | vulkan | SDL3 | SDL3 renderer driver vulkan | linux/windows/macos/android |
| sdl | directx | SDL3 | SDL3 direct3d12/11/9 | windows |
| sdl | auto | SDL3 | SDL picks the best driver | linux/windows/macos/android |


### 5.1 Auswahlregeln

1. Format ist `layer:api`; ein einzelner Name ist eine Abkürzung (`vulkan` = `native:vulkan`).
2. `directx` ist nur auf Windows und nur unter `sdl` erlaubt.
3. `native:directx` wird mit einer klaren Meldung abgelehnt.
4. `sdl:auto` lässt SDL den Treiber wählen (macOS: Metal).
5. `auto` bleibt der historische Pfad: native GL, dann native Vulkan, dann SDL.
6. Ein expliziter Fehlschlag fällt nicht still zurück.
7. Die Wahl wird im Options-Screen angezeigt und per Klick gewechselt.
8. Ein Wechsel öffnet das Fenster neu; die Selection wird als String übergeben.
9. Die Selection wird geloggt und im Crash-Report gespeichert.
10. DirectX-Treiberreihenfolge: direct3d12, direct3d11, direct3d.


### 5.2 Startreihenfolge

1. `LIBMATTI_Render_ParseSelection` validiert die Tokens.
2. `LIBMATTI_Platform_SupportsApi` prüft die API auf dieser Plattform.
3. `LIBMATTI_Render_InitSelection` mappt Layer/API auf Backend-Ops.
4. `is_available()` prüft, ob der Backend-Code einkompiliert und ladbar ist.
5. Backend-Init öffnet Fenster und Kontext (GLFW oder SDL).
6. SDL wählt den Render-Driver nach API.
7. Bei Erfolg wird die Selection im Renderer-Config gespeichert.
8. Bei Fehler wird eine Meldung mit Layer, API, Plattform, Backendliste gesetzt.


### 5.3 native:opengl

GLFW + GL 3.2 core. Benötigt: GLFW/GL/libGL. Verfügbar auf: linux/windows/macos.

- Start: `--layer native --api opengl` oder `MATTI_LAYER=native MATTI_API=opengl`.
- Erfolg: Fenster öffnet, ein Frame wird präsentiert, Options zeigt die Selection.
- Fehler: Exit ungleich 0, Log nennt den Grund, kein schwarzes Fenster.
- Test: `--ui-once --ui-screen main` und Screenshot-Vergleich.
- CI: eigener Job pro Plattform und Kombination, soweit der Runner es kann.


### 5.4 native:vulkan

GLFW NO_API + Vulkan swapchain. Benötigt: GLFW/libvulkan. Verfügbar auf: linux/windows/macos.

- Start: `--layer native --api vulkan` oder `MATTI_LAYER=native MATTI_API=vulkan`.
- Erfolg: Fenster öffnet, ein Frame wird präsentiert, Options zeigt die Selection.
- Fehler: Exit ungleich 0, Log nennt den Grund, kein schwarzes Fenster.
- Test: `--ui-once --ui-screen main` und Screenshot-Vergleich.
- CI: eigener Job pro Plattform und Kombination, soweit der Runner es kann.


### 5.5 sdl:opengl

SDL3 renderer driver opengl. Benötigt: SDL3/libGL. Verfügbar auf: linux/windows/macos/android.

- Start: `--layer sdl --api opengl` oder `MATTI_LAYER=sdl MATTI_API=opengl`.
- Erfolg: Fenster öffnet, ein Frame wird präsentiert, Options zeigt die Selection.
- Fehler: Exit ungleich 0, Log nennt den Grund, kein schwarzes Fenster.
- Test: `--ui-once --ui-screen main` und Screenshot-Vergleich.
- CI: eigener Job pro Plattform und Kombination, soweit der Runner es kann.


### 5.6 sdl:vulkan

SDL3 renderer driver vulkan. Benötigt: SDL3/libvulkan. Verfügbar auf: linux/windows/macos/android.

- Start: `--layer sdl --api vulkan` oder `MATTI_LAYER=sdl MATTI_API=vulkan`.
- Erfolg: Fenster öffnet, ein Frame wird präsentiert, Options zeigt die Selection.
- Fehler: Exit ungleich 0, Log nennt den Grund, kein schwarzes Fenster.
- Test: `--ui-once --ui-screen main` und Screenshot-Vergleich.
- CI: eigener Job pro Plattform und Kombination, soweit der Runner es kann.


### 5.7 sdl:directx

SDL3 direct3d12/11/9. Benötigt: SDL3/d3d12/d3d11. Verfügbar auf: windows.

- Start: `--layer sdl --api directx` oder `MATTI_LAYER=sdl MATTI_API=directx`.
- Erfolg: Fenster öffnet, ein Frame wird präsentiert, Options zeigt die Selection.
- Fehler: Exit ungleich 0, Log nennt den Grund, kein schwarzes Fenster.
- Test: `--ui-once --ui-screen main` und Screenshot-Vergleich.
- CI: eigener Job pro Plattform und Kombination, soweit der Runner es kann.


### 5.8 sdl:auto

SDL picks the best driver. Benötigt: SDL3. Verfügbar auf: linux/windows/macos/android.

- Start: `--layer sdl --api auto` oder `MATTI_LAYER=sdl MATTI_API=auto`.
- Erfolg: Fenster öffnet, ein Frame wird präsentiert, Options zeigt die Selection.
- Fehler: Exit ungleich 0, Log nennt den Grund, kein schwarzes Fenster.
- Test: `--ui-once --ui-screen main` und Screenshot-Vergleich.
- CI: eigener Job pro Plattform und Kombination, soweit der Runner es kann.


## 6. Plattform- und Bitnesserkennung

`matti/Platform.c` erkennt die Plattform zur Compile-Zeit und beantwortet Architektur, Bitness und Endianness zur Laufzeit. Die Bibliothek braucht keine Angabe des Aufrufers.

| Abfrage | Rückgabe | Quelle |
|---|---|---|
| Platform_Get | linux/windows/macos/android/freebsd/unknown | Präprozessor |
| Platform_Arch | x86_64/x86/arm64/arm/riscv64 | Compiler-Makros |
| Platform_Bits | 32 oder 64 | sizeof(void*) |
| Platform_IsLittleEndian | 0/1 | Laufzeit-Probe |
| SupportsApi(directx) | nur Windows | Plattformregel |
| DefaultRenderer | layer:api | Plattform + Bitness |

Bitness wird bewusst zur Laufzeit über die Pointerbreite bestimmt: ein 32-Bit-Prozess auf einem 64-Bit-Windows (WOW64) oder ein x86-Android meldet korrekt 32 Bit.

- Android wird vor Linux geprüft (`__ANDROID__` vor `__linux__`).
- macOS erkennt auch Universal-Builds korrekt; die Arch ist die laufende Hälfte.
- FreeBSD nutzt den Linux-Pfad, meldet sich aber als freebsd.
- Unbekannte Plattformen starten headless und melden den Grund.


## 7. Repository-Struktur

Die folgenden Pfade sind die verbindliche Zielstruktur. Vorhandene Pfade sind bereits im Baum; geplante Pfade werden in den Meilensteinen angelegt.

- `CMakeLists.txt`
- `CMakePresets.json`
- `planported.MD`
- `README.md`
- `cmake/MattiNative.cmake`
- `cmake/MattiRust.cmake`
- `cmake/MattiPlatform.cmake`
- `cmake/MattiLauncher.cmake`
- `library/src/libmatti/matti/Platform.h`
- `library/src/libmatti/matti/Platform.c`
- `library/src/libmatti/matti/render/render.h`
- `library/src/libmatti/matti/render/render.c`
- `library/src/libmatti/matti/render/client_opengl.c`
- `library/src/libmatti/matti/render/client_vulkan.c`
- `library/src/libmatti/matti/render/client_sdl.c`
- `library/src/libmatti/matti/ui/menu.h`
- `library/src/libmatti/matti/ui/menu.c`
- `library/src/libmatti/matti/ui/texture.h`
- `library/src/libmatti/matti/ui/texture.c`
- `library/src/libmatti/matti/ui/bridge.h`
- `library/src/libmatti/matti/ui/bridge.c`
- `library/src/libmatti/matti/ui/assets/gui/widgets.png`
- `library/src/libmatti/matti/ui/assets/gui/options_background.png`
- `library/src/libmatti/matti/ui/assets/gui/title/minecraft.png`
- `library/src/libmatti/matti/ui/assets/gui/title/edition.png`
- `library/src/libmatti/matti/ui/assets/gui/title/background/panorama_0.png`
- `library/src/libmatti/matti/ui/assets/font/ascii.png`
- `library/src/libmatti/rs/Cargo.toml`
- `library/src/libmatti/rs/src/lib.rs`
- `library/src/libmatti/rs/src/ui/mod.rs`
- `library/src/libmatti/rs/src/ui/ffi.rs`
- `library/src/mobile/settings.gradle.kts`
- `library/src/mobile/build.gradle.kts`
- `library/src/mobile/matti-kotlin/build.gradle.kts`
- `library/src/mobile/matti-kotlin/src/main/kotlin/dev/matticraft/libmatti/LibMatti.kt`
- `library/src/mobile/matti-kotlin/src/main/cpp/libmatti_jni.c`
- `library/src/mobile/mobile_client/build.gradle.kts`
- `library/src/mobile/mobile_client/src/main/kotlin/dev/matticraft/mobile/MainActivity.kt`
- `launchers/Cargo.toml`
- `launchers/launcher-core/Cargo.toml`
- `launchers/launcher-core/src/lib.rs`
- `launchers/launcher-core/src/profile.rs`
- `launchers/launcher-core/src/store.rs`
- `launchers/launcher-core/src/account.rs`
- `launchers/launcher-core/src/mods.rs`
- `launchers/launcher-core/src/logs.rs`
- `launchers/launcher-core/src/update.rs`
- `launchers/launcher-core/src/launch.rs`
- `launchers/launcher-core/src/platform.rs`
- `launchers/launcher-core/src/bin/matticraft-update-sign.rs`
- `launchers/desktop/Cargo.toml`
- `launchers/desktop/src/app.rs`
- `launchers/desktop/src/lib.rs`
- `launchers/desktop/src/main.rs`
- `launchers/macos/Cargo.toml`
- `launchers/macos/src/main.rs`
- `packaging/linux/nfpm.yaml`
- `packaging/linux/appimage.sh`
- `packaging/linux/org.matticraft.Matticraft.yml`
- `packaging/windows/matticraft.wxs`
- `packaging/windows/matticraft-wixl.wxs`
- `packaging/windows/portable.ps1`
- `packaging/windows/build-msi.ps1`
- `packaging/windows/build-msi-linux.sh`
- `packaging/macos/dmg.sh`
- `packaging/macos/pkg.sh`
- `packaging/freebsd/portable.sh`
- `packaging/android/signing.gradle.kts`
- `.github/workflows/portable-release.yml`
- `tools/fetch_vanilla_gui_assets.py`
- `tools/generate_planported.py`


## 8. C-API-Referenz

Alle Funktionen sind `extern "C"`, thread-sicher soweit nicht anders angegeben, und in derselben Bibliothek. Namen folgen `LIBMATTI_<Bereich>_`.


### 8.1 Platform

**`LIBMATTI_Platform_Get`** — returns LIBMATTI_PlatformKind. (compile-time macro selection)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_Platform_Name`** — returns linux/windows/macos/android/freebsd. (used in logs and errors)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_Platform_Arch`** — returns x86_64/x86/arm64/arm/riscv64. (arch detection)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_Platform_Bits`** — returns 32 or 64. (sizeof(void*))

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_Platform_IsLittleEndian`** — returns 0/1. (endianness for asset formats)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_Platform_IsDesktop`** — returns 0/1. (launcher and window policy)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_Platform_SupportsApi`** — checks opengl/vulkan/directx/metal. (menu filtering and validation)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_Platform_DefaultRenderer`** — returns layer:api. (first-start default)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.


### 8.2 Render selection

**`LIBMATTI_Render_DefaultSelection`** — returns auto:auto. (bootstrap)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_Render_ParseSelection`** — parses layer:api. (rejects unknown tokens)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_Render_SelectionString`** — formats layer:api. (logs, options, restart)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_Render_LayerName`** — auto/native/sdl. (display)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_Render_ApiName`** — auto/opengl/vulkan/directx. (display)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_Render_InitSelection`** — opens the window. (validates platform/API)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_Render_GetLayer`** — selection of a renderer. (telemetry)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_Render_GetApi`** — selection of a renderer. (telemetry)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_Render_InitByName`** — compat entry point. (accepts old names)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.


### 8.3 Renderer

**`LIBMATTI_Render_Init`** — backend enum. (native backends only)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_Render_InitOpenGL`** — shortcut. (native:opengl)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_Render_InitVulkan`** — shortcut. (native:vulkan)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_Render_InitSDL3`** — shortcut. (sdl:auto)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_Render_GetBackendName`** — opengl/vulkan/sdl3. (options label)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_Render_AvailableBackends`** — space-separated list. (error messages)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_Render_GetWidth/Height`** — logical size. (UI layout)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_Render_GetWindowSize`** — physical size. (DPI mapping)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_Render_Resize`** — resize framebuffer. (window resize)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_Render_Present`** — upload + swap. (frame end)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_Render_SavePPM`** — screenshot. (UI tests)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.


### 8.4 UI window

**`LIBMATTI_UI_WindowOpen`** — opens the game window. (closes early window first)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_WindowClose`** — destroys renderer. (restart path)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_WindowWidth/Height`** — logical size. (layout)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_BackendName`** — backend in use. (options)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_RequestedRenderer`** — layer:api string. (options restart)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_LastError`** — last failure. (crash screen)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_PollEvents`** — pump input. (loop)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_ShouldClose`** — close flag. (loop)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_RequestClose`** — ask to close. (Quit)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_SetTitle`** — window title. (profile name)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_Time`** — seconds since start. (animation)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.


### 8.5 UI drawing

**`LIBMATTI_UI_Clear`** — fill framebuffer. (background)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_FillRect`** — opaque rect. (panels)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_FillRectBlend`** — alpha rect. (dirt gradient)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_DrawPixel`** — single pixel. (panorama)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_DrawTexture`** — scaled texture. (logo)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_DrawTextureRegion`** — region + tint. (widgets)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_DrawTextureTiled`** — tiled texture. (dirt)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_DrawText`** — Monocraft text. (labels)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_TextWidth/Height`** — text metrics. (centering)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_Present`** — swap. (frame end)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_SaveScreenshot`** — PPM. (CI)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.


### 8.6 Menu

**`LIBMATTI_UI_GuiScale`** — 1..4. (layout)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_SetGuiScale`** — set scale. (options)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_GuiWidth/Height`** — logical size. (layout)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_MainWindow_Reset`** — state reset. (window reopen)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_MainWindow_SetScreen`** — enum screen. (QA)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_MainWindow_SetScreenByName`** — string screen. (CLI)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_MainWindow_CurrentScreen`** — enum. (tests)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_MainWindow_ScreenName`** — string. (logs)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_MainWindow_ShouldClose`** — close flag. (loop)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_MainWindow_Update`** — input edges. (frame)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_MainWindow_Draw`** — draw screen. (frame)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_MainWindow_TakeRestart`** — renderer/fullscreen. (restart)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_DrawMenuBackground`** — dirt. (screens)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_DrawPanorama`** — cube. (title)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_DrawButton`** — widget sheet. (screens)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_DrawButtonCentered`** — convenience. (screens)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_DrawLabelCentered`** — centered text. (screens)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_DrawLabel`** — text. (screens)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.


### 8.7 Texture

**`LIBMATTI_UI_Texture_Load`** — asset root + embedded. (cached)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_Texture_Width/Height`** — dimensions. (layout)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_Texture_Pixels`** — ARGB pixels. (panorama)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_Texture_SetAssetRoot`** — override root. (mobile)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LIBMATTI_UI_Texture_AssetRoot`** — current root. (logs)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.


### 8.8 Rust window

**`matti_rs_init`** — C entry point. (catch_unwind)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`window_open`** — bridge. (Rust FFI)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`main_window_update`** — bridge. (Rust FFI)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`main_window_draw`** — bridge. (Rust FFI)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`main_window_take_restart`** — bridge. (Rust FFI)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.


### 8.9 Mobile

**`LibMatti.run`** — Kotlin entry. (JNI)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LibMatti.setSurface`** — surface handover. (JNI)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LibMatti.resize`** — size handover. (JNI)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LibMatti.pressEscape`** — back button. (JNI)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.

**`LibMatti.shutdown`** — close. (JNI)

- Parameter: siehe Header; ungültige Zeiger werden geprüft.
- Rückgabe: 0/1, Zeiger oder `void`; Fehler über `LastError`.
- Thread: Main/UI-Thread, außer ausdrücklich vermerkt.


## 9. Rust-Fenster

Rust öffnet das Fenster, pumpt Events und ruft die C-Menüs. Es enthält keine UI. Die gesamte Rust-Oberfläche besteht aus zwei Dateien.

- `matti_rs_init(argc, argv)` fängt Panics und gibt einen Exit-Status zurück.
- `LaunchOptions::parse` versteht `--layer`, `--api`, `--ui-screen`, `--ui-once` (kein `--renderer`).
- Die Schleife ruft Poll, Update, Draw, Present und Screenshot.
- Restart: C meldet Renderer/Fullscreen, Rust schließt und öffnet neu.
- Kein Rust-Code kennt Texturen, Buttons oder Screens.


## 10. Android: Kotlin-Lib und Mobile-Client


### 10.1 matti-kotlin

Android-Library `dev.matticraft.libmatti`, die `libmatti` per JNI lädt. Dieselbe C-Bibliothek erkennt Android und arm64/x86_64 selbst.

- `LibMatti.run(args)` blockiert im Spiel-Loop.
- `setSurface`, `resize`, `pressEscape`, `shutdown` sind die Lifecycle-Hooks.
- JNI-Datei liegt unter `src/main/cpp/libmatti_jni.c` und ist im Desktop-Build leer.
- CMake bindet das Hauptprojekt als Subdirectory ein.
- ABIs: arm64-v8a und x86_64; minSdk 26.
- Assets werden aus der APK in den App-Datenordner entpackt.


### 10.2 mobile_client

Compose-App `dev.matticraft.mobile` mit Launcher-Screen und SurfaceView.

- Start-Button ruft `LibMatti.run(arrayOf("--layer", "sdl", "--api", "vulkan"))`.
- Renderer-Preset kommt aus `LibMatti.defaultSelection()` (JNI → `LIBMATTI_Platform_DefaultRenderer`).
- Profilformat `profile.json` wie im Desktop-Launcher, inklusive instance/mods/logs.
- Mods-Ordner pro Profil mit Manifest-Validierung; Crash-Reports lokal.
- Landscape, Fullscreen, ConfigChanges werden selbst behandelt.
- Touch wird zu Maus, Back zu ESC, Controller zu GLFW-Codes.


### 10.3 Android-Build

1. Android SDK 35 und NDK 27 installieren.
2. `cd library/src/mobile && ./gradlew :mobile_client:assembleDebug`.
3. Release: Signing-Config aus Secrets, `assembleRelease` und `bundleRelease`.
4. APK-Größe prüfen; Assets und Native-Libs pro ABI.
5. Instrumentationstest startet die Activity und prüft ein Frame-Log.


## 11. Launcher für alle Plattformen

Ein gemeinsamer Launcher-Core in Rust, drei Frontends. Der Core enthält Profile, Accounts, Versionen, Mods, Logs und Updates; die Frontends sind dünn und teilen das Profilformat.

| Frontend | Technik | Ziel | Besonderheiten |
|---|---|---|---|
| Desktop | Rust + egui | Linux/Windows | ein Binary, modernes Theme, keine Web-Abhängigkeit |
| macOS | Rust + egui + App-Bundle | macOS 12+ | Universal2, Notarisierung, Menüleiste |
| Android | Kotlin + Compose | Android 8+ | gleiche Features, Start in mobile_client |

- Profilformat `profile.json` mit Name, Version, Renderer-Selection, Pfaden.
- Renderer-Preset kommt aus `LIBMATTI_Platform_DefaultRenderer`.
- Update-Manifest signiert; SHA-256 pro Artefakt.
- Logs und Crash-Reports lokal, Upload opt-in.
- Mods-Ordner pro Instanz, Validierung über Manifest.


## 12. Build-System und Presets

Ein CMake-Baum, Presets pro Ziel. Die Rendererwahl ist kein Build-Flag.

| Preset | Toolchain | Ausgabe |
|---|---|---|
| linux-x64 | clang/gcc | build/linux-x64 |
| linux-x86 | clang/gcc | build/linux-x86 |
| linux-arm64 | clang/gcc | build/linux-arm64 |
| windows-msvc-x64 | MSVC | build/windows-msvc-x64 |
| windows-msvc-arm64 | MSVC | build/windows-msvc-arm64 |
| windows-msvc-x86 | MSVC | build/windows-msvc-x86 |
| macos-universal | Apple clang (arm64+x86_64) | build/macos-universal |
| android-arm64 | NDK clang | build/android-arm64 |
| android-x64 | NDK clang | build/android-x64 |
| freebsd-x64 | clang/gcc | build/freebsd-x64 |


### 12.1 linux-x64

- Configure: `cmake --preset linux-x64`.
- Build: `cmake --build --preset linux-x64`.
- Test: `ctest --preset linux-x64`.
- Install: `cmake --install build/linux-x64 --prefix staging/linux-x64`.
- Paketierung: Job in `.github/workflows/portable-release.yml`.


### 12.2 linux-x86

- Configure: `cmake --preset linux-x86`.
- Build: `cmake --build --preset linux-x86`.
- Test: `ctest --preset linux-x86`.
- Install: `cmake --install build/linux-x86 --prefix staging/linux-x86`.
- Paketierung: Job in `.github/workflows/portable-release.yml`.


### 12.3 linux-arm64

- Configure: `cmake --preset linux-arm64`.
- Build: `cmake --build --preset linux-arm64`.
- Test: `ctest --preset linux-arm64`.
- Install: `cmake --install build/linux-arm64 --prefix staging/linux-arm64`.
- Paketierung: Job in `.github/workflows/portable-release.yml`.


### 12.4 windows-msvc-x64

- Configure: `cmake --preset windows-msvc-x64`.
- Build: `cmake --build --preset windows-msvc-x64`.
- Test: `ctest --preset windows-msvc-x64`.
- Install: `cmake --install build/windows-msvc-x64 --prefix staging/windows-msvc-x64`.
- Paketierung: Job in `.github/workflows/portable-release.yml`.


### 12.5 windows-msvc-arm64

- Configure: `cmake --preset windows-msvc-arm64`.
- Build: `cmake --build --preset windows-msvc-arm64`.
- Test: `ctest --preset windows-msvc-arm64`.
- Install: `cmake --install build/windows-msvc-arm64 --prefix staging/windows-msvc-arm64`.
- Paketierung: Job in `.github/workflows/portable-release.yml`.


### 12.6 windows-msvc-x86

- Configure: `cmake --preset windows-msvc-x86`.
- Build: `cmake --build --preset windows-msvc-x86`.
- Test: `ctest --preset windows-msvc-x86`.
- Install: `cmake --install build/windows-msvc-x86 --prefix staging/windows-msvc-x86`.
- Paketierung: Job in `.github/workflows/portable-release.yml`.


### 12.7 macos-universal

- Configure: `cmake --preset macos-universal`.
- Build: `cmake --build --preset macos-universal`.
- Test: `ctest --preset macos-universal`.
- Install: `cmake --install build/macos-universal --prefix staging/macos-universal`.
- Paketierung: Job in `.github/workflows/portable-release.yml`.


### 12.8 android-arm64

- Configure: `cmake --preset android-arm64`.
- Build: `cmake --build --preset android-arm64`.
- Test: `ctest --preset android-arm64`.
- Install: `cmake --install build/android-arm64 --prefix staging/android-arm64`.
- Paketierung: Job in `.github/workflows/portable-release.yml`.


### 12.9 android-x64

- Configure: `cmake --preset android-x64`.
- Build: `cmake --build --preset android-x64`.
- Test: `ctest --preset android-x64`.
- Install: `cmake --install build/android-x64 --prefix staging/android-x64`.
- Paketierung: Job in `.github/workflows/portable-release.yml`.


### 12.10 freebsd-x64

- Configure: `cmake --preset freebsd-x64`.
- Build: `cmake --build --preset freebsd-x64`.
- Test: `ctest --preset freebsd-x64`.
- Install: `cmake --install build/freebsd-x64 --prefix staging/freebsd-x64`.
- Paketierung: Job in `.github/workflows/portable-release.yml`.


## 13. Packaging-Matrix

Jedes Paket entsteht aus demselben Commit und derselben Version. Die folgenden Rezepte sind verbindlich.

| Format | Werkzeug | Rezept | Artefakt |
|---|---|---|---|
| deb | nfpm package --packager deb | packaging/linux/nfpm.yaml | matticraft_<ver>_<arch>.deb |
| rpm | nfpm package --packager rpm | packaging/linux/nfpm.yaml | matticraft-<ver>.<arch>.rpm |
| AppImage | linuxdeploy + appimagetool | packaging/linux/appimage.sh | Matticraft-<ver>-x86_64.AppImage |
| Flatpak | flatpak-builder | packaging/linux/org.matticraft.Matticraft.yml | org.matticraft.Matticraft.flatpak |
| tar.zst | cmake --install + tar --zstd | packaging/linux/portable.sh | matticraft-<ver>-linux-<arch>.tar.zst |
| MSI | wix build | packaging/windows/matticraft.wxs | Matticraft-<ver>-x64.msi |
| Windows ZIP | cmake --install + 7z | packaging/windows/portable.ps1 | Matticraft-<ver>-windows-x64.zip |
| DMG | create-dmg + codesign | packaging/macos/dmg.sh | Matticraft-<ver>-universal.dmg |
| PKG | pkgbuild + productbuild | packaging/macos/pkg.sh | Matticraft-<ver>-universal.pkg |
| APK | gradlew assembleRelease | library/src/mobile/mobile_client | Matticraft-<ver>-arm64.apk |
| AAB | gradlew bundleRelease | library/src/mobile/mobile_client | Matticraft-<ver>.aab |


### 13.1 deb

1. Version aus `CMakeLists.txt`/`Cargo.toml` lesen.
2. Staging aus `cmake --install` verwenden.
3. Werkzeug: nfpm package --packager deb.
4. Rezept: `packaging/linux/nfpm.yaml`.
5. Signieren (falls Plattform) und Timestamp setzen.
6. Artefakt `matticraft_<ver>_<arch>.deb` in `dist/` ablegen.
7. SHA-256 in `dist/SHA256SUMS` schreiben.
8. SBOM-Eintrag erzeugen.
9. Smoke-Test: Installation/Start, Version, Fenster oder Activity.
10. Im Release-Manifest verlinken.


### 13.2 rpm

1. Version aus `CMakeLists.txt`/`Cargo.toml` lesen.
2. Staging aus `cmake --install` verwenden.
3. Werkzeug: nfpm package --packager rpm.
4. Rezept: `packaging/linux/nfpm.yaml`.
5. Signieren (falls Plattform) und Timestamp setzen.
6. Artefakt `matticraft-<ver>.<arch>.rpm` in `dist/` ablegen.
7. SHA-256 in `dist/SHA256SUMS` schreiben.
8. SBOM-Eintrag erzeugen.
9. Smoke-Test: Installation/Start, Version, Fenster oder Activity.
10. Im Release-Manifest verlinken.


### 13.3 AppImage

1. Version aus `CMakeLists.txt`/`Cargo.toml` lesen.
2. Staging aus `cmake --install` verwenden.
3. Werkzeug: linuxdeploy + appimagetool.
4. Rezept: `packaging/linux/appimage.sh`.
5. Signieren (falls Plattform) und Timestamp setzen.
6. Artefakt `Matticraft-<ver>-x86_64.AppImage` in `dist/` ablegen.
7. SHA-256 in `dist/SHA256SUMS` schreiben.
8. SBOM-Eintrag erzeugen.
9. Smoke-Test: Installation/Start, Version, Fenster oder Activity.
10. Im Release-Manifest verlinken.


### 13.4 Flatpak

1. Version aus `CMakeLists.txt`/`Cargo.toml` lesen.
2. Staging aus `cmake --install` verwenden.
3. Werkzeug: flatpak-builder.
4. Rezept: `packaging/linux/org.matticraft.Matticraft.yml`.
5. Signieren (falls Plattform) und Timestamp setzen.
6. Artefakt `org.matticraft.Matticraft.flatpak` in `dist/` ablegen.
7. SHA-256 in `dist/SHA256SUMS` schreiben.
8. SBOM-Eintrag erzeugen.
9. Smoke-Test: Installation/Start, Version, Fenster oder Activity.
10. Im Release-Manifest verlinken.


### 13.5 tar.zst

1. Version aus `CMakeLists.txt`/`Cargo.toml` lesen.
2. Staging aus `cmake --install` verwenden.
3. Werkzeug: cmake --install + tar --zstd.
4. Rezept: `packaging/linux/portable.sh`.
5. Signieren (falls Plattform) und Timestamp setzen.
6. Artefakt `matticraft-<ver>-linux-<arch>.tar.zst` in `dist/` ablegen.
7. SHA-256 in `dist/SHA256SUMS` schreiben.
8. SBOM-Eintrag erzeugen.
9. Smoke-Test: Installation/Start, Version, Fenster oder Activity.
10. Im Release-Manifest verlinken.


### 13.6 MSI

1. Version aus `CMakeLists.txt`/`Cargo.toml` lesen.
2. Staging aus `cmake --install` verwenden.
3. Werkzeug: wix build.
4. Rezept: `packaging/windows/matticraft.wxs`.
5. Signieren (falls Plattform) und Timestamp setzen.
6. Artefakt `Matticraft-<ver>-x64.msi` in `dist/` ablegen.
7. SHA-256 in `dist/SHA256SUMS` schreiben.
8. SBOM-Eintrag erzeugen.
9. Smoke-Test: Installation/Start, Version, Fenster oder Activity.
10. Im Release-Manifest verlinken.


### 13.7 Windows ZIP

1. Version aus `CMakeLists.txt`/`Cargo.toml` lesen.
2. Staging aus `cmake --install` verwenden.
3. Werkzeug: cmake --install + 7z.
4. Rezept: `packaging/windows/portable.ps1`.
5. Signieren (falls Plattform) und Timestamp setzen.
6. Artefakt `Matticraft-<ver>-windows-x64.zip` in `dist/` ablegen.
7. SHA-256 in `dist/SHA256SUMS` schreiben.
8. SBOM-Eintrag erzeugen.
9. Smoke-Test: Installation/Start, Version, Fenster oder Activity.
10. Im Release-Manifest verlinken.


### 13.8 DMG

1. Version aus `CMakeLists.txt`/`Cargo.toml` lesen.
2. Staging aus `cmake --install` verwenden.
3. Werkzeug: create-dmg + codesign.
4. Rezept: `packaging/macos/dmg.sh`.
5. Signieren (falls Plattform) und Timestamp setzen.
6. Artefakt `Matticraft-<ver>-universal.dmg` in `dist/` ablegen.
7. SHA-256 in `dist/SHA256SUMS` schreiben.
8. SBOM-Eintrag erzeugen.
9. Smoke-Test: Installation/Start, Version, Fenster oder Activity.
10. Im Release-Manifest verlinken.


### 13.9 PKG

1. Version aus `CMakeLists.txt`/`Cargo.toml` lesen.
2. Staging aus `cmake --install` verwenden.
3. Werkzeug: pkgbuild + productbuild.
4. Rezept: `packaging/macos/pkg.sh`.
5. Signieren (falls Plattform) und Timestamp setzen.
6. Artefakt `Matticraft-<ver>-universal.pkg` in `dist/` ablegen.
7. SHA-256 in `dist/SHA256SUMS` schreiben.
8. SBOM-Eintrag erzeugen.
9. Smoke-Test: Installation/Start, Version, Fenster oder Activity.
10. Im Release-Manifest verlinken.


### 13.10 APK

1. Version aus `CMakeLists.txt`/`Cargo.toml` lesen.
2. Staging aus `cmake --install` verwenden.
3. Werkzeug: gradlew assembleRelease.
4. Rezept: `library/src/mobile/mobile_client`.
5. Signieren (falls Plattform) und Timestamp setzen.
6. Artefakt `Matticraft-<ver>-arm64.apk` in `dist/` ablegen.
7. SHA-256 in `dist/SHA256SUMS` schreiben.
8. SBOM-Eintrag erzeugen.
9. Smoke-Test: Installation/Start, Version, Fenster oder Activity.
10. Im Release-Manifest verlinken.


### 13.11 AAB

1. Version aus `CMakeLists.txt`/`Cargo.toml` lesen.
2. Staging aus `cmake --install` verwenden.
3. Werkzeug: gradlew bundleRelease.
4. Rezept: `library/src/mobile/mobile_client`.
5. Signieren (falls Plattform) und Timestamp setzen.
6. Artefakt `Matticraft-<ver>.aab` in `dist/` ablegen.
7. SHA-256 in `dist/SHA256SUMS` schreiben.
8. SBOM-Eintrag erzeugen.
9. Smoke-Test: Installation/Start, Version, Fenster oder Activity.
10. Im Release-Manifest verlinken.


## 14. GitHub Actions und Releases

GitHub Releases bauen alles auf Linux-Runnern: Linux-Pakete, Windows per MinGW-w64 + wixl (MSI), macOS per osxcross (DMG, unsigniert) und Android per SDK/NDK. Es werden keine windows-* oder macos-* Runner benötigt; der Workflow ist manuell und per Tag auslösbar.

Voraussetzungen für die Cross-Ziele auf Linux: mingw-w64 + msitools für Windows, ein osxcross-Baum samt (nicht redistributierbarer) Apple-SDK für macOS, Android SDK/NDK für APK/AAB. Signierung von MSI/DMG/PKG bleibt plattformgebunden (Authenticode/Notarisierung) und ist optional.


### 14.1 Trigger

- `workflow_dispatch` mit Inputs: version, channel, sign.
- Push eines Tags `v*`.
- Nächtlicher Cron für Nightly-Artefakte (ohne Store-Upload).


### 14.2 Jobs

- **prepare**: Version, Commit, Matrix, Cache-Keys.
- **linux-x64**: Build, Tests, deb/rpm/AppImage/Flatpak/tar.zst.
- **linux-arm64**: Build, deb/tar.zst (Cross oder arm64-Runner).
- **windows-x64**: MSVC-Build, Tests, MSI/ZIP, Signierung.
- **windows-arm64**: MSVC-ARM64-Build, ZIP.
- **macos-universal**: Universal-Build, DMG/PKG, Notarisierung.
- **android**: Gradle, APK/AAB, Signierung, Emulator-Smoke.
- **rust**: cargo build/test/fmt/clippy, Artefakte einbetten.
- **release**: Artefakte sammeln, Checksums, SBOM, Draft-Release.


### 14.3 Schritte je Job

1. Repository auschecken (fetch-depth 0 für Versionen).
2. Toolchain installieren (rustup, CMake, Ninja, Plattform-SDK).
3. Cache wiederherstellen (ccache, cargo, Gradle).
4. Configure und Build mit dem Plattform-Preset.
5. Tests und Headless-Screenshots ausführen.
6. Paket bauen und signieren.
7. Artefakt hochladen (Name: `<platform>-<arch>-<format>`).
8. Logs und SBOM als Artefakt sichern.
9. Release-Job: Checksums, Manifest, Draft-Release.


### 14.4 Release-Artefakte

- `matticraft-linux-x64.AppImage`, `.deb`, `.rpm`, `.flatpak`, `.tar.zst`.
- `matticraft-windows-x64.msi`, `.zip`, `.exe` (optional).
- `matticraft-macos-universal.dmg`, `.pkg`.
- `matticraft-android-arm64.apk`, `matticraft-android.aab`.
- `SHA256SUMS`, `SBOM.cdx.json`, `latest.json`, Signaturen.


### 14.5 Benötigte Secrets

- `WINDOWS_CERT_PFX`
- `WINDOWS_CERT_PASSWORD`
- `APPLE_ID`
- `APPLE_TEAM_ID`
- `APPLE_APP_PASSWORD`
- `ANDROID_KEYSTORE_BASE64`
- `ANDROID_KEY_ALIAS`
- `ANDROID_KEY_PASSWORD`
- `GPG_PRIVATE_KEY`
- `GPG_PASSPHRASE`
- `DISCORD_WEBHOOK (optional)`

Ohne Secrets entstehen unsignierte Testartefakte; der Release-Job markiert sie als `unsigned` und veröffentlicht sie nicht als Stable.


## 15. Tests und Qualitätssicherung

Testmatrix: Screens × Layer/API × Plattform × GUI-Scale. Jede Kombination hat ein definiertes Erwartungsergebnis.


### 15.1 Screens

- Title: Layout, Text, Hover, Klickziel, ESC-Verhalten.
- Singleplayer: Layout, Text, Hover, Klickziel, ESC-Verhalten.
- Multiplayer: Layout, Text, Hover, Klickziel, ESC-Verhalten.
- Options: Layout, Text, Hover, Klickziel, ESC-Verhalten.
- Pause: Layout, Text, Hover, Klickziel, ESC-Verhalten.
- World: Layout, Text, Hover, Klickziel, ESC-Verhalten.
- Info: Layout, Text, Hover, Klickziel, ESC-Verhalten.
- Loading: Layout, Text, Hover, Klickziel, ESC-Verhalten.
- Crash: Layout, Text, Hover, Klickziel, ESC-Verhalten.
- Mods: Layout, Text, Hover, Klickziel, ESC-Verhalten.
- Language: Layout, Text, Hover, Klickziel, ESC-Verhalten.
- Accessibility: Layout, Text, Hover, Klickziel, ESC-Verhalten.


### 15.2 Renderer-Kombinationen

- `native:opengl` auf linux/windows/macos: Start, Frame, Screenshot, kein Crash.
- `native:vulkan` auf linux/windows/macos: Start, Frame, Screenshot, kein Crash.
- `sdl:opengl` auf linux/windows/macos/android: Start, Frame, Screenshot, kein Crash.
- `sdl:vulkan` auf linux/windows/macos/android: Start, Frame, Screenshot, kein Crash.
- `sdl:directx` auf windows: Start, Frame, Screenshot, kein Crash.
- `sdl:auto` auf linux/windows/macos/android: Start, Frame, Screenshot, kein Crash.


### 15.3 Testarten

- Unit-Tests der C-Harnesses (ctest).
- Rust-Tests und clippy.
- Golden Screenshots mit Toleranz für Font-Antialiasing.
- Headless-Läufe mit `MATTI_HEADLESS=1 --ui-once`.
- Android-Instrumentation mit Surface- und Frame-Assertion.
- Packaging-Smoke je Format.
- Launcher-Tests für Profil- und Update-Parsing.
- Lasttest: 30 Minuten Loop ohne Leak (RSS stabil).


### 15.4 Abnahmekriterien (Kurzfassung)

1. Jede Plattform baut mit einem Preset.
2. Jede unterstützte Selection startet oder meldet einen klaren Fehler.
3. Kein stiller Fallback.
4. Options-Screen zeigt und wechselt die Selection.
5. Screenshots sind reproduzierbar.
6. Pakete installieren und starten.
7. Android-App startet und rendert ein Frame.
8. GitHub-Release enthält alle Artefakte und Checksums.


## 16. Sicherheit und Signierung

- Windows: Authenticode + Timestamp, MSI mit Upgrade-Code.
- macOS: Developer ID, Hardened Runtime, Notarisierung, Stapling.
- Android: eigener Keystore, Play App Signing, versionCode aus CI.
- Linux: GPG/minisign über SHA256SUMS, Flatpak mit Flathub-Signatur.
- Alle Downloads HTTPS; Manifeste signiert; Rollback-Schutz.
- Keine Secrets in Logs; Secrets nur in GitHub Environments.
- SBOM (CycloneDX) und Lizenzliste pro Release.


## 17. Meilensteine

- **M0 Renderer-Layer** ✅ — Selection, Plattformprüfung, Options-Wechsel, Tests auf Linux.
- **M1 Platform/Arch** ✅ — Arch/Bits/Endianness, Defaults, Logging.
- **M2 Android** 🟡 — Gradle-Gerüst, JNI, Compose-App, APK.
- **M3 Launcher** ⬜ — Core + drei Frontends, Profilformat.
- **M4 Packaging** ⬜ — deb/rpm/AppImage/Flatpak/MSI/DMG/APK.
- **M5 Release-CI** ⬜ — Matrix, Signierung, Draft-Release, SBOM.
- **M6 Store/Härtung** ⬜ — Notarisierung, Play Store, Updater, Crash-Report.


## 18. Datei-für-Datei-Arbeitsliste

- [ ] `CMakeLists.txt` — prüfen, ergänzen, testen.
- [ ] `CMakePresets.json` — prüfen, ergänzen, testen.
- [ ] `planported.MD` — prüfen, ergänzen, testen.
- [ ] `README.md` — prüfen, ergänzen, testen.
- [ ] `cmake/MattiNative.cmake` — prüfen, ergänzen, testen.
- [ ] `cmake/MattiRust.cmake` — prüfen, ergänzen, testen.
- [ ] `cmake/MattiPlatform.cmake` — prüfen, ergänzen, testen.
- [ ] `cmake/MattiLauncher.cmake` — prüfen, ergänzen, testen.
- [ ] `library/src/libmatti/matti/Platform.h` — prüfen, ergänzen, testen.
- [ ] `library/src/libmatti/matti/Platform.c` — prüfen, ergänzen, testen.
- [ ] `library/src/libmatti/matti/render/render.h` — prüfen, ergänzen, testen.
- [ ] `library/src/libmatti/matti/render/render.c` — prüfen, ergänzen, testen.
- [ ] `library/src/libmatti/matti/render/client_opengl.c` — prüfen, ergänzen, testen.
- [ ] `library/src/libmatti/matti/render/client_vulkan.c` — prüfen, ergänzen, testen.
- [ ] `library/src/libmatti/matti/render/client_sdl.c` — prüfen, ergänzen, testen.
- [ ] `library/src/libmatti/matti/ui/menu.h` — prüfen, ergänzen, testen.
- [ ] `library/src/libmatti/matti/ui/menu.c` — prüfen, ergänzen, testen.
- [ ] `library/src/libmatti/matti/ui/texture.h` — prüfen, ergänzen, testen.
- [ ] `library/src/libmatti/matti/ui/texture.c` — prüfen, ergänzen, testen.
- [ ] `library/src/libmatti/matti/ui/bridge.h` — prüfen, ergänzen, testen.
- [ ] `library/src/libmatti/matti/ui/bridge.c` — prüfen, ergänzen, testen.
- [ ] `library/src/libmatti/matti/ui/assets/gui/widgets.png` — prüfen, ergänzen, testen.
- [ ] `library/src/libmatti/matti/ui/assets/gui/options_background.png` — prüfen, ergänzen, testen.
- [ ] `library/src/libmatti/matti/ui/assets/gui/title/minecraft.png` — prüfen, ergänzen, testen.
- [ ] `library/src/libmatti/matti/ui/assets/gui/title/edition.png` — prüfen, ergänzen, testen.
- [ ] `library/src/libmatti/matti/ui/assets/gui/title/background/panorama_0.png` — prüfen, ergänzen, testen.
- [ ] `library/src/libmatti/matti/ui/assets/font/ascii.png` — prüfen, ergänzen, testen.
- [ ] `library/src/libmatti/rs/Cargo.toml` — prüfen, ergänzen, testen.
- [ ] `library/src/libmatti/rs/src/lib.rs` — prüfen, ergänzen, testen.
- [ ] `library/src/libmatti/rs/src/ui/mod.rs` — prüfen, ergänzen, testen.
- [ ] `library/src/libmatti/rs/src/ui/ffi.rs` — prüfen, ergänzen, testen.
- [ ] `library/src/mobile/settings.gradle.kts` — prüfen, ergänzen, testen.
- [ ] `library/src/mobile/build.gradle.kts` — prüfen, ergänzen, testen.
- [ ] `library/src/mobile/matti-kotlin/build.gradle.kts` — prüfen, ergänzen, testen.
- [ ] `library/src/mobile/matti-kotlin/src/main/kotlin/dev/matticraft/libmatti/LibMatti.kt` — prüfen, ergänzen, testen.
- [ ] `library/src/mobile/matti-kotlin/src/main/cpp/libmatti_jni.c` — prüfen, ergänzen, testen.
- [ ] `library/src/mobile/mobile_client/build.gradle.kts` — prüfen, ergänzen, testen.
- [ ] `library/src/mobile/mobile_client/src/main/kotlin/dev/matticraft/mobile/MainActivity.kt` — prüfen, ergänzen, testen.
- [ ] `launchers/Cargo.toml` — prüfen, ergänzen, testen.
- [ ] `launchers/launcher-core/Cargo.toml` — prüfen, ergänzen, testen.
- [ ] `launchers/launcher-core/src/lib.rs` — prüfen, ergänzen, testen.
- [ ] `launchers/launcher-core/src/profile.rs` — prüfen, ergänzen, testen.
- [ ] `launchers/launcher-core/src/store.rs` — prüfen, ergänzen, testen.
- [ ] `launchers/launcher-core/src/account.rs` — prüfen, ergänzen, testen.
- [ ] `launchers/launcher-core/src/mods.rs` — prüfen, ergänzen, testen.
- [ ] `launchers/launcher-core/src/logs.rs` — prüfen, ergänzen, testen.
- [ ] `launchers/launcher-core/src/update.rs` — prüfen, ergänzen, testen.
- [ ] `launchers/launcher-core/src/launch.rs` — prüfen, ergänzen, testen.
- [ ] `launchers/launcher-core/src/platform.rs` — prüfen, ergänzen, testen.
- [ ] `launchers/launcher-core/src/bin/matticraft-update-sign.rs` — prüfen, ergänzen, testen.
- [ ] `launchers/desktop/Cargo.toml` — prüfen, ergänzen, testen.
- [ ] `launchers/desktop/src/app.rs` — prüfen, ergänzen, testen.
- [ ] `launchers/desktop/src/lib.rs` — prüfen, ergänzen, testen.
- [ ] `launchers/desktop/src/main.rs` — prüfen, ergänzen, testen.
- [ ] `launchers/macos/Cargo.toml` — prüfen, ergänzen, testen.
- [ ] `launchers/macos/src/main.rs` — prüfen, ergänzen, testen.
- [ ] `packaging/linux/nfpm.yaml` — prüfen, ergänzen, testen.
- [ ] `packaging/linux/appimage.sh` — prüfen, ergänzen, testen.
- [ ] `packaging/linux/org.matticraft.Matticraft.yml` — prüfen, ergänzen, testen.
- [ ] `packaging/windows/matticraft.wxs` — prüfen, ergänzen, testen.
- [ ] `packaging/windows/matticraft-wixl.wxs` — prüfen, ergänzen, testen.
- [ ] `packaging/windows/portable.ps1` — prüfen, ergänzen, testen.
- [ ] `packaging/windows/build-msi.ps1` — prüfen, ergänzen, testen.
- [ ] `packaging/windows/build-msi-linux.sh` — prüfen, ergänzen, testen.
- [ ] `packaging/macos/dmg.sh` — prüfen, ergänzen, testen.
- [ ] `packaging/macos/pkg.sh` — prüfen, ergänzen, testen.
- [ ] `packaging/freebsd/portable.sh` — prüfen, ergänzen, testen.
- [ ] `packaging/android/signing.gradle.kts` — prüfen, ergänzen, testen.
- [ ] `.github/workflows/portable-release.yml` — prüfen, ergänzen, testen.
- [ ] `tools/fetch_vanilla_gui_assets.py` — prüfen, ergänzen, testen.
- [ ] `tools/generate_planported.py` — prüfen, ergänzen, testen.


## 19. Risiken und offene Entscheidungen

| Risiko | Gegenmaßnahme |
|---|---|
| DirectX/Metal nur über SDL | Treiberqualität variiert; Capability-Report im Launcher. |
| macOS OpenGL deprecated | Standard Metal via SDL-Auto, GL als Fallback. |
| Android Surface/Lifecycle | JNI-Threading und Asset-Extraktion früh testen. |
| Signierung | Zertifikate/Secrets sind extern; Testartefakte kennzeichnen. |
| Binary-Größe | Panorama/Assets ~32 MB; Kompression oder Shared Assets prüfen. |
| CI-Kosten | macOS/Windows-Runner teuer; Nightly begrenzen. |
| Universal-Build | arm64/x86_64 müssen beide getestet werden. |
| 32-Bit | Vulkan-Treiber selten; Default OpenGL auf 32-Bit-Linux. |

- Desktop-Launcher: egui (empfohlen) vs. Tauri.
- Android-Launcher als eigene App vs. Screen in mobile_client.
- Flathub jetzt oder nach M5.
- Ob Assets externalisiert werden.


## 20. Anhänge


### 20.1 CLI

| Flag | Bedeutung | Gilt für |
|---|---|---|
| --help | print help | all |
| --launchTarget <name> | neoforge/minecraft/testharness | all |
| --gameDir <dir> | game directory | desktop |
| --assetsDir <dir> | vanilla asset root | desktop |
| --width <n> | framebuffer width | desktop |
| --height <n> | framebuffer height | desktop |
| --fullscreen | start fullscreen | desktop |
| --layer native|sdl | window/render layer | all |
| --api opengl|vulkan|directx|metal | API below the layer | all |
| --ui-screen <name> | start screen (QA) | all |
| --ui-once | one frame + screenshot | CI |
| --no-render | headless | CI |
| --no-tests | skip selftests | all |


### 20.2 Umgebungsvariablen

| Variable | Bedeutung |
|---|---|
| MATTI_RENDERER | layer:api fallback |
| MATTI_LAYER | layer fallback when --layer is absent |
| MATTI_API | API fallback when --api is absent |
| LIBMATTI_VK | legacy TRUE -> vulkan |
| MATTI_ASSETS_DIR | asset root override |
| MATTI_HEADLESS | no window |
| MATTI_UI_SCREENSHOT | screenshot path for --ui-once |
| MATTI_THEME_FONT | theme font override |
| SDL_VIDEODRIVER | SDL platform driver |
| SDL_RENDER_DRIVER | SDL render driver override |
| MATTI_CAM_YAW / MATTI_CAM_PITCH | camera debug |
| MATTI_CHUNK_DEBUG | chunk culling logs |
| ANDROID_STORAGE | app-private storage on Android |


### 20.3 Selection-Strings

- `auto` — historischer Auto-Pfad.
- `native:opengl` — GLFW + OpenGL 3.2.
- `native:vulkan` — GLFW + Vulkan.
- `sdl:auto` — SDL wählt den Driver.
- `sdl:opengl` — SDL opengl/opengles2.
- `sdl:vulkan` — SDL vulkan.
- `sdl:directx` — SDL direct3d12/11/9 (Windows).
- `opengl` — Kurzform für native:opengl.
- `vulkan` — Kurzform für native:vulkan.
- `directx` — Kurzform für sdl:directx.


### 20.4 Dateiformate

- `profile.json` — Launcher-Profil (Name, Version, Selection, Pfade).
- `latest.json` — Update-Manifest (Version, URL, SHA-256, Größe, Signatur).
- `SBOM.cdx.json` — CycloneDX-SBOM.
- `SHA256SUMS` — Checksummen aller Artefakte.
- `options.txt` — Spieleinstellungen (Vanilla-kompatibel).
- `horizon.txt` — Port-spezifische Einstellungen (optional).


## 21. Nummerierte Abnahmekriterien

Die folgenden Kriterien sind die konkrete Prüfliste für PR, CI und Release. Jede Zeile ist eindeutig; die Nummer wird in Issues referenziert.

AC-0001 | linux-x86_64 (64-bit) | `native:opengl` | Fenster öffnet | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0002 | linux-x86_64 (64-bit) | `native:opengl` | Frame präsentiert | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0003 | linux-x86_64 (64-bit) | `native:opengl` | Screenshot erzeugt | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0004 | linux-x86_64 (64-bit) | `native:opengl` | Options zeigt Selection | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0005 | linux-x86_64 (64-bit) | `native:opengl` | Selection-Wechsel startet neu | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0006 | linux-x86_64 (64-bit) | `native:opengl` | Kein Crash im 60s-Loop | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0007 | linux-x86_64 (64-bit) | `native:opengl` | Exit-Code korrekt | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0008 | linux-x86_64 (64-bit) | `native:opengl` | Log enthält Plattform | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0009 | linux-x86_64 (64-bit) | `native:opengl` | Assets geladen | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0010 | linux-x86_64 (64-bit) | `native:opengl` | UI-Text sichtbar | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0011 | linux-x86_64 (64-bit) | `native:opengl` | Maus/ESC reagieren | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0012 | linux-x86_64 (64-bit) | `native:opengl` | Speicher stabil | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0013 | linux-x86_64 (64-bit) | `native:vulkan` | Fenster öffnet | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0014 | linux-x86_64 (64-bit) | `native:vulkan` | Frame präsentiert | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0015 | linux-x86_64 (64-bit) | `native:vulkan` | Screenshot erzeugt | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0016 | linux-x86_64 (64-bit) | `native:vulkan` | Options zeigt Selection | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0017 | linux-x86_64 (64-bit) | `native:vulkan` | Selection-Wechsel startet neu | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0018 | linux-x86_64 (64-bit) | `native:vulkan` | Kein Crash im 60s-Loop | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0019 | linux-x86_64 (64-bit) | `native:vulkan` | Exit-Code korrekt | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0020 | linux-x86_64 (64-bit) | `native:vulkan` | Log enthält Plattform | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0021 | linux-x86_64 (64-bit) | `native:vulkan` | Assets geladen | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0022 | linux-x86_64 (64-bit) | `native:vulkan` | UI-Text sichtbar | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0023 | linux-x86_64 (64-bit) | `native:vulkan` | Maus/ESC reagieren | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0024 | linux-x86_64 (64-bit) | `native:vulkan` | Speicher stabil | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0025 | linux-x86_64 (64-bit) | `sdl:opengl` | Fenster öffnet | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0026 | linux-x86_64 (64-bit) | `sdl:opengl` | Frame präsentiert | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0027 | linux-x86_64 (64-bit) | `sdl:opengl` | Screenshot erzeugt | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0028 | linux-x86_64 (64-bit) | `sdl:opengl` | Options zeigt Selection | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0029 | linux-x86_64 (64-bit) | `sdl:opengl` | Selection-Wechsel startet neu | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0030 | linux-x86_64 (64-bit) | `sdl:opengl` | Kein Crash im 60s-Loop | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0031 | linux-x86_64 (64-bit) | `sdl:opengl` | Exit-Code korrekt | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0032 | linux-x86_64 (64-bit) | `sdl:opengl` | Log enthält Plattform | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0033 | linux-x86_64 (64-bit) | `sdl:opengl` | Assets geladen | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0034 | linux-x86_64 (64-bit) | `sdl:opengl` | UI-Text sichtbar | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0035 | linux-x86_64 (64-bit) | `sdl:opengl` | Maus/ESC reagieren | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0036 | linux-x86_64 (64-bit) | `sdl:opengl` | Speicher stabil | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0037 | linux-x86_64 (64-bit) | `sdl:vulkan` | Fenster öffnet | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0038 | linux-x86_64 (64-bit) | `sdl:vulkan` | Frame präsentiert | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0039 | linux-x86_64 (64-bit) | `sdl:vulkan` | Screenshot erzeugt | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0040 | linux-x86_64 (64-bit) | `sdl:vulkan` | Options zeigt Selection | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0041 | linux-x86_64 (64-bit) | `sdl:vulkan` | Selection-Wechsel startet neu | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0042 | linux-x86_64 (64-bit) | `sdl:vulkan` | Kein Crash im 60s-Loop | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0043 | linux-x86_64 (64-bit) | `sdl:vulkan` | Exit-Code korrekt | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0044 | linux-x86_64 (64-bit) | `sdl:vulkan` | Log enthält Plattform | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0045 | linux-x86_64 (64-bit) | `sdl:vulkan` | Assets geladen | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0046 | linux-x86_64 (64-bit) | `sdl:vulkan` | UI-Text sichtbar | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0047 | linux-x86_64 (64-bit) | `sdl:vulkan` | Maus/ESC reagieren | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0048 | linux-x86_64 (64-bit) | `sdl:vulkan` | Speicher stabil | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0049 | linux-x86_64 (64-bit) | `sdl:directx` | Fenster öffnet | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0050 | linux-x86_64 (64-bit) | `sdl:directx` | Frame präsentiert | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0051 | linux-x86_64 (64-bit) | `sdl:directx` | Screenshot erzeugt | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0052 | linux-x86_64 (64-bit) | `sdl:directx` | Options zeigt Selection | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0053 | linux-x86_64 (64-bit) | `sdl:directx` | Selection-Wechsel startet neu | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0054 | linux-x86_64 (64-bit) | `sdl:directx` | Kein Crash im 60s-Loop | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0055 | linux-x86_64 (64-bit) | `sdl:directx` | Exit-Code korrekt | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0056 | linux-x86_64 (64-bit) | `sdl:directx` | Log enthält Plattform | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0057 | linux-x86_64 (64-bit) | `sdl:directx` | Assets geladen | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0058 | linux-x86_64 (64-bit) | `sdl:directx` | UI-Text sichtbar | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0059 | linux-x86_64 (64-bit) | `sdl:directx` | Maus/ESC reagieren | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0060 | linux-x86_64 (64-bit) | `sdl:directx` | Speicher stabil | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0061 | linux-x86_64 (64-bit) | `sdl:auto` | Fenster öffnet | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0062 | linux-x86_64 (64-bit) | `sdl:auto` | Frame präsentiert | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0063 | linux-x86_64 (64-bit) | `sdl:auto` | Screenshot erzeugt | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0064 | linux-x86_64 (64-bit) | `sdl:auto` | Options zeigt Selection | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0065 | linux-x86_64 (64-bit) | `sdl:auto` | Selection-Wechsel startet neu | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0066 | linux-x86_64 (64-bit) | `sdl:auto` | Kein Crash im 60s-Loop | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0067 | linux-x86_64 (64-bit) | `sdl:auto` | Exit-Code korrekt | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0068 | linux-x86_64 (64-bit) | `sdl:auto` | Log enthält Plattform | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0069 | linux-x86_64 (64-bit) | `sdl:auto` | Assets geladen | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0070 | linux-x86_64 (64-bit) | `sdl:auto` | UI-Text sichtbar | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0071 | linux-x86_64 (64-bit) | `sdl:auto` | Maus/ESC reagieren | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0072 | linux-x86_64 (64-bit) | `sdl:auto` | Speicher stabil | Pakete: deb/rpm/AppImage/Flatpak/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0073 | linux-x86 (32-bit) | `native:opengl` | Fenster öffnet | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0074 | linux-x86 (32-bit) | `native:opengl` | Frame präsentiert | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0075 | linux-x86 (32-bit) | `native:opengl` | Screenshot erzeugt | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0076 | linux-x86 (32-bit) | `native:opengl` | Options zeigt Selection | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0077 | linux-x86 (32-bit) | `native:opengl` | Selection-Wechsel startet neu | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0078 | linux-x86 (32-bit) | `native:opengl` | Kein Crash im 60s-Loop | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0079 | linux-x86 (32-bit) | `native:opengl` | Exit-Code korrekt | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0080 | linux-x86 (32-bit) | `native:opengl` | Log enthält Plattform | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0081 | linux-x86 (32-bit) | `native:opengl` | Assets geladen | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0082 | linux-x86 (32-bit) | `native:opengl` | UI-Text sichtbar | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0083 | linux-x86 (32-bit) | `native:opengl` | Maus/ESC reagieren | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0084 | linux-x86 (32-bit) | `native:opengl` | Speicher stabil | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0085 | linux-x86 (32-bit) | `native:vulkan` | Fenster öffnet | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0086 | linux-x86 (32-bit) | `native:vulkan` | Frame präsentiert | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0087 | linux-x86 (32-bit) | `native:vulkan` | Screenshot erzeugt | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0088 | linux-x86 (32-bit) | `native:vulkan` | Options zeigt Selection | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0089 | linux-x86 (32-bit) | `native:vulkan` | Selection-Wechsel startet neu | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0090 | linux-x86 (32-bit) | `native:vulkan` | Kein Crash im 60s-Loop | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0091 | linux-x86 (32-bit) | `native:vulkan` | Exit-Code korrekt | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0092 | linux-x86 (32-bit) | `native:vulkan` | Log enthält Plattform | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0093 | linux-x86 (32-bit) | `native:vulkan` | Assets geladen | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0094 | linux-x86 (32-bit) | `native:vulkan` | UI-Text sichtbar | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0095 | linux-x86 (32-bit) | `native:vulkan` | Maus/ESC reagieren | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0096 | linux-x86 (32-bit) | `native:vulkan` | Speicher stabil | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0097 | linux-x86 (32-bit) | `sdl:opengl` | Fenster öffnet | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0098 | linux-x86 (32-bit) | `sdl:opengl` | Frame präsentiert | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0099 | linux-x86 (32-bit) | `sdl:opengl` | Screenshot erzeugt | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0100 | linux-x86 (32-bit) | `sdl:opengl` | Options zeigt Selection | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0101 | linux-x86 (32-bit) | `sdl:opengl` | Selection-Wechsel startet neu | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0102 | linux-x86 (32-bit) | `sdl:opengl` | Kein Crash im 60s-Loop | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0103 | linux-x86 (32-bit) | `sdl:opengl` | Exit-Code korrekt | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0104 | linux-x86 (32-bit) | `sdl:opengl` | Log enthält Plattform | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0105 | linux-x86 (32-bit) | `sdl:opengl` | Assets geladen | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0106 | linux-x86 (32-bit) | `sdl:opengl` | UI-Text sichtbar | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0107 | linux-x86 (32-bit) | `sdl:opengl` | Maus/ESC reagieren | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0108 | linux-x86 (32-bit) | `sdl:opengl` | Speicher stabil | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0109 | linux-x86 (32-bit) | `sdl:vulkan` | Fenster öffnet | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0110 | linux-x86 (32-bit) | `sdl:vulkan` | Frame präsentiert | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0111 | linux-x86 (32-bit) | `sdl:vulkan` | Screenshot erzeugt | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0112 | linux-x86 (32-bit) | `sdl:vulkan` | Options zeigt Selection | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0113 | linux-x86 (32-bit) | `sdl:vulkan` | Selection-Wechsel startet neu | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0114 | linux-x86 (32-bit) | `sdl:vulkan` | Kein Crash im 60s-Loop | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0115 | linux-x86 (32-bit) | `sdl:vulkan` | Exit-Code korrekt | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0116 | linux-x86 (32-bit) | `sdl:vulkan` | Log enthält Plattform | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0117 | linux-x86 (32-bit) | `sdl:vulkan` | Assets geladen | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0118 | linux-x86 (32-bit) | `sdl:vulkan` | UI-Text sichtbar | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0119 | linux-x86 (32-bit) | `sdl:vulkan` | Maus/ESC reagieren | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0120 | linux-x86 (32-bit) | `sdl:vulkan` | Speicher stabil | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0121 | linux-x86 (32-bit) | `sdl:directx` | Fenster öffnet | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0122 | linux-x86 (32-bit) | `sdl:directx` | Frame präsentiert | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0123 | linux-x86 (32-bit) | `sdl:directx` | Screenshot erzeugt | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0124 | linux-x86 (32-bit) | `sdl:directx` | Options zeigt Selection | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0125 | linux-x86 (32-bit) | `sdl:directx` | Selection-Wechsel startet neu | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0126 | linux-x86 (32-bit) | `sdl:directx` | Kein Crash im 60s-Loop | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0127 | linux-x86 (32-bit) | `sdl:directx` | Exit-Code korrekt | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0128 | linux-x86 (32-bit) | `sdl:directx` | Log enthält Plattform | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0129 | linux-x86 (32-bit) | `sdl:directx` | Assets geladen | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0130 | linux-x86 (32-bit) | `sdl:directx` | UI-Text sichtbar | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0131 | linux-x86 (32-bit) | `sdl:directx` | Maus/ESC reagieren | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0132 | linux-x86 (32-bit) | `sdl:directx` | Speicher stabil | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0133 | linux-x86 (32-bit) | `sdl:auto` | Fenster öffnet | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0134 | linux-x86 (32-bit) | `sdl:auto` | Frame präsentiert | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0135 | linux-x86 (32-bit) | `sdl:auto` | Screenshot erzeugt | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0136 | linux-x86 (32-bit) | `sdl:auto` | Options zeigt Selection | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0137 | linux-x86 (32-bit) | `sdl:auto` | Selection-Wechsel startet neu | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0138 | linux-x86 (32-bit) | `sdl:auto` | Kein Crash im 60s-Loop | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0139 | linux-x86 (32-bit) | `sdl:auto` | Exit-Code korrekt | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0140 | linux-x86 (32-bit) | `sdl:auto` | Log enthält Plattform | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0141 | linux-x86 (32-bit) | `sdl:auto` | Assets geladen | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0142 | linux-x86 (32-bit) | `sdl:auto` | UI-Text sichtbar | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0143 | linux-x86 (32-bit) | `sdl:auto` | Maus/ESC reagieren | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0144 | linux-x86 (32-bit) | `sdl:auto` | Speicher stabil | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0145 | linux-arm64 (64-bit) | `native:opengl` | Fenster öffnet | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0146 | linux-arm64 (64-bit) | `native:opengl` | Frame präsentiert | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0147 | linux-arm64 (64-bit) | `native:opengl` | Screenshot erzeugt | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0148 | linux-arm64 (64-bit) | `native:opengl` | Options zeigt Selection | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0149 | linux-arm64 (64-bit) | `native:opengl` | Selection-Wechsel startet neu | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0150 | linux-arm64 (64-bit) | `native:opengl` | Kein Crash im 60s-Loop | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0151 | linux-arm64 (64-bit) | `native:opengl` | Exit-Code korrekt | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0152 | linux-arm64 (64-bit) | `native:opengl` | Log enthält Plattform | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0153 | linux-arm64 (64-bit) | `native:opengl` | Assets geladen | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0154 | linux-arm64 (64-bit) | `native:opengl` | UI-Text sichtbar | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0155 | linux-arm64 (64-bit) | `native:opengl` | Maus/ESC reagieren | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0156 | linux-arm64 (64-bit) | `native:opengl` | Speicher stabil | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0157 | linux-arm64 (64-bit) | `native:vulkan` | Fenster öffnet | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0158 | linux-arm64 (64-bit) | `native:vulkan` | Frame präsentiert | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0159 | linux-arm64 (64-bit) | `native:vulkan` | Screenshot erzeugt | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0160 | linux-arm64 (64-bit) | `native:vulkan` | Options zeigt Selection | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0161 | linux-arm64 (64-bit) | `native:vulkan` | Selection-Wechsel startet neu | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0162 | linux-arm64 (64-bit) | `native:vulkan` | Kein Crash im 60s-Loop | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0163 | linux-arm64 (64-bit) | `native:vulkan` | Exit-Code korrekt | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0164 | linux-arm64 (64-bit) | `native:vulkan` | Log enthält Plattform | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0165 | linux-arm64 (64-bit) | `native:vulkan` | Assets geladen | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0166 | linux-arm64 (64-bit) | `native:vulkan` | UI-Text sichtbar | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0167 | linux-arm64 (64-bit) | `native:vulkan` | Maus/ESC reagieren | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0168 | linux-arm64 (64-bit) | `native:vulkan` | Speicher stabil | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0169 | linux-arm64 (64-bit) | `sdl:opengl` | Fenster öffnet | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0170 | linux-arm64 (64-bit) | `sdl:opengl` | Frame präsentiert | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0171 | linux-arm64 (64-bit) | `sdl:opengl` | Screenshot erzeugt | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0172 | linux-arm64 (64-bit) | `sdl:opengl` | Options zeigt Selection | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0173 | linux-arm64 (64-bit) | `sdl:opengl` | Selection-Wechsel startet neu | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0174 | linux-arm64 (64-bit) | `sdl:opengl` | Kein Crash im 60s-Loop | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0175 | linux-arm64 (64-bit) | `sdl:opengl` | Exit-Code korrekt | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0176 | linux-arm64 (64-bit) | `sdl:opengl` | Log enthält Plattform | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0177 | linux-arm64 (64-bit) | `sdl:opengl` | Assets geladen | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0178 | linux-arm64 (64-bit) | `sdl:opengl` | UI-Text sichtbar | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0179 | linux-arm64 (64-bit) | `sdl:opengl` | Maus/ESC reagieren | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0180 | linux-arm64 (64-bit) | `sdl:opengl` | Speicher stabil | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0181 | linux-arm64 (64-bit) | `sdl:vulkan` | Fenster öffnet | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0182 | linux-arm64 (64-bit) | `sdl:vulkan` | Frame präsentiert | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0183 | linux-arm64 (64-bit) | `sdl:vulkan` | Screenshot erzeugt | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0184 | linux-arm64 (64-bit) | `sdl:vulkan` | Options zeigt Selection | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0185 | linux-arm64 (64-bit) | `sdl:vulkan` | Selection-Wechsel startet neu | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0186 | linux-arm64 (64-bit) | `sdl:vulkan` | Kein Crash im 60s-Loop | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0187 | linux-arm64 (64-bit) | `sdl:vulkan` | Exit-Code korrekt | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0188 | linux-arm64 (64-bit) | `sdl:vulkan` | Log enthält Plattform | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0189 | linux-arm64 (64-bit) | `sdl:vulkan` | Assets geladen | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0190 | linux-arm64 (64-bit) | `sdl:vulkan` | UI-Text sichtbar | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0191 | linux-arm64 (64-bit) | `sdl:vulkan` | Maus/ESC reagieren | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0192 | linux-arm64 (64-bit) | `sdl:vulkan` | Speicher stabil | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0193 | linux-arm64 (64-bit) | `sdl:directx` | Fenster öffnet | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0194 | linux-arm64 (64-bit) | `sdl:directx` | Frame präsentiert | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0195 | linux-arm64 (64-bit) | `sdl:directx` | Screenshot erzeugt | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0196 | linux-arm64 (64-bit) | `sdl:directx` | Options zeigt Selection | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0197 | linux-arm64 (64-bit) | `sdl:directx` | Selection-Wechsel startet neu | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0198 | linux-arm64 (64-bit) | `sdl:directx` | Kein Crash im 60s-Loop | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0199 | linux-arm64 (64-bit) | `sdl:directx` | Exit-Code korrekt | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0200 | linux-arm64 (64-bit) | `sdl:directx` | Log enthält Plattform | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0201 | linux-arm64 (64-bit) | `sdl:directx` | Assets geladen | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0202 | linux-arm64 (64-bit) | `sdl:directx` | UI-Text sichtbar | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0203 | linux-arm64 (64-bit) | `sdl:directx` | Maus/ESC reagieren | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0204 | linux-arm64 (64-bit) | `sdl:directx` | Speicher stabil | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0205 | linux-arm64 (64-bit) | `sdl:auto` | Fenster öffnet | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0206 | linux-arm64 (64-bit) | `sdl:auto` | Frame präsentiert | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0207 | linux-arm64 (64-bit) | `sdl:auto` | Screenshot erzeugt | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0208 | linux-arm64 (64-bit) | `sdl:auto` | Options zeigt Selection | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0209 | linux-arm64 (64-bit) | `sdl:auto` | Selection-Wechsel startet neu | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0210 | linux-arm64 (64-bit) | `sdl:auto` | Kein Crash im 60s-Loop | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0211 | linux-arm64 (64-bit) | `sdl:auto` | Exit-Code korrekt | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0212 | linux-arm64 (64-bit) | `sdl:auto` | Log enthält Plattform | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0213 | linux-arm64 (64-bit) | `sdl:auto` | Assets geladen | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0214 | linux-arm64 (64-bit) | `sdl:auto` | UI-Text sichtbar | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0215 | linux-arm64 (64-bit) | `sdl:auto` | Maus/ESC reagieren | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0216 | linux-arm64 (64-bit) | `sdl:auto` | Speicher stabil | Pakete: deb/tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0217 | windows-x86_64 (64-bit) | `native:opengl` | Fenster öffnet | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0218 | windows-x86_64 (64-bit) | `native:opengl` | Frame präsentiert | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0219 | windows-x86_64 (64-bit) | `native:opengl` | Screenshot erzeugt | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0220 | windows-x86_64 (64-bit) | `native:opengl` | Options zeigt Selection | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0221 | windows-x86_64 (64-bit) | `native:opengl` | Selection-Wechsel startet neu | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0222 | windows-x86_64 (64-bit) | `native:opengl` | Kein Crash im 60s-Loop | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0223 | windows-x86_64 (64-bit) | `native:opengl` | Exit-Code korrekt | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0224 | windows-x86_64 (64-bit) | `native:opengl` | Log enthält Plattform | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0225 | windows-x86_64 (64-bit) | `native:opengl` | Assets geladen | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0226 | windows-x86_64 (64-bit) | `native:opengl` | UI-Text sichtbar | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0227 | windows-x86_64 (64-bit) | `native:opengl` | Maus/ESC reagieren | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0228 | windows-x86_64 (64-bit) | `native:opengl` | Speicher stabil | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0229 | windows-x86_64 (64-bit) | `native:vulkan` | Fenster öffnet | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0230 | windows-x86_64 (64-bit) | `native:vulkan` | Frame präsentiert | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0231 | windows-x86_64 (64-bit) | `native:vulkan` | Screenshot erzeugt | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0232 | windows-x86_64 (64-bit) | `native:vulkan` | Options zeigt Selection | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0233 | windows-x86_64 (64-bit) | `native:vulkan` | Selection-Wechsel startet neu | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0234 | windows-x86_64 (64-bit) | `native:vulkan` | Kein Crash im 60s-Loop | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0235 | windows-x86_64 (64-bit) | `native:vulkan` | Exit-Code korrekt | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0236 | windows-x86_64 (64-bit) | `native:vulkan` | Log enthält Plattform | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0237 | windows-x86_64 (64-bit) | `native:vulkan` | Assets geladen | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0238 | windows-x86_64 (64-bit) | `native:vulkan` | UI-Text sichtbar | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0239 | windows-x86_64 (64-bit) | `native:vulkan` | Maus/ESC reagieren | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0240 | windows-x86_64 (64-bit) | `native:vulkan` | Speicher stabil | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0241 | windows-x86_64 (64-bit) | `sdl:opengl` | Fenster öffnet | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0242 | windows-x86_64 (64-bit) | `sdl:opengl` | Frame präsentiert | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0243 | windows-x86_64 (64-bit) | `sdl:opengl` | Screenshot erzeugt | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0244 | windows-x86_64 (64-bit) | `sdl:opengl` | Options zeigt Selection | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0245 | windows-x86_64 (64-bit) | `sdl:opengl` | Selection-Wechsel startet neu | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0246 | windows-x86_64 (64-bit) | `sdl:opengl` | Kein Crash im 60s-Loop | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0247 | windows-x86_64 (64-bit) | `sdl:opengl` | Exit-Code korrekt | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0248 | windows-x86_64 (64-bit) | `sdl:opengl` | Log enthält Plattform | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0249 | windows-x86_64 (64-bit) | `sdl:opengl` | Assets geladen | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0250 | windows-x86_64 (64-bit) | `sdl:opengl` | UI-Text sichtbar | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0251 | windows-x86_64 (64-bit) | `sdl:opengl` | Maus/ESC reagieren | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0252 | windows-x86_64 (64-bit) | `sdl:opengl` | Speicher stabil | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0253 | windows-x86_64 (64-bit) | `sdl:vulkan` | Fenster öffnet | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0254 | windows-x86_64 (64-bit) | `sdl:vulkan` | Frame präsentiert | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0255 | windows-x86_64 (64-bit) | `sdl:vulkan` | Screenshot erzeugt | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0256 | windows-x86_64 (64-bit) | `sdl:vulkan` | Options zeigt Selection | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0257 | windows-x86_64 (64-bit) | `sdl:vulkan` | Selection-Wechsel startet neu | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0258 | windows-x86_64 (64-bit) | `sdl:vulkan` | Kein Crash im 60s-Loop | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0259 | windows-x86_64 (64-bit) | `sdl:vulkan` | Exit-Code korrekt | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0260 | windows-x86_64 (64-bit) | `sdl:vulkan` | Log enthält Plattform | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0261 | windows-x86_64 (64-bit) | `sdl:vulkan` | Assets geladen | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0262 | windows-x86_64 (64-bit) | `sdl:vulkan` | UI-Text sichtbar | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0263 | windows-x86_64 (64-bit) | `sdl:vulkan` | Maus/ESC reagieren | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0264 | windows-x86_64 (64-bit) | `sdl:vulkan` | Speicher stabil | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0265 | windows-x86_64 (64-bit) | `sdl:directx` | Fenster öffnet | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0266 | windows-x86_64 (64-bit) | `sdl:directx` | Frame präsentiert | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0267 | windows-x86_64 (64-bit) | `sdl:directx` | Screenshot erzeugt | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0268 | windows-x86_64 (64-bit) | `sdl:directx` | Options zeigt Selection | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0269 | windows-x86_64 (64-bit) | `sdl:directx` | Selection-Wechsel startet neu | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0270 | windows-x86_64 (64-bit) | `sdl:directx` | Kein Crash im 60s-Loop | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0271 | windows-x86_64 (64-bit) | `sdl:directx` | Exit-Code korrekt | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0272 | windows-x86_64 (64-bit) | `sdl:directx` | Log enthält Plattform | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0273 | windows-x86_64 (64-bit) | `sdl:directx` | Assets geladen | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0274 | windows-x86_64 (64-bit) | `sdl:directx` | UI-Text sichtbar | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0275 | windows-x86_64 (64-bit) | `sdl:directx` | Maus/ESC reagieren | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0276 | windows-x86_64 (64-bit) | `sdl:directx` | Speicher stabil | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0277 | windows-x86_64 (64-bit) | `sdl:auto` | Fenster öffnet | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0278 | windows-x86_64 (64-bit) | `sdl:auto` | Frame präsentiert | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0279 | windows-x86_64 (64-bit) | `sdl:auto` | Screenshot erzeugt | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0280 | windows-x86_64 (64-bit) | `sdl:auto` | Options zeigt Selection | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0281 | windows-x86_64 (64-bit) | `sdl:auto` | Selection-Wechsel startet neu | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0282 | windows-x86_64 (64-bit) | `sdl:auto` | Kein Crash im 60s-Loop | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0283 | windows-x86_64 (64-bit) | `sdl:auto` | Exit-Code korrekt | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0284 | windows-x86_64 (64-bit) | `sdl:auto` | Log enthält Plattform | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0285 | windows-x86_64 (64-bit) | `sdl:auto` | Assets geladen | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0286 | windows-x86_64 (64-bit) | `sdl:auto` | UI-Text sichtbar | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0287 | windows-x86_64 (64-bit) | `sdl:auto` | Maus/ESC reagieren | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0288 | windows-x86_64 (64-bit) | `sdl:auto` | Speicher stabil | Pakete: msi/zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0289 | windows-arm64 (64-bit) | `native:opengl` | Fenster öffnet | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0290 | windows-arm64 (64-bit) | `native:opengl` | Frame präsentiert | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0291 | windows-arm64 (64-bit) | `native:opengl` | Screenshot erzeugt | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0292 | windows-arm64 (64-bit) | `native:opengl` | Options zeigt Selection | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0293 | windows-arm64 (64-bit) | `native:opengl` | Selection-Wechsel startet neu | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0294 | windows-arm64 (64-bit) | `native:opengl` | Kein Crash im 60s-Loop | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0295 | windows-arm64 (64-bit) | `native:opengl` | Exit-Code korrekt | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0296 | windows-arm64 (64-bit) | `native:opengl` | Log enthält Plattform | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0297 | windows-arm64 (64-bit) | `native:opengl` | Assets geladen | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0298 | windows-arm64 (64-bit) | `native:opengl` | UI-Text sichtbar | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0299 | windows-arm64 (64-bit) | `native:opengl` | Maus/ESC reagieren | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0300 | windows-arm64 (64-bit) | `native:opengl` | Speicher stabil | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0301 | windows-arm64 (64-bit) | `native:vulkan` | Fenster öffnet | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0302 | windows-arm64 (64-bit) | `native:vulkan` | Frame präsentiert | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0303 | windows-arm64 (64-bit) | `native:vulkan` | Screenshot erzeugt | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0304 | windows-arm64 (64-bit) | `native:vulkan` | Options zeigt Selection | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0305 | windows-arm64 (64-bit) | `native:vulkan` | Selection-Wechsel startet neu | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0306 | windows-arm64 (64-bit) | `native:vulkan` | Kein Crash im 60s-Loop | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0307 | windows-arm64 (64-bit) | `native:vulkan` | Exit-Code korrekt | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0308 | windows-arm64 (64-bit) | `native:vulkan` | Log enthält Plattform | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0309 | windows-arm64 (64-bit) | `native:vulkan` | Assets geladen | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0310 | windows-arm64 (64-bit) | `native:vulkan` | UI-Text sichtbar | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0311 | windows-arm64 (64-bit) | `native:vulkan` | Maus/ESC reagieren | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0312 | windows-arm64 (64-bit) | `native:vulkan` | Speicher stabil | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0313 | windows-arm64 (64-bit) | `sdl:opengl` | Fenster öffnet | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0314 | windows-arm64 (64-bit) | `sdl:opengl` | Frame präsentiert | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0315 | windows-arm64 (64-bit) | `sdl:opengl` | Screenshot erzeugt | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0316 | windows-arm64 (64-bit) | `sdl:opengl` | Options zeigt Selection | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0317 | windows-arm64 (64-bit) | `sdl:opengl` | Selection-Wechsel startet neu | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0318 | windows-arm64 (64-bit) | `sdl:opengl` | Kein Crash im 60s-Loop | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0319 | windows-arm64 (64-bit) | `sdl:opengl` | Exit-Code korrekt | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0320 | windows-arm64 (64-bit) | `sdl:opengl` | Log enthält Plattform | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0321 | windows-arm64 (64-bit) | `sdl:opengl` | Assets geladen | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0322 | windows-arm64 (64-bit) | `sdl:opengl` | UI-Text sichtbar | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0323 | windows-arm64 (64-bit) | `sdl:opengl` | Maus/ESC reagieren | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0324 | windows-arm64 (64-bit) | `sdl:opengl` | Speicher stabil | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0325 | windows-arm64 (64-bit) | `sdl:vulkan` | Fenster öffnet | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0326 | windows-arm64 (64-bit) | `sdl:vulkan` | Frame präsentiert | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0327 | windows-arm64 (64-bit) | `sdl:vulkan` | Screenshot erzeugt | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0328 | windows-arm64 (64-bit) | `sdl:vulkan` | Options zeigt Selection | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0329 | windows-arm64 (64-bit) | `sdl:vulkan` | Selection-Wechsel startet neu | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0330 | windows-arm64 (64-bit) | `sdl:vulkan` | Kein Crash im 60s-Loop | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0331 | windows-arm64 (64-bit) | `sdl:vulkan` | Exit-Code korrekt | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0332 | windows-arm64 (64-bit) | `sdl:vulkan` | Log enthält Plattform | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0333 | windows-arm64 (64-bit) | `sdl:vulkan` | Assets geladen | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0334 | windows-arm64 (64-bit) | `sdl:vulkan` | UI-Text sichtbar | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0335 | windows-arm64 (64-bit) | `sdl:vulkan` | Maus/ESC reagieren | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0336 | windows-arm64 (64-bit) | `sdl:vulkan` | Speicher stabil | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0337 | windows-arm64 (64-bit) | `sdl:directx` | Fenster öffnet | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0338 | windows-arm64 (64-bit) | `sdl:directx` | Frame präsentiert | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0339 | windows-arm64 (64-bit) | `sdl:directx` | Screenshot erzeugt | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0340 | windows-arm64 (64-bit) | `sdl:directx` | Options zeigt Selection | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0341 | windows-arm64 (64-bit) | `sdl:directx` | Selection-Wechsel startet neu | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0342 | windows-arm64 (64-bit) | `sdl:directx` | Kein Crash im 60s-Loop | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0343 | windows-arm64 (64-bit) | `sdl:directx` | Exit-Code korrekt | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0344 | windows-arm64 (64-bit) | `sdl:directx` | Log enthält Plattform | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0345 | windows-arm64 (64-bit) | `sdl:directx` | Assets geladen | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0346 | windows-arm64 (64-bit) | `sdl:directx` | UI-Text sichtbar | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0347 | windows-arm64 (64-bit) | `sdl:directx` | Maus/ESC reagieren | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0348 | windows-arm64 (64-bit) | `sdl:directx` | Speicher stabil | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0349 | windows-arm64 (64-bit) | `sdl:auto` | Fenster öffnet | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0350 | windows-arm64 (64-bit) | `sdl:auto` | Frame präsentiert | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0351 | windows-arm64 (64-bit) | `sdl:auto` | Screenshot erzeugt | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0352 | windows-arm64 (64-bit) | `sdl:auto` | Options zeigt Selection | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0353 | windows-arm64 (64-bit) | `sdl:auto` | Selection-Wechsel startet neu | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0354 | windows-arm64 (64-bit) | `sdl:auto` | Kein Crash im 60s-Loop | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0355 | windows-arm64 (64-bit) | `sdl:auto` | Exit-Code korrekt | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0356 | windows-arm64 (64-bit) | `sdl:auto` | Log enthält Plattform | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0357 | windows-arm64 (64-bit) | `sdl:auto` | Assets geladen | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0358 | windows-arm64 (64-bit) | `sdl:auto` | UI-Text sichtbar | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0359 | windows-arm64 (64-bit) | `sdl:auto` | Maus/ESC reagieren | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0360 | windows-arm64 (64-bit) | `sdl:auto` | Speicher stabil | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0361 | windows-x86 (32-bit) | `native:opengl` | Fenster öffnet | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0362 | windows-x86 (32-bit) | `native:opengl` | Frame präsentiert | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0363 | windows-x86 (32-bit) | `native:opengl` | Screenshot erzeugt | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0364 | windows-x86 (32-bit) | `native:opengl` | Options zeigt Selection | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0365 | windows-x86 (32-bit) | `native:opengl` | Selection-Wechsel startet neu | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0366 | windows-x86 (32-bit) | `native:opengl` | Kein Crash im 60s-Loop | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0367 | windows-x86 (32-bit) | `native:opengl` | Exit-Code korrekt | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0368 | windows-x86 (32-bit) | `native:opengl` | Log enthält Plattform | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0369 | windows-x86 (32-bit) | `native:opengl` | Assets geladen | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0370 | windows-x86 (32-bit) | `native:opengl` | UI-Text sichtbar | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0371 | windows-x86 (32-bit) | `native:opengl` | Maus/ESC reagieren | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0372 | windows-x86 (32-bit) | `native:opengl` | Speicher stabil | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0373 | windows-x86 (32-bit) | `native:vulkan` | Fenster öffnet | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0374 | windows-x86 (32-bit) | `native:vulkan` | Frame präsentiert | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0375 | windows-x86 (32-bit) | `native:vulkan` | Screenshot erzeugt | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0376 | windows-x86 (32-bit) | `native:vulkan` | Options zeigt Selection | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0377 | windows-x86 (32-bit) | `native:vulkan` | Selection-Wechsel startet neu | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0378 | windows-x86 (32-bit) | `native:vulkan` | Kein Crash im 60s-Loop | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0379 | windows-x86 (32-bit) | `native:vulkan` | Exit-Code korrekt | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0380 | windows-x86 (32-bit) | `native:vulkan` | Log enthält Plattform | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0381 | windows-x86 (32-bit) | `native:vulkan` | Assets geladen | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0382 | windows-x86 (32-bit) | `native:vulkan` | UI-Text sichtbar | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0383 | windows-x86 (32-bit) | `native:vulkan` | Maus/ESC reagieren | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0384 | windows-x86 (32-bit) | `native:vulkan` | Speicher stabil | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0385 | windows-x86 (32-bit) | `sdl:opengl` | Fenster öffnet | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0386 | windows-x86 (32-bit) | `sdl:opengl` | Frame präsentiert | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0387 | windows-x86 (32-bit) | `sdl:opengl` | Screenshot erzeugt | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0388 | windows-x86 (32-bit) | `sdl:opengl` | Options zeigt Selection | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0389 | windows-x86 (32-bit) | `sdl:opengl` | Selection-Wechsel startet neu | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0390 | windows-x86 (32-bit) | `sdl:opengl` | Kein Crash im 60s-Loop | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0391 | windows-x86 (32-bit) | `sdl:opengl` | Exit-Code korrekt | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0392 | windows-x86 (32-bit) | `sdl:opengl` | Log enthält Plattform | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0393 | windows-x86 (32-bit) | `sdl:opengl` | Assets geladen | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0394 | windows-x86 (32-bit) | `sdl:opengl` | UI-Text sichtbar | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0395 | windows-x86 (32-bit) | `sdl:opengl` | Maus/ESC reagieren | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0396 | windows-x86 (32-bit) | `sdl:opengl` | Speicher stabil | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0397 | windows-x86 (32-bit) | `sdl:vulkan` | Fenster öffnet | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0398 | windows-x86 (32-bit) | `sdl:vulkan` | Frame präsentiert | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0399 | windows-x86 (32-bit) | `sdl:vulkan` | Screenshot erzeugt | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0400 | windows-x86 (32-bit) | `sdl:vulkan` | Options zeigt Selection | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0401 | windows-x86 (32-bit) | `sdl:vulkan` | Selection-Wechsel startet neu | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0402 | windows-x86 (32-bit) | `sdl:vulkan` | Kein Crash im 60s-Loop | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0403 | windows-x86 (32-bit) | `sdl:vulkan` | Exit-Code korrekt | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0404 | windows-x86 (32-bit) | `sdl:vulkan` | Log enthält Plattform | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0405 | windows-x86 (32-bit) | `sdl:vulkan` | Assets geladen | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0406 | windows-x86 (32-bit) | `sdl:vulkan` | UI-Text sichtbar | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0407 | windows-x86 (32-bit) | `sdl:vulkan` | Maus/ESC reagieren | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0408 | windows-x86 (32-bit) | `sdl:vulkan` | Speicher stabil | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0409 | windows-x86 (32-bit) | `sdl:directx` | Fenster öffnet | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0410 | windows-x86 (32-bit) | `sdl:directx` | Frame präsentiert | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0411 | windows-x86 (32-bit) | `sdl:directx` | Screenshot erzeugt | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0412 | windows-x86 (32-bit) | `sdl:directx` | Options zeigt Selection | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0413 | windows-x86 (32-bit) | `sdl:directx` | Selection-Wechsel startet neu | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0414 | windows-x86 (32-bit) | `sdl:directx` | Kein Crash im 60s-Loop | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0415 | windows-x86 (32-bit) | `sdl:directx` | Exit-Code korrekt | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0416 | windows-x86 (32-bit) | `sdl:directx` | Log enthält Plattform | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0417 | windows-x86 (32-bit) | `sdl:directx` | Assets geladen | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0418 | windows-x86 (32-bit) | `sdl:directx` | UI-Text sichtbar | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0419 | windows-x86 (32-bit) | `sdl:directx` | Maus/ESC reagieren | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0420 | windows-x86 (32-bit) | `sdl:directx` | Speicher stabil | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0421 | windows-x86 (32-bit) | `sdl:auto` | Fenster öffnet | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0422 | windows-x86 (32-bit) | `sdl:auto` | Frame präsentiert | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0423 | windows-x86 (32-bit) | `sdl:auto` | Screenshot erzeugt | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0424 | windows-x86 (32-bit) | `sdl:auto` | Options zeigt Selection | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0425 | windows-x86 (32-bit) | `sdl:auto` | Selection-Wechsel startet neu | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0426 | windows-x86 (32-bit) | `sdl:auto` | Kein Crash im 60s-Loop | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0427 | windows-x86 (32-bit) | `sdl:auto` | Exit-Code korrekt | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0428 | windows-x86 (32-bit) | `sdl:auto` | Log enthält Plattform | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0429 | windows-x86 (32-bit) | `sdl:auto` | Assets geladen | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0430 | windows-x86 (32-bit) | `sdl:auto` | UI-Text sichtbar | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0431 | windows-x86 (32-bit) | `sdl:auto` | Maus/ESC reagieren | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0432 | windows-x86 (32-bit) | `sdl:auto` | Speicher stabil | Pakete: zip | Ergebnis: PASS/FAIL dokumentiert.
AC-0433 | macos-universal (64-bit) | `native:opengl` | Fenster öffnet | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0434 | macos-universal (64-bit) | `native:opengl` | Frame präsentiert | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0435 | macos-universal (64-bit) | `native:opengl` | Screenshot erzeugt | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0436 | macos-universal (64-bit) | `native:opengl` | Options zeigt Selection | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0437 | macos-universal (64-bit) | `native:opengl` | Selection-Wechsel startet neu | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0438 | macos-universal (64-bit) | `native:opengl` | Kein Crash im 60s-Loop | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0439 | macos-universal (64-bit) | `native:opengl` | Exit-Code korrekt | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0440 | macos-universal (64-bit) | `native:opengl` | Log enthält Plattform | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0441 | macos-universal (64-bit) | `native:opengl` | Assets geladen | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0442 | macos-universal (64-bit) | `native:opengl` | UI-Text sichtbar | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0443 | macos-universal (64-bit) | `native:opengl` | Maus/ESC reagieren | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0444 | macos-universal (64-bit) | `native:opengl` | Speicher stabil | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0445 | macos-universal (64-bit) | `native:vulkan` | Fenster öffnet | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0446 | macos-universal (64-bit) | `native:vulkan` | Frame präsentiert | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0447 | macos-universal (64-bit) | `native:vulkan` | Screenshot erzeugt | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0448 | macos-universal (64-bit) | `native:vulkan` | Options zeigt Selection | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0449 | macos-universal (64-bit) | `native:vulkan` | Selection-Wechsel startet neu | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0450 | macos-universal (64-bit) | `native:vulkan` | Kein Crash im 60s-Loop | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0451 | macos-universal (64-bit) | `native:vulkan` | Exit-Code korrekt | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0452 | macos-universal (64-bit) | `native:vulkan` | Log enthält Plattform | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0453 | macos-universal (64-bit) | `native:vulkan` | Assets geladen | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0454 | macos-universal (64-bit) | `native:vulkan` | UI-Text sichtbar | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0455 | macos-universal (64-bit) | `native:vulkan` | Maus/ESC reagieren | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0456 | macos-universal (64-bit) | `native:vulkan` | Speicher stabil | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0457 | macos-universal (64-bit) | `sdl:opengl` | Fenster öffnet | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0458 | macos-universal (64-bit) | `sdl:opengl` | Frame präsentiert | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0459 | macos-universal (64-bit) | `sdl:opengl` | Screenshot erzeugt | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0460 | macos-universal (64-bit) | `sdl:opengl` | Options zeigt Selection | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0461 | macos-universal (64-bit) | `sdl:opengl` | Selection-Wechsel startet neu | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0462 | macos-universal (64-bit) | `sdl:opengl` | Kein Crash im 60s-Loop | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0463 | macos-universal (64-bit) | `sdl:opengl` | Exit-Code korrekt | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0464 | macos-universal (64-bit) | `sdl:opengl` | Log enthält Plattform | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0465 | macos-universal (64-bit) | `sdl:opengl` | Assets geladen | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0466 | macos-universal (64-bit) | `sdl:opengl` | UI-Text sichtbar | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0467 | macos-universal (64-bit) | `sdl:opengl` | Maus/ESC reagieren | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0468 | macos-universal (64-bit) | `sdl:opengl` | Speicher stabil | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0469 | macos-universal (64-bit) | `sdl:vulkan` | Fenster öffnet | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0470 | macos-universal (64-bit) | `sdl:vulkan` | Frame präsentiert | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0471 | macos-universal (64-bit) | `sdl:vulkan` | Screenshot erzeugt | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0472 | macos-universal (64-bit) | `sdl:vulkan` | Options zeigt Selection | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0473 | macos-universal (64-bit) | `sdl:vulkan` | Selection-Wechsel startet neu | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0474 | macos-universal (64-bit) | `sdl:vulkan` | Kein Crash im 60s-Loop | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0475 | macos-universal (64-bit) | `sdl:vulkan` | Exit-Code korrekt | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0476 | macos-universal (64-bit) | `sdl:vulkan` | Log enthält Plattform | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0477 | macos-universal (64-bit) | `sdl:vulkan` | Assets geladen | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0478 | macos-universal (64-bit) | `sdl:vulkan` | UI-Text sichtbar | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0479 | macos-universal (64-bit) | `sdl:vulkan` | Maus/ESC reagieren | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0480 | macos-universal (64-bit) | `sdl:vulkan` | Speicher stabil | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0481 | macos-universal (64-bit) | `sdl:directx` | Fenster öffnet | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0482 | macos-universal (64-bit) | `sdl:directx` | Frame präsentiert | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0483 | macos-universal (64-bit) | `sdl:directx` | Screenshot erzeugt | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0484 | macos-universal (64-bit) | `sdl:directx` | Options zeigt Selection | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0485 | macos-universal (64-bit) | `sdl:directx` | Selection-Wechsel startet neu | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0486 | macos-universal (64-bit) | `sdl:directx` | Kein Crash im 60s-Loop | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0487 | macos-universal (64-bit) | `sdl:directx` | Exit-Code korrekt | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0488 | macos-universal (64-bit) | `sdl:directx` | Log enthält Plattform | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0489 | macos-universal (64-bit) | `sdl:directx` | Assets geladen | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0490 | macos-universal (64-bit) | `sdl:directx` | UI-Text sichtbar | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0491 | macos-universal (64-bit) | `sdl:directx` | Maus/ESC reagieren | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0492 | macos-universal (64-bit) | `sdl:directx` | Speicher stabil | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0493 | macos-universal (64-bit) | `sdl:auto` | Fenster öffnet | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0494 | macos-universal (64-bit) | `sdl:auto` | Frame präsentiert | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0495 | macos-universal (64-bit) | `sdl:auto` | Screenshot erzeugt | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0496 | macos-universal (64-bit) | `sdl:auto` | Options zeigt Selection | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0497 | macos-universal (64-bit) | `sdl:auto` | Selection-Wechsel startet neu | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0498 | macos-universal (64-bit) | `sdl:auto` | Kein Crash im 60s-Loop | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0499 | macos-universal (64-bit) | `sdl:auto` | Exit-Code korrekt | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0500 | macos-universal (64-bit) | `sdl:auto` | Log enthält Plattform | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0501 | macos-universal (64-bit) | `sdl:auto` | Assets geladen | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0502 | macos-universal (64-bit) | `sdl:auto` | UI-Text sichtbar | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0503 | macos-universal (64-bit) | `sdl:auto` | Maus/ESC reagieren | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0504 | macos-universal (64-bit) | `sdl:auto` | Speicher stabil | Pakete: dmg/pkg | Ergebnis: PASS/FAIL dokumentiert.
AC-0505 | android-arm64-v8a (64-bit) | `native:opengl` | Fenster öffnet | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0506 | android-arm64-v8a (64-bit) | `native:opengl` | Frame präsentiert | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0507 | android-arm64-v8a (64-bit) | `native:opengl` | Screenshot erzeugt | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0508 | android-arm64-v8a (64-bit) | `native:opengl` | Options zeigt Selection | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0509 | android-arm64-v8a (64-bit) | `native:opengl` | Selection-Wechsel startet neu | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0510 | android-arm64-v8a (64-bit) | `native:opengl` | Kein Crash im 60s-Loop | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0511 | android-arm64-v8a (64-bit) | `native:opengl` | Exit-Code korrekt | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0512 | android-arm64-v8a (64-bit) | `native:opengl` | Log enthält Plattform | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0513 | android-arm64-v8a (64-bit) | `native:opengl` | Assets geladen | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0514 | android-arm64-v8a (64-bit) | `native:opengl` | UI-Text sichtbar | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0515 | android-arm64-v8a (64-bit) | `native:opengl` | Maus/ESC reagieren | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0516 | android-arm64-v8a (64-bit) | `native:opengl` | Speicher stabil | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0517 | android-arm64-v8a (64-bit) | `native:vulkan` | Fenster öffnet | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0518 | android-arm64-v8a (64-bit) | `native:vulkan` | Frame präsentiert | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0519 | android-arm64-v8a (64-bit) | `native:vulkan` | Screenshot erzeugt | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0520 | android-arm64-v8a (64-bit) | `native:vulkan` | Options zeigt Selection | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0521 | android-arm64-v8a (64-bit) | `native:vulkan` | Selection-Wechsel startet neu | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0522 | android-arm64-v8a (64-bit) | `native:vulkan` | Kein Crash im 60s-Loop | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0523 | android-arm64-v8a (64-bit) | `native:vulkan` | Exit-Code korrekt | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0524 | android-arm64-v8a (64-bit) | `native:vulkan` | Log enthält Plattform | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0525 | android-arm64-v8a (64-bit) | `native:vulkan` | Assets geladen | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0526 | android-arm64-v8a (64-bit) | `native:vulkan` | UI-Text sichtbar | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0527 | android-arm64-v8a (64-bit) | `native:vulkan` | Maus/ESC reagieren | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0528 | android-arm64-v8a (64-bit) | `native:vulkan` | Speicher stabil | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0529 | android-arm64-v8a (64-bit) | `sdl:opengl` | Fenster öffnet | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0530 | android-arm64-v8a (64-bit) | `sdl:opengl` | Frame präsentiert | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0531 | android-arm64-v8a (64-bit) | `sdl:opengl` | Screenshot erzeugt | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0532 | android-arm64-v8a (64-bit) | `sdl:opengl` | Options zeigt Selection | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0533 | android-arm64-v8a (64-bit) | `sdl:opengl` | Selection-Wechsel startet neu | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0534 | android-arm64-v8a (64-bit) | `sdl:opengl` | Kein Crash im 60s-Loop | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0535 | android-arm64-v8a (64-bit) | `sdl:opengl` | Exit-Code korrekt | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0536 | android-arm64-v8a (64-bit) | `sdl:opengl` | Log enthält Plattform | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0537 | android-arm64-v8a (64-bit) | `sdl:opengl` | Assets geladen | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0538 | android-arm64-v8a (64-bit) | `sdl:opengl` | UI-Text sichtbar | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0539 | android-arm64-v8a (64-bit) | `sdl:opengl` | Maus/ESC reagieren | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0540 | android-arm64-v8a (64-bit) | `sdl:opengl` | Speicher stabil | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0541 | android-arm64-v8a (64-bit) | `sdl:vulkan` | Fenster öffnet | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0542 | android-arm64-v8a (64-bit) | `sdl:vulkan` | Frame präsentiert | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0543 | android-arm64-v8a (64-bit) | `sdl:vulkan` | Screenshot erzeugt | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0544 | android-arm64-v8a (64-bit) | `sdl:vulkan` | Options zeigt Selection | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0545 | android-arm64-v8a (64-bit) | `sdl:vulkan` | Selection-Wechsel startet neu | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0546 | android-arm64-v8a (64-bit) | `sdl:vulkan` | Kein Crash im 60s-Loop | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0547 | android-arm64-v8a (64-bit) | `sdl:vulkan` | Exit-Code korrekt | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0548 | android-arm64-v8a (64-bit) | `sdl:vulkan` | Log enthält Plattform | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0549 | android-arm64-v8a (64-bit) | `sdl:vulkan` | Assets geladen | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0550 | android-arm64-v8a (64-bit) | `sdl:vulkan` | UI-Text sichtbar | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0551 | android-arm64-v8a (64-bit) | `sdl:vulkan` | Maus/ESC reagieren | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0552 | android-arm64-v8a (64-bit) | `sdl:vulkan` | Speicher stabil | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0553 | android-arm64-v8a (64-bit) | `sdl:directx` | Fenster öffnet | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0554 | android-arm64-v8a (64-bit) | `sdl:directx` | Frame präsentiert | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0555 | android-arm64-v8a (64-bit) | `sdl:directx` | Screenshot erzeugt | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0556 | android-arm64-v8a (64-bit) | `sdl:directx` | Options zeigt Selection | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0557 | android-arm64-v8a (64-bit) | `sdl:directx` | Selection-Wechsel startet neu | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0558 | android-arm64-v8a (64-bit) | `sdl:directx` | Kein Crash im 60s-Loop | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0559 | android-arm64-v8a (64-bit) | `sdl:directx` | Exit-Code korrekt | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0560 | android-arm64-v8a (64-bit) | `sdl:directx` | Log enthält Plattform | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0561 | android-arm64-v8a (64-bit) | `sdl:directx` | Assets geladen | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0562 | android-arm64-v8a (64-bit) | `sdl:directx` | UI-Text sichtbar | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0563 | android-arm64-v8a (64-bit) | `sdl:directx` | Maus/ESC reagieren | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0564 | android-arm64-v8a (64-bit) | `sdl:directx` | Speicher stabil | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0565 | android-arm64-v8a (64-bit) | `sdl:auto` | Fenster öffnet | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0566 | android-arm64-v8a (64-bit) | `sdl:auto` | Frame präsentiert | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0567 | android-arm64-v8a (64-bit) | `sdl:auto` | Screenshot erzeugt | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0568 | android-arm64-v8a (64-bit) | `sdl:auto` | Options zeigt Selection | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0569 | android-arm64-v8a (64-bit) | `sdl:auto` | Selection-Wechsel startet neu | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0570 | android-arm64-v8a (64-bit) | `sdl:auto` | Kein Crash im 60s-Loop | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0571 | android-arm64-v8a (64-bit) | `sdl:auto` | Exit-Code korrekt | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0572 | android-arm64-v8a (64-bit) | `sdl:auto` | Log enthält Plattform | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0573 | android-arm64-v8a (64-bit) | `sdl:auto` | Assets geladen | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0574 | android-arm64-v8a (64-bit) | `sdl:auto` | UI-Text sichtbar | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0575 | android-arm64-v8a (64-bit) | `sdl:auto` | Maus/ESC reagieren | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0576 | android-arm64-v8a (64-bit) | `sdl:auto` | Speicher stabil | Pakete: apk/aab | Ergebnis: PASS/FAIL dokumentiert.
AC-0577 | android-x86_64 (64-bit) | `native:opengl` | Fenster öffnet | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0578 | android-x86_64 (64-bit) | `native:opengl` | Frame präsentiert | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0579 | android-x86_64 (64-bit) | `native:opengl` | Screenshot erzeugt | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0580 | android-x86_64 (64-bit) | `native:opengl` | Options zeigt Selection | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0581 | android-x86_64 (64-bit) | `native:opengl` | Selection-Wechsel startet neu | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0582 | android-x86_64 (64-bit) | `native:opengl` | Kein Crash im 60s-Loop | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0583 | android-x86_64 (64-bit) | `native:opengl` | Exit-Code korrekt | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0584 | android-x86_64 (64-bit) | `native:opengl` | Log enthält Plattform | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0585 | android-x86_64 (64-bit) | `native:opengl` | Assets geladen | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0586 | android-x86_64 (64-bit) | `native:opengl` | UI-Text sichtbar | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0587 | android-x86_64 (64-bit) | `native:opengl` | Maus/ESC reagieren | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0588 | android-x86_64 (64-bit) | `native:opengl` | Speicher stabil | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0589 | android-x86_64 (64-bit) | `native:vulkan` | Fenster öffnet | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0590 | android-x86_64 (64-bit) | `native:vulkan` | Frame präsentiert | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0591 | android-x86_64 (64-bit) | `native:vulkan` | Screenshot erzeugt | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0592 | android-x86_64 (64-bit) | `native:vulkan` | Options zeigt Selection | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0593 | android-x86_64 (64-bit) | `native:vulkan` | Selection-Wechsel startet neu | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0594 | android-x86_64 (64-bit) | `native:vulkan` | Kein Crash im 60s-Loop | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0595 | android-x86_64 (64-bit) | `native:vulkan` | Exit-Code korrekt | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0596 | android-x86_64 (64-bit) | `native:vulkan` | Log enthält Plattform | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0597 | android-x86_64 (64-bit) | `native:vulkan` | Assets geladen | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0598 | android-x86_64 (64-bit) | `native:vulkan` | UI-Text sichtbar | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0599 | android-x86_64 (64-bit) | `native:vulkan` | Maus/ESC reagieren | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0600 | android-x86_64 (64-bit) | `native:vulkan` | Speicher stabil | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0601 | android-x86_64 (64-bit) | `sdl:opengl` | Fenster öffnet | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0602 | android-x86_64 (64-bit) | `sdl:opengl` | Frame präsentiert | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0603 | android-x86_64 (64-bit) | `sdl:opengl` | Screenshot erzeugt | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0604 | android-x86_64 (64-bit) | `sdl:opengl` | Options zeigt Selection | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0605 | android-x86_64 (64-bit) | `sdl:opengl` | Selection-Wechsel startet neu | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0606 | android-x86_64 (64-bit) | `sdl:opengl` | Kein Crash im 60s-Loop | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0607 | android-x86_64 (64-bit) | `sdl:opengl` | Exit-Code korrekt | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0608 | android-x86_64 (64-bit) | `sdl:opengl` | Log enthält Plattform | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0609 | android-x86_64 (64-bit) | `sdl:opengl` | Assets geladen | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0610 | android-x86_64 (64-bit) | `sdl:opengl` | UI-Text sichtbar | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0611 | android-x86_64 (64-bit) | `sdl:opengl` | Maus/ESC reagieren | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0612 | android-x86_64 (64-bit) | `sdl:opengl` | Speicher stabil | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0613 | android-x86_64 (64-bit) | `sdl:vulkan` | Fenster öffnet | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0614 | android-x86_64 (64-bit) | `sdl:vulkan` | Frame präsentiert | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0615 | android-x86_64 (64-bit) | `sdl:vulkan` | Screenshot erzeugt | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0616 | android-x86_64 (64-bit) | `sdl:vulkan` | Options zeigt Selection | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0617 | android-x86_64 (64-bit) | `sdl:vulkan` | Selection-Wechsel startet neu | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0618 | android-x86_64 (64-bit) | `sdl:vulkan` | Kein Crash im 60s-Loop | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0619 | android-x86_64 (64-bit) | `sdl:vulkan` | Exit-Code korrekt | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0620 | android-x86_64 (64-bit) | `sdl:vulkan` | Log enthält Plattform | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0621 | android-x86_64 (64-bit) | `sdl:vulkan` | Assets geladen | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0622 | android-x86_64 (64-bit) | `sdl:vulkan` | UI-Text sichtbar | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0623 | android-x86_64 (64-bit) | `sdl:vulkan` | Maus/ESC reagieren | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0624 | android-x86_64 (64-bit) | `sdl:vulkan` | Speicher stabil | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0625 | android-x86_64 (64-bit) | `sdl:directx` | Fenster öffnet | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0626 | android-x86_64 (64-bit) | `sdl:directx` | Frame präsentiert | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0627 | android-x86_64 (64-bit) | `sdl:directx` | Screenshot erzeugt | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0628 | android-x86_64 (64-bit) | `sdl:directx` | Options zeigt Selection | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0629 | android-x86_64 (64-bit) | `sdl:directx` | Selection-Wechsel startet neu | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0630 | android-x86_64 (64-bit) | `sdl:directx` | Kein Crash im 60s-Loop | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0631 | android-x86_64 (64-bit) | `sdl:directx` | Exit-Code korrekt | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0632 | android-x86_64 (64-bit) | `sdl:directx` | Log enthält Plattform | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0633 | android-x86_64 (64-bit) | `sdl:directx` | Assets geladen | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0634 | android-x86_64 (64-bit) | `sdl:directx` | UI-Text sichtbar | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0635 | android-x86_64 (64-bit) | `sdl:directx` | Maus/ESC reagieren | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0636 | android-x86_64 (64-bit) | `sdl:directx` | Speicher stabil | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0637 | android-x86_64 (64-bit) | `sdl:auto` | Fenster öffnet | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0638 | android-x86_64 (64-bit) | `sdl:auto` | Frame präsentiert | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0639 | android-x86_64 (64-bit) | `sdl:auto` | Screenshot erzeugt | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0640 | android-x86_64 (64-bit) | `sdl:auto` | Options zeigt Selection | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0641 | android-x86_64 (64-bit) | `sdl:auto` | Selection-Wechsel startet neu | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0642 | android-x86_64 (64-bit) | `sdl:auto` | Kein Crash im 60s-Loop | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0643 | android-x86_64 (64-bit) | `sdl:auto` | Exit-Code korrekt | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0644 | android-x86_64 (64-bit) | `sdl:auto` | Log enthält Plattform | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0645 | android-x86_64 (64-bit) | `sdl:auto` | Assets geladen | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0646 | android-x86_64 (64-bit) | `sdl:auto` | UI-Text sichtbar | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0647 | android-x86_64 (64-bit) | `sdl:auto` | Maus/ESC reagieren | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0648 | android-x86_64 (64-bit) | `sdl:auto` | Speicher stabil | Pakete: apk | Ergebnis: PASS/FAIL dokumentiert.
AC-0649 | freebsd-x86_64 (64-bit) | `native:opengl` | Fenster öffnet | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0650 | freebsd-x86_64 (64-bit) | `native:opengl` | Frame präsentiert | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0651 | freebsd-x86_64 (64-bit) | `native:opengl` | Screenshot erzeugt | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0652 | freebsd-x86_64 (64-bit) | `native:opengl` | Options zeigt Selection | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0653 | freebsd-x86_64 (64-bit) | `native:opengl` | Selection-Wechsel startet neu | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0654 | freebsd-x86_64 (64-bit) | `native:opengl` | Kein Crash im 60s-Loop | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0655 | freebsd-x86_64 (64-bit) | `native:opengl` | Exit-Code korrekt | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0656 | freebsd-x86_64 (64-bit) | `native:opengl` | Log enthält Plattform | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0657 | freebsd-x86_64 (64-bit) | `native:opengl` | Assets geladen | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0658 | freebsd-x86_64 (64-bit) | `native:opengl` | UI-Text sichtbar | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0659 | freebsd-x86_64 (64-bit) | `native:opengl` | Maus/ESC reagieren | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0660 | freebsd-x86_64 (64-bit) | `native:opengl` | Speicher stabil | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0661 | freebsd-x86_64 (64-bit) | `native:vulkan` | Fenster öffnet | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0662 | freebsd-x86_64 (64-bit) | `native:vulkan` | Frame präsentiert | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0663 | freebsd-x86_64 (64-bit) | `native:vulkan` | Screenshot erzeugt | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0664 | freebsd-x86_64 (64-bit) | `native:vulkan` | Options zeigt Selection | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0665 | freebsd-x86_64 (64-bit) | `native:vulkan` | Selection-Wechsel startet neu | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0666 | freebsd-x86_64 (64-bit) | `native:vulkan` | Kein Crash im 60s-Loop | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0667 | freebsd-x86_64 (64-bit) | `native:vulkan` | Exit-Code korrekt | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0668 | freebsd-x86_64 (64-bit) | `native:vulkan` | Log enthält Plattform | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0669 | freebsd-x86_64 (64-bit) | `native:vulkan` | Assets geladen | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0670 | freebsd-x86_64 (64-bit) | `native:vulkan` | UI-Text sichtbar | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0671 | freebsd-x86_64 (64-bit) | `native:vulkan` | Maus/ESC reagieren | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0672 | freebsd-x86_64 (64-bit) | `native:vulkan` | Speicher stabil | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0673 | freebsd-x86_64 (64-bit) | `sdl:opengl` | Fenster öffnet | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0674 | freebsd-x86_64 (64-bit) | `sdl:opengl` | Frame präsentiert | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0675 | freebsd-x86_64 (64-bit) | `sdl:opengl` | Screenshot erzeugt | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0676 | freebsd-x86_64 (64-bit) | `sdl:opengl` | Options zeigt Selection | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0677 | freebsd-x86_64 (64-bit) | `sdl:opengl` | Selection-Wechsel startet neu | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0678 | freebsd-x86_64 (64-bit) | `sdl:opengl` | Kein Crash im 60s-Loop | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0679 | freebsd-x86_64 (64-bit) | `sdl:opengl` | Exit-Code korrekt | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0680 | freebsd-x86_64 (64-bit) | `sdl:opengl` | Log enthält Plattform | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0681 | freebsd-x86_64 (64-bit) | `sdl:opengl` | Assets geladen | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0682 | freebsd-x86_64 (64-bit) | `sdl:opengl` | UI-Text sichtbar | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0683 | freebsd-x86_64 (64-bit) | `sdl:opengl` | Maus/ESC reagieren | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0684 | freebsd-x86_64 (64-bit) | `sdl:opengl` | Speicher stabil | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0685 | freebsd-x86_64 (64-bit) | `sdl:vulkan` | Fenster öffnet | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0686 | freebsd-x86_64 (64-bit) | `sdl:vulkan` | Frame präsentiert | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0687 | freebsd-x86_64 (64-bit) | `sdl:vulkan` | Screenshot erzeugt | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0688 | freebsd-x86_64 (64-bit) | `sdl:vulkan` | Options zeigt Selection | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0689 | freebsd-x86_64 (64-bit) | `sdl:vulkan` | Selection-Wechsel startet neu | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0690 | freebsd-x86_64 (64-bit) | `sdl:vulkan` | Kein Crash im 60s-Loop | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0691 | freebsd-x86_64 (64-bit) | `sdl:vulkan` | Exit-Code korrekt | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0692 | freebsd-x86_64 (64-bit) | `sdl:vulkan` | Log enthält Plattform | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0693 | freebsd-x86_64 (64-bit) | `sdl:vulkan` | Assets geladen | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0694 | freebsd-x86_64 (64-bit) | `sdl:vulkan` | UI-Text sichtbar | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0695 | freebsd-x86_64 (64-bit) | `sdl:vulkan` | Maus/ESC reagieren | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0696 | freebsd-x86_64 (64-bit) | `sdl:vulkan` | Speicher stabil | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0697 | freebsd-x86_64 (64-bit) | `sdl:directx` | Fenster öffnet | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0698 | freebsd-x86_64 (64-bit) | `sdl:directx` | Frame präsentiert | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0699 | freebsd-x86_64 (64-bit) | `sdl:directx` | Screenshot erzeugt | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0700 | freebsd-x86_64 (64-bit) | `sdl:directx` | Options zeigt Selection | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0701 | freebsd-x86_64 (64-bit) | `sdl:directx` | Selection-Wechsel startet neu | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0702 | freebsd-x86_64 (64-bit) | `sdl:directx` | Kein Crash im 60s-Loop | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0703 | freebsd-x86_64 (64-bit) | `sdl:directx` | Exit-Code korrekt | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0704 | freebsd-x86_64 (64-bit) | `sdl:directx` | Log enthält Plattform | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0705 | freebsd-x86_64 (64-bit) | `sdl:directx` | Assets geladen | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0706 | freebsd-x86_64 (64-bit) | `sdl:directx` | UI-Text sichtbar | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0707 | freebsd-x86_64 (64-bit) | `sdl:directx` | Maus/ESC reagieren | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0708 | freebsd-x86_64 (64-bit) | `sdl:directx` | Speicher stabil | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0709 | freebsd-x86_64 (64-bit) | `sdl:auto` | Fenster öffnet | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0710 | freebsd-x86_64 (64-bit) | `sdl:auto` | Frame präsentiert | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0711 | freebsd-x86_64 (64-bit) | `sdl:auto` | Screenshot erzeugt | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0712 | freebsd-x86_64 (64-bit) | `sdl:auto` | Options zeigt Selection | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0713 | freebsd-x86_64 (64-bit) | `sdl:auto` | Selection-Wechsel startet neu | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0714 | freebsd-x86_64 (64-bit) | `sdl:auto` | Kein Crash im 60s-Loop | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0715 | freebsd-x86_64 (64-bit) | `sdl:auto` | Exit-Code korrekt | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0716 | freebsd-x86_64 (64-bit) | `sdl:auto` | Log enthält Plattform | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0717 | freebsd-x86_64 (64-bit) | `sdl:auto` | Assets geladen | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0718 | freebsd-x86_64 (64-bit) | `sdl:auto` | UI-Text sichtbar | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0719 | freebsd-x86_64 (64-bit) | `sdl:auto` | Maus/ESC reagieren | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0720 | freebsd-x86_64 (64-bit) | `sdl:auto` | Speicher stabil | Pakete: tar.zst | Ergebnis: PASS/FAIL dokumentiert.
AC-0721 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0722 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0723 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0724 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0725 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0726 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0727 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0728 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0729 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0730 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0731 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0732 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0733 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0734 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0735 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0736 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0737 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0738 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0739 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0740 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0741 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0742 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0743 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0744 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0745 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0746 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0747 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0748 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0749 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0750 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0751 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0752 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0753 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0754 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0755 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0756 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0757 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0758 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0759 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0760 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0761 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0762 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0763 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0764 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0765 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0766 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0767 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0768 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0769 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0770 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0771 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0772 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0773 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0774 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0775 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0776 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0777 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0778 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0779 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0780 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0781 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0782 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0783 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0784 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0785 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0786 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0787 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0788 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0789 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0790 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0791 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0792 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0793 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0794 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0795 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0796 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0797 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0798 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0799 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0800 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0801 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0802 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0803 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0804 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0805 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0806 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0807 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0808 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0809 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0810 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0811 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0812 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0813 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0814 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0815 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0816 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0817 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0818 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0819 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0820 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0821 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0822 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0823 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0824 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0825 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0826 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0827 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0828 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0829 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0830 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0831 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0832 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0833 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0834 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0835 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0836 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0837 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0838 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0839 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0840 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0841 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0842 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0843 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0844 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0845 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0846 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0847 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0848 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0849 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0850 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0851 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0852 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0853 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0854 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0855 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0856 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0857 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0858 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0859 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0860 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0861 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0862 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0863 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0864 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0865 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0866 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0867 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0868 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0869 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0870 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0871 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0872 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0873 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0874 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0875 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0876 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0877 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0878 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0879 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0880 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0881 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0882 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0883 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0884 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0885 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0886 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0887 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0888 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0889 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0890 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0891 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0892 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0893 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0894 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0895 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0896 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0897 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0898 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0899 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0900 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0901 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0902 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0903 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0904 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0905 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0906 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0907 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0908 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0909 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0910 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0911 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0912 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0913 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0914 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0915 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0916 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0917 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0918 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0919 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0920 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0921 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0922 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0923 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0924 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0925 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0926 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0927 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0928 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0929 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0930 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0931 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0932 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0933 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0934 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0935 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0936 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0937 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0938 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0939 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0940 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0941 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0942 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0943 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0944 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0945 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0946 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0947 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0948 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0949 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0950 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0951 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0952 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0953 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0954 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0955 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0956 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0957 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0958 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0959 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0960 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0961 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0962 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0963 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0964 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0965 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0966 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0967 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0968 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0969 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0970 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0971 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0972 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0973 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0974 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0975 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0976 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0977 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0978 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0979 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0980 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0981 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0982 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0983 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0984 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0985 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0986 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0987 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0988 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0989 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0990 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0991 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0992 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0993 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0994 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0995 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0996 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0997 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0998 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-0999 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1000 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1001 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1002 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1003 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1004 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1005 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1006 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1007 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1008 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1009 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1010 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1011 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1012 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1013 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1014 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1015 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1016 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1017 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1018 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1019 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1020 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1021 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1022 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1023 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1024 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1025 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1026 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1027 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1028 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1029 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1030 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1031 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1032 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1033 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1034 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1035 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1036 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1037 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1038 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1039 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1040 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1041 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1042 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1043 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1044 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1045 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1046 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1047 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1048 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1049 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1050 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1051 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1052 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1053 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1054 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1055 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1056 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1057 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1058 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1059 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1060 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1061 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1062 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1063 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1064 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1065 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1066 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1067 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1068 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1069 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1070 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1071 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1072 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1073 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1074 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1075 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1076 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1077 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1078 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1079 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1080 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1081 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1082 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1083 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1084 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1085 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1086 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1087 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1088 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1089 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1090 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1091 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1092 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1093 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1094 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1095 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1096 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1097 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1098 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1099 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1100 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1101 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1102 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1103 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1104 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1105 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1106 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1107 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1108 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1109 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1110 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1111 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1112 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1113 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1114 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1115 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1116 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1117 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1118 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1119 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1120 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1121 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1122 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1123 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1124 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1125 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1126 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1127 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1128 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1129 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1130 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1131 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1132 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1133 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1134 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1135 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1136 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1137 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1138 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1139 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1140 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1141 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1142 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1143 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1144 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1145 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1146 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1147 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1148 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1149 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1150 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1151 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1152 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1153 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1154 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1155 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1156 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1157 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1158 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1159 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1160 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1161 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1162 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1163 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1164 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1165 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1166 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1167 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1168 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1169 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1170 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1171 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1172 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1173 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1174 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1175 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1176 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1177 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1178 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1179 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1180 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1181 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1182 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1183 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1184 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1185 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1186 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1187 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1188 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1189 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1190 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1191 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1192 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1193 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1194 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1195 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1196 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1197 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1198 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1199 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1200 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1201 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1202 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1203 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1204 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1205 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1206 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1207 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1208 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1209 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1210 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1211 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1212 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1213 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1214 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1215 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1216 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1217 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1218 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1219 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1220 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1221 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1222 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1223 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1224 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1225 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1226 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1227 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1228 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1229 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1230 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1231 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1232 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1233 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1234 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1235 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1236 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1237 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1238 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1239 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1240 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1241 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1242 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1243 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1244 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1245 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1246 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1247 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1248 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1249 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1250 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1251 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1252 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1253 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1254 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1255 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1256 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1257 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1258 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1259 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1260 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1261 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1262 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1263 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1264 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1265 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1266 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1267 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1268 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1269 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1270 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1271 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1272 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1273 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1274 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1275 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1276 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1277 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1278 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1279 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1280 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1281 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1282 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1283 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1284 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1285 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1286 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1287 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1288 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1289 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1290 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1291 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1292 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1293 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1294 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1295 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1296 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1297 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1298 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1299 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1300 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1301 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1302 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1303 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1304 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1305 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1306 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1307 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1308 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1309 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1310 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1311 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1312 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1313 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1314 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1315 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1316 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1317 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1318 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1319 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1320 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1321 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1322 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1323 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1324 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1325 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1326 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1327 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1328 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1329 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1330 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1331 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1332 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1333 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1334 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1335 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1336 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1337 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1338 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1339 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1340 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1341 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1342 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1343 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1344 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1345 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1346 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1347 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1348 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1349 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1350 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1351 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1352 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1353 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1354 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1355 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1356 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1357 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1358 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1359 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1360 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1361 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1362 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1363 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1364 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1365 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1366 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1367 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1368 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1369 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1370 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1371 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1372 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1373 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1374 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1375 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1376 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1377 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1378 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1379 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1380 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1381 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1382 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1383 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1384 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1385 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1386 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1387 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1388 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1389 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1390 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1391 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1392 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1393 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1394 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1395 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1396 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1397 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1398 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1399 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1400 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1401 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1402 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1403 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1404 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1405 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1406 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1407 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1408 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1409 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1410 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1411 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1412 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1413 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1414 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1415 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1416 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1417 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1418 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1419 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1420 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1421 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1422 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1423 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1424 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1425 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1426 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1427 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1428 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1429 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1430 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1431 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1432 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1433 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1434 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1435 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1436 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1437 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1438 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1439 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1440 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1441 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1442 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1443 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1444 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1445 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1446 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1447 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1448 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1449 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1450 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1451 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1452 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1453 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1454 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1455 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1456 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1457 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1458 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1459 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1460 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1461 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1462 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1463 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1464 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1465 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1466 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1467 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1468 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1469 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1470 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1471 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1472 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1473 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1474 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1475 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1476 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1477 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1478 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1479 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1480 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1481 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1482 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1483 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1484 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1485 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1486 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1487 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1488 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1489 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1490 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1491 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1492 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1493 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1494 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1495 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1496 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1497 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1498 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1499 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1500 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1501 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1502 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1503 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1504 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1505 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1506 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1507 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1508 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1509 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1510 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1511 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1512 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1513 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1514 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1515 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1516 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1517 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1518 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1519 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1520 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1521 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1522 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1523 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1524 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1525 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1526 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1527 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1528 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1529 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1530 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1531 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1532 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1533 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1534 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1535 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1536 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1537 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1538 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1539 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1540 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1541 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1542 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1543 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1544 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1545 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1546 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1547 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1548 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1549 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1550 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1551 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1552 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1553 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1554 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1555 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1556 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1557 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1558 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1559 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1560 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1561 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1562 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1563 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1564 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1565 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1566 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1567 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1568 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1569 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1570 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1571 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1572 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1573 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1574 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1575 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1576 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1577 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1578 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1579 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1580 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1581 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1582 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1583 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1584 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1585 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1586 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1587 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1588 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1589 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1590 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1591 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1592 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1593 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1594 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1595 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1596 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1597 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1598 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1599 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1600 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1601 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1602 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1603 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1604 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1605 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1606 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1607 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1608 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1609 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1610 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1611 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1612 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1613 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1614 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1615 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1616 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1617 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1618 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1619 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1620 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1621 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1622 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1623 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1624 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1625 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1626 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1627 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1628 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1629 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1630 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1631 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1632 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1633 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1634 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1635 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1636 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1637 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1638 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1639 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1640 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1641 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1642 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1643 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1644 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1645 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1646 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1647 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1648 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1649 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1650 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1651 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1652 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1653 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1654 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1655 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1656 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1657 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1658 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1659 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1660 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1661 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1662 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1663 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1664 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1665 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1666 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1667 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1668 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1669 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1670 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1671 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1672 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1673 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1674 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1675 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1676 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1677 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1678 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1679 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1680 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1681 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1682 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1683 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1684 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1685 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1686 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1687 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1688 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1689 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1690 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1691 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1692 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1693 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1694 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1695 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1696 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1697 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1698 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1699 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1700 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1701 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1702 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1703 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1704 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1705 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1706 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1707 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1708 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1709 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1710 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1711 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1712 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1713 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1714 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1715 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1716 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1717 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1718 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1719 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1720 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1721 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1722 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1723 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1724 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1725 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1726 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1727 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1728 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1729 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1730 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1731 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1732 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1733 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1734 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1735 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1736 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1737 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1738 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1739 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1740 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1741 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1742 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1743 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1744 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1745 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1746 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1747 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1748 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1749 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1750 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1751 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1752 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1753 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1754 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1755 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1756 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1757 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1758 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1759 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1760 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1761 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1762 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1763 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1764 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1765 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1766 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1767 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1768 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1769 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1770 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1771 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1772 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1773 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1774 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1775 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1776 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1777 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1778 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1779 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1780 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1781 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1782 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1783 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1784 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1785 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1786 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1787 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1788 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1789 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1790 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1791 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1792 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1793 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1794 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1795 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1796 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1797 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1798 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1799 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1800 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1801 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1802 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1803 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1804 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1805 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1806 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1807 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1808 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1809 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1810 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1811 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1812 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1813 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1814 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1815 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1816 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1817 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1818 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1819 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1820 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1821 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1822 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1823 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1824 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1825 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1826 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1827 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1828 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1829 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1830 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1831 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1832 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1833 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1834 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1835 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1836 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1837 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1838 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1839 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1840 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1841 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1842 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1843 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1844 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1845 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1846 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1847 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1848 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1849 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1850 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1851 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1852 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1853 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1854 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1855 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1856 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1857 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1858 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1859 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1860 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1861 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1862 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1863 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1864 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1865 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1866 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1867 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1868 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1869 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1870 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1871 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1872 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1873 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1874 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1875 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1876 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1877 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1878 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1879 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1880 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1881 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1882 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1883 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1884 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1885 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1886 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1887 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1888 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1889 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1890 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1891 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1892 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1893 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1894 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1895 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1896 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1897 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1898 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1899 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1900 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1901 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1902 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1903 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1904 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1905 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1906 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1907 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1908 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1909 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1910 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1911 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1912 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1913 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1914 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1915 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1916 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1917 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1918 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1919 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1920 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1921 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1922 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1923 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1924 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1925 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1926 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1927 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1928 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1929 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1930 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1931 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1932 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1933 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1934 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1935 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1936 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1937 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1938 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1939 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1940 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1941 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1942 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1943 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1944 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1945 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1946 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1947 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1948 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1949 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1950 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1951 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1952 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1953 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1954 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1955 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1956 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1957 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1958 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1959 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1960 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1961 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1962 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1963 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1964 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1965 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1966 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1967 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1968 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1969 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1970 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1971 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1972 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1973 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1974 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1975 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1976 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1977 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1978 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1979 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1980 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1981 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1982 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1983 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1984 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1985 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1986 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1987 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1988 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1989 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1990 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1991 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1992 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1993 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1994 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1995 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1996 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1997 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1998 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-1999 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2000 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2001 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2002 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2003 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2004 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2005 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2006 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2007 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2008 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2009 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2010 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2011 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2012 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2013 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2014 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2015 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2016 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2017 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2018 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2019 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2020 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2021 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2022 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2023 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2024 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2025 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2026 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2027 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2028 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2029 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2030 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2031 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2032 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2033 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2034 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2035 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2036 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2037 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2038 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2039 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2040 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2041 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2042 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2043 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2044 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2045 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2046 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2047 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2048 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2049 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2050 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2051 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2052 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2053 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2054 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2055 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2056 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2057 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2058 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2059 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2060 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2061 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2062 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2063 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2064 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2065 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2066 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2067 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2068 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2069 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2070 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2071 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2072 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2073 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2074 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2075 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2076 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2077 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2078 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2079 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2080 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2081 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2082 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2083 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2084 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2085 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2086 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2087 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2088 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2089 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2090 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2091 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2092 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2093 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2094 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2095 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2096 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2097 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2098 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2099 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2100 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2101 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2102 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2103 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2104 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2105 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2106 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2107 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2108 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2109 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2110 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2111 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2112 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2113 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2114 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2115 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2116 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2117 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2118 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2119 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2120 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2121 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2122 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2123 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2124 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2125 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2126 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2127 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2128 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2129 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2130 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2131 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2132 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2133 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2134 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2135 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2136 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2137 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2138 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2139 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2140 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2141 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2142 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2143 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2144 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2145 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2146 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2147 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2148 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2149 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2150 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2151 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2152 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2153 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2154 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2155 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2156 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2157 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2158 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2159 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2160 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2161 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2162 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2163 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2164 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2165 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2166 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2167 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2168 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2169 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2170 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2171 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2172 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2173 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2174 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2175 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2176 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2177 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2178 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2179 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2180 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2181 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2182 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2183 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2184 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2185 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2186 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2187 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2188 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2189 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2190 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2191 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2192 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2193 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2194 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2195 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2196 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2197 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2198 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2199 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2200 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2201 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2202 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2203 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2204 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2205 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2206 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2207 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2208 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2209 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2210 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2211 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2212 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2213 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2214 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2215 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2216 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2217 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2218 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2219 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2220 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2221 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2222 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2223 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2224 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2225 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2226 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2227 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2228 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2229 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2230 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2231 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2232 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2233 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2234 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2235 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2236 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2237 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2238 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2239 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2240 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2241 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2242 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2243 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2244 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2245 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2246 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2247 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2248 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2249 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2250 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2251 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2252 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2253 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2254 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2255 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2256 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2257 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2258 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2259 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2260 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2261 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2262 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2263 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2264 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2265 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2266 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2267 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2268 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2269 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2270 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2271 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2272 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2273 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2274 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2275 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2276 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2277 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2278 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2279 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2280 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2281 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2282 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2283 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2284 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2285 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2286 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2287 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2288 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2289 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2290 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2291 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2292 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2293 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2294 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2295 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2296 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2297 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2298 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2299 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2300 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2301 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2302 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2303 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2304 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2305 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2306 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2307 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2308 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2309 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2310 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2311 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2312 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2313 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2314 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2315 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2316 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2317 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2318 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2319 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2320 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2321 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2322 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2323 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2324 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2325 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2326 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2327 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2328 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2329 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2330 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2331 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2332 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2333 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2334 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2335 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2336 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2337 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2338 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2339 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2340 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2341 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2342 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2343 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2344 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2345 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2346 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2347 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2348 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2349 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2350 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2351 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2352 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2353 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2354 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2355 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2356 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2357 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2358 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2359 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2360 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2361 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2362 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2363 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2364 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2365 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2366 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2367 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2368 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2369 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2370 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2371 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2372 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2373 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2374 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2375 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2376 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2377 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2378 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2379 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2380 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2381 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2382 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2383 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2384 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2385 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2386 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2387 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2388 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2389 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2390 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2391 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2392 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2393 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2394 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2395 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2396 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2397 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2398 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2399 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2400 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2401 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2402 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2403 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2404 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2405 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2406 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2407 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2408 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2409 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2410 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2411 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2412 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2413 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2414 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2415 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2416 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2417 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2418 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2419 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2420 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2421 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2422 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2423 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2424 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2425 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2426 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2427 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2428 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2429 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2430 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2431 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2432 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2433 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2434 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2435 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2436 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2437 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2438 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2439 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2440 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2441 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2442 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2443 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2444 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2445 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2446 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2447 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2448 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2449 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2450 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2451 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2452 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2453 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2454 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2455 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2456 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2457 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2458 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2459 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2460 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2461 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2462 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2463 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2464 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2465 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2466 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2467 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2468 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2469 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2470 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2471 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2472 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2473 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2474 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2475 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2476 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2477 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2478 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2479 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2480 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2481 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2482 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2483 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2484 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2485 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2486 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2487 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2488 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2489 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2490 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2491 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2492 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2493 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2494 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2495 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2496 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2497 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2498 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2499 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2500 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2501 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2502 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2503 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2504 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2505 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2506 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2507 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2508 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2509 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2510 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2511 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2512 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2513 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2514 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2515 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2516 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2517 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2518 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2519 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2520 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2521 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2522 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2523 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2524 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2525 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2526 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2527 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2528 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2529 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2530 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2531 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2532 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2533 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2534 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2535 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2536 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2537 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2538 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2539 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2540 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2541 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2542 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2543 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2544 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2545 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2546 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2547 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2548 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2549 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2550 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2551 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2552 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2553 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2554 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2555 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2556 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2557 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2558 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2559 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2560 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2561 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2562 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2563 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2564 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2565 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2566 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2567 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2568 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2569 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2570 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2571 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2572 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2573 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2574 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2575 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2576 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2577 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2578 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2579 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2580 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2581 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2582 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2583 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2584 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2585 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2586 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2587 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2588 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2589 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2590 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2591 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2592 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2593 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2594 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2595 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2596 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2597 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2598 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2599 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2600 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2601 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2602 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2603 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2604 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2605 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2606 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2607 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2608 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2609 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2610 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2611 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2612 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2613 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2614 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2615 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2616 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2617 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2618 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2619 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2620 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2621 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2622 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2623 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2624 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2625 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2626 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2627 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2628 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2629 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2630 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2631 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2632 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2633 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2634 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2635 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2636 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2637 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2638 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2639 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2640 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2641 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2642 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2643 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2644 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2645 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2646 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2647 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2648 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2649 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2650 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2651 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2652 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2653 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2654 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2655 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2656 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2657 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2658 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2659 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2660 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2661 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2662 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2663 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2664 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2665 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2666 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2667 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2668 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2669 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2670 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2671 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2672 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2673 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2674 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2675 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2676 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2677 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2678 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2679 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2680 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2681 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2682 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2683 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2684 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2685 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2686 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2687 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2688 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2689 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2690 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2691 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2692 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2693 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2694 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2695 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2696 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2697 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2698 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2699 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2700 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2701 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2702 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2703 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2704 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2705 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2706 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2707 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2708 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2709 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2710 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2711 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2712 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2713 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2714 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2715 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2716 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2717 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2718 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2719 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2720 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2721 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2722 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2723 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2724 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2725 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2726 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2727 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2728 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2729 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2730 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2731 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2732 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2733 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2734 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2735 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2736 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2737 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2738 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2739 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2740 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2741 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2742 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2743 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2744 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2745 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2746 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2747 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2748 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2749 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2750 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2751 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2752 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2753 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2754 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2755 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2756 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2757 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2758 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2759 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2760 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2761 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2762 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2763 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2764 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2765 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2766 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2767 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2768 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2769 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2770 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2771 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2772 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2773 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2774 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2775 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2776 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2777 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2778 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2779 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2780 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2781 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2782 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2783 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2784 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2785 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2786 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2787 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2788 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2789 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2790 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2791 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2792 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2793 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2794 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2795 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2796 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2797 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2798 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2799 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2800 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2801 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2802 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2803 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2804 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2805 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2806 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2807 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2808 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2809 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2810 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2811 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2812 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2813 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2814 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2815 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2816 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2817 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2818 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2819 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2820 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2821 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2822 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2823 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2824 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2825 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2826 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2827 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2828 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2829 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2830 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2831 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2832 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2833 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2834 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2835 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2836 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2837 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2838 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2839 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2840 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2841 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2842 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2843 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2844 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2845 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2846 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2847 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2848 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2849 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2850 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2851 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2852 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2853 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2854 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2855 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2856 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2857 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2858 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2859 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2860 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2861 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2862 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2863 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2864 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2865 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2866 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2867 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2868 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2869 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2870 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2871 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2872 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2873 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2874 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2875 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2876 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2877 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2878 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2879 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2880 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2881 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2882 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2883 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2884 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2885 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2886 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2887 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2888 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2889 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2890 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2891 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2892 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2893 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2894 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2895 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2896 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2897 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2898 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2899 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2900 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2901 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2902 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2903 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2904 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2905 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2906 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2907 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2908 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2909 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2910 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2911 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2912 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2913 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2914 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2915 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2916 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2917 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2918 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2919 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2920 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2921 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2922 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2923 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2924 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2925 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2926 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2927 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2928 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2929 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2930 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2931 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2932 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2933 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2934 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2935 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2936 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2937 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2938 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2939 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2940 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2941 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2942 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2943 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2944 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2945 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2946 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2947 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2948 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2949 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2950 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2951 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2952 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2953 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2954 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2955 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2956 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2957 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2958 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2959 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2960 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2961 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2962 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2963 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2964 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2965 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2966 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2967 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2968 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2969 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2970 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2971 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2972 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2973 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2974 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2975 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2976 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2977 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2978 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2979 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2980 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2981 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2982 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2983 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2984 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2985 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2986 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2987 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2988 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2989 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2990 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2991 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2992 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2993 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2994 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2995 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2996 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2997 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2998 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-2999 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3000 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3001 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3002 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3003 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3004 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3005 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3006 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3007 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3008 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3009 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3010 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3011 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3012 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3013 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3014 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3015 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3016 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3017 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3018 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3019 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3020 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3021 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3022 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3023 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3024 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3025 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3026 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3027 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3028 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3029 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3030 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3031 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3032 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3033 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3034 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3035 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3036 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3037 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3038 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3039 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3040 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3041 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3042 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3043 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3044 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3045 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3046 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3047 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3048 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3049 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3050 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3051 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3052 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3053 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3054 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3055 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3056 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3057 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3058 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3059 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3060 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3061 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3062 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3063 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3064 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3065 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3066 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3067 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3068 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3069 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3070 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3071 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3072 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3073 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3074 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3075 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3076 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3077 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3078 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3079 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3080 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3081 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3082 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3083 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3084 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3085 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3086 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3087 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3088 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3089 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3090 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3091 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3092 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3093 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3094 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3095 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3096 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3097 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3098 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3099 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3100 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3101 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3102 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3103 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3104 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3105 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3106 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3107 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3108 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3109 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3110 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3111 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3112 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3113 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3114 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3115 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3116 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3117 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3118 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3119 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3120 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3121 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3122 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3123 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3124 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3125 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3126 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3127 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3128 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3129 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3130 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3131 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3132 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3133 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3134 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3135 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3136 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3137 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3138 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3139 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3140 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3141 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3142 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3143 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3144 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3145 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3146 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3147 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3148 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3149 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3150 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3151 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3152 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3153 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3154 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3155 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3156 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3157 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3158 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3159 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3160 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3161 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3162 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3163 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3164 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3165 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3166 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3167 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3168 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3169 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3170 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3171 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3172 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3173 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3174 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3175 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3176 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3177 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3178 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3179 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3180 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3181 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3182 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3183 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3184 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3185 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3186 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3187 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3188 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3189 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3190 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3191 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3192 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3193 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3194 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3195 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3196 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3197 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3198 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3199 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3200 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3201 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3202 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3203 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3204 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3205 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3206 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3207 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3208 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3209 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3210 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3211 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3212 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3213 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3214 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3215 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3216 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3217 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3218 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3219 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3220 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3221 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3222 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3223 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3224 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3225 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3226 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3227 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3228 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3229 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3230 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3231 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3232 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3233 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3234 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3235 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3236 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3237 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3238 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3239 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3240 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3241 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3242 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3243 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3244 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3245 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3246 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3247 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3248 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3249 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3250 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3251 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3252 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3253 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3254 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3255 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3256 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3257 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3258 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3259 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3260 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3261 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3262 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3263 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3264 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3265 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3266 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3267 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3268 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3269 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3270 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3271 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3272 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3273 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3274 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3275 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3276 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3277 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3278 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3279 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3280 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3281 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3282 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3283 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3284 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3285 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3286 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3287 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3288 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3289 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3290 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3291 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3292 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3293 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3294 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3295 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3296 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3297 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3298 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3299 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3300 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3301 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3302 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3303 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3304 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3305 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3306 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3307 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3308 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3309 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3310 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3311 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3312 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3313 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3314 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3315 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3316 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3317 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3318 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3319 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3320 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3321 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3322 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3323 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3324 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3325 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3326 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3327 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3328 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3329 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3330 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3331 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3332 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3333 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3334 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3335 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3336 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3337 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3338 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3339 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3340 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3341 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3342 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3343 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3344 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3345 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3346 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3347 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3348 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3349 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3350 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3351 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3352 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3353 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3354 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3355 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3356 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3357 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3358 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3359 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3360 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3361 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3362 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3363 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3364 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3365 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3366 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3367 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3368 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3369 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3370 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3371 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3372 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3373 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3374 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3375 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3376 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3377 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3378 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3379 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3380 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3381 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3382 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3383 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3384 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3385 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3386 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3387 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3388 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3389 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3390 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3391 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3392 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3393 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3394 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3395 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3396 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3397 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3398 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3399 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3400 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3401 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3402 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3403 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3404 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3405 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3406 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3407 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3408 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3409 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3410 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3411 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3412 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3413 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3414 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3415 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3416 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3417 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3418 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3419 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3420 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3421 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3422 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3423 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3424 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3425 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3426 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3427 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3428 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3429 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3430 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3431 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3432 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3433 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3434 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3435 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3436 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3437 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3438 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3439 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3440 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3441 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3442 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3443 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3444 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3445 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3446 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3447 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3448 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3449 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3450 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3451 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3452 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3453 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3454 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3455 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3456 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3457 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3458 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3459 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3460 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3461 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3462 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3463 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3464 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3465 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3466 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3467 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3468 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3469 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3470 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3471 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3472 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3473 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3474 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3475 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3476 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3477 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3478 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3479 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3480 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3481 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3482 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3483 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3484 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3485 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3486 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3487 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3488 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3489 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3490 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3491 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3492 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3493 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3494 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3495 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3496 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3497 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3498 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3499 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3500 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3501 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3502 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3503 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3504 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3505 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-3506 | Release | Artefakt, Checksumme, SBOM und Signatur vorhanden | Ergebnis: PASS/FAIL dokumentiert.
AC-4999 | Nightly | Build, Test, Artefakt, Log | Ergebnis: PASS/FAIL.
