cmake_minimum_required(VERSION 3.20)

# Platform/toolchain detection must run before project(): on NixOS there is no
# /usr/bin/gcc, the compiler lives in /run/current-system/sw/bin.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(MattiPlatform)

project(Matticraft C)

option(MATTI_MOBILE_ONLY "Build only libmatti for the Android JNI library" OFF)

if(WIN32 AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    include_directories(BEFORE "${CMAKE_SOURCE_DIR}/cmake/compat/mingw")
    add_compile_options("SHELL:-include ${CMAKE_SOURCE_DIR}/cmake/compat/mingw/matti_compat.h")
    link_libraries(ws2_32)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
# gnu11: the port uses POSIX/GNU interfaces such as strndup, getline and dlinfo.
set(CMAKE_C_EXTENSIONS ON)

# Autoconf-style checks (headers, functions, libraries, types) + config.h.
include(MattiConfigureChecks)

# Native libraries the client dlopen()s (libglfw) are embedded into the
# executables, so no library search path is needed at runtime.
include(MattiNative)

# The launcher is a separate Rust workspace; CMake builds it with cargo and
# installs it next to the game (MATTI_BUILD_LAUNCHER=OFF skips it).
include(MattiLauncher)

find_package(Threads REQUIRED)

enable_testing()

add_subdirectory(library)

if(NOT MATTI_MOBILE_ONLY)
add_subdirectory(client)
add_subdirectory(vulkan_client)

# The test fixtures and mods use ELF sections (objcopy/ELF section injection),
# which Mach-O cannot represent. The macOS release still builds the game and
# launcher; only the ELF-only development targets are omitted there.
if(NOT APPLE)
    add_subdirectory(tests)

    # The mods are a separate project: they only include the library headers, the
    # running game provides the symbols. This target configures and builds the mods
    # project (cmake -S mods -B mods/build && cmake --build mods/build); the built
    # .so files land in mods/, the folder --gameDir <repo root> scans.
    add_custom_target(build_mods
            COMMAND ${CMAKE_COMMAND} -S "${CMAKE_CURRENT_SOURCE_DIR}/mods" -B "${CMAKE_CURRENT_SOURCE_DIR}/mods/build"
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_CURRENT_SOURCE_DIR}/mods/build"
            COMMENT "Building the mods project (output: mods/*.so)"
            VERBATIM
    )

    # One target for everything: building "matticraft" builds the library, the
    # game, every test harness with its fixture mods and the mods project. Running
    # it executes the selftest phase (every harness) and then the launcher.
    add_dependencies(matticraft build_mods
            test_modscan test_soscan test_mixinfacade test_m4proof)
endif()

# The plain client run (run configuration "runClient"): the same binary as
# matticraft but without the test harnesses as dependencies - the selftest
# phase is skipped via --no-tests, so no harness binary is needed.
add_executable(runClient)

target_link_libraries(runClient PRIVATE
        $<LINK_LIBRARY:WHOLE_ARCHIVE,matti> matti_rust
)
target_sources(runClient PRIVATE
        client/src/main.c
        client/src/selftest.c
        client/src/demo-service.c
)
target_include_directories(runClient PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/client/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/library/src"
)
target_compile_definitions(runClient PRIVATE
        MATTI_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
        MATTI_BUILD_DIR="${CMAKE_BINARY_DIR}"
        # the selftest phase probes for the harness binaries; runClient ships without them
        MATTICRAFT_CLIENT_ONLY=1
)
set_target_properties(runClient PROPERTIES
        ENABLE_EXPORTS ON
)
matti_embed_native_bundle(runClient)
matti_embed_ui_assets(runClient)
matti_embed_rust_artifacts(runClient matti_rust ".matti_rust")

add_executable(vulkanClient)

target_link_libraries(vulkanClient PRIVATE
        $<LINK_LIBRARY:WHOLE_ARCHIVE,matti> matti_rust
)
target_sources(vulkanClient PRIVATE
        vulkan_client/src/main.c
        vulkan_client/src/demo-service.c
        vulkan_client/src/selftest.c
)
target_include_directories(vulkanClient PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/client/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/library/src"
)
target_compile_definitions(vulkanClient PRIVATE
        MATTI_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
        MATTI_BUILD_DIR="${CMAKE_BINARY_DIR}"
        # the selftest phase probes for the harness binaries; runClient ships without them
        MATTICRAFT_CLIENT_ONLY=1
)
set_target_properties(vulkanClient PROPERTIES
        ENABLE_EXPORTS ON
)
matti_embed_native_bundle(vulkanClient)
matti_embed_ui_assets(vulkanClient)
matti_embed_rust_artifacts(vulkanClient matti_rust ".matti_rust")

# The demo mods come with the client. build_mods is ELF-only.
if(APPLE)
    add_dependencies(runClient vulkanClient demo_app demo_lib)
else()
    add_dependencies(runClient vulkanClient demo_app demo_lib build_mods)
endif()

# Install rules used by packaging/ (tar.zst, deb, rpm, AppImage, Flatpak, MSI,
# DMG/PKG). Assets are embedded in the binaries, so the staging tree only
# carries executables, the shared libmatti and the public headers.
include(GNUInstallDirs)
# Packages (deb/rpm/AppImage/DMG) expect lib/, not lib64/.
if(CMAKE_INSTALL_LIBDIR MATCHES "lib64")
    set(CMAKE_INSTALL_LIBDIR "lib")
endif()
# Keep the documentation path identical to the package recipes.
set(CMAKE_INSTALL_DOCDIR "share/doc/matticraft")
install(TARGETS runClient vulkanClient vulkancraft
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(TARGETS matti_shared
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY library/src/libmatti/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libmatti
        FILES_MATCHING PATTERN "*.h")
install(FILES README.md planported.MD DESTINATION ${CMAKE_INSTALL_DOCDIR})

# UI resources for the platforms without ELF section embedding (Windows uses
# PE sections, macOS ships a bundle): the launcher exports MATTI_UI_ASSETS and
# MATTI_THEME_FONT pointing at these files.
install(DIRECTORY library/src/libmatti/matti/ui/assets/
        DESTINATION ${CMAKE_INSTALL_DATADIR}/matticraft/assets)
install(FILES
        vendor/FancyModLoader/earlydisplay/src/main/resources/net/neoforged/fml/earlydisplay/theme/Monocraft.ttf
        DESTINATION ${CMAKE_INSTALL_DATADIR}/matticraft)

# Launcher (Rust/egui) and its desktop entry. The library and the game are
# installed above; the launcher is the entry point of every package.
matti_add_launcher()
endif()
