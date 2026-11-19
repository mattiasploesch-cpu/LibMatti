# MattiLauncher.cmake - builds the Matticraft launcher (Rust + egui).
#
# The launcher is not part of libmatti: it is a separate Rust workspace at
# launchers/ with one core crate and thin frontends. CMake only orchestrates
# cargo and the install rules, so a release build produces the game and the
# launcher from the same tree and the same commit.
#
#   matti_add_launcher()   adds the `launcher` target and install rules
#
# The target is skipped for cross builds (the Rust target and its system
# libraries have to exist on the host) and can be disabled with
# -DMATTI_BUILD_LAUNCHER=OFF when only the game library is needed.

include_guard(GLOBAL)

if(CMAKE_CROSSCOMPILING)
    set(_matti_launcher_default OFF)
else()
    set(_matti_launcher_default ON)
endif()

option(MATTI_BUILD_LAUNCHER
        "Build the Matticraft launcher (Rust/egui frontend)" ${_matti_launcher_default})
set(MATTI_UPDATE_PUBLIC_KEY "" CACHE STRING
        "Ed25519 public key (hex) compiled into the launcher for update manifests")

function(matti_add_launcher)
    if(NOT MATTI_BUILD_LAUNCHER)
        message(STATUS "Matticraft launcher disabled (MATTI_BUILD_LAUNCHER=OFF)")
        return()
    endif()

    set(_manifest "${CMAKE_SOURCE_DIR}/launchers/Cargo.toml")
    if(NOT EXISTS "${_manifest}")
        message(WARNING "launchers/Cargo.toml is missing - launcher target skipped")
        return()
    endif()

    if(NOT MATTI_CARGO)
        message(WARNING "cargo not found - the Matticraft launcher is not built")
        return()
    endif()

    if(CMAKE_BUILD_TYPE MATCHES "^(Release|RelWithDebInfo|MinSizeRel)$")
        set(_profile "release")
        set(_profile_dir "release")
    else()
        set(_profile "dev")
        set(_profile_dir "debug")
    endif()

    if(APPLE)
        set(_package "launcher-macos")
        set(_bin_name "matticraft-launcher-macos")
    else()
        set(_package "launcher-desktop")
        set(_bin_name "matticraft-launcher")
    endif()

    # Keep cargo's default target directory (launchers/target) so a dev tree
    # launcher finds the game binary through launch::local_candidates().
    set(_target_dir "${CMAKE_SOURCE_DIR}/launchers/target")
    set(_binary "${_target_dir}/${_profile_dir}/${_bin_name}${CMAKE_EXECUTABLE_SUFFIX}")

    file(GLOB_RECURSE _sources CONFIGURE_DEPENDS
            "${CMAKE_SOURCE_DIR}/launchers/*.rs"
            "${CMAKE_SOURCE_DIR}/launchers/*.toml")
    if(EXISTS "${CMAKE_SOURCE_DIR}/launchers/Cargo.lock")
        list(APPEND _sources "${CMAKE_SOURCE_DIR}/launchers/Cargo.lock")
    endif()

    set(_env "CARGO_TARGET_DIR=${_target_dir}")
    if(MATTI_UPDATE_PUBLIC_KEY)
        list(APPEND _env "MATTI_UPDATE_PUBLIC_KEY=${MATTI_UPDATE_PUBLIC_KEY}")
    endif()

    add_custom_command(
            OUTPUT "${_binary}"
            COMMAND ${CMAKE_COMMAND} -E env ${_env}
                    "${MATTI_CARGO}" build
                    --manifest-path "${_manifest}"
                    --package "${_package}"
                    --bin "${_bin_name}"
                    --profile "${_profile}"
            DEPENDS ${_sources}
            COMMENT "Building Matticraft launcher ${_bin_name} (${_profile})"
            USES_TERMINAL
            VERBATIM)

    add_custom_target(launcher ALL DEPENDS "${_binary}")

    install(PROGRAMS "${_binary}"
            DESTINATION ${CMAKE_INSTALL_BINDIR}
            RENAME "matticraft-launcher${CMAKE_EXECUTABLE_SUFFIX}")

    if(UNIX AND NOT APPLE)
        install(FILES packaging/linux/matticraft.desktop
                DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
                RENAME org.matticraft.Matticraft.desktop)
        install(FILES packaging/linux/matticraft.svg
                DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
                RENAME org.matticraft.Matticraft.svg)
        install(FILES packaging/linux/matticraft.png
                DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps
                RENAME org.matticraft.Matticraft.png)
        install(FILES packaging/linux/org.matticraft.Matticraft.appdata.xml
                DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
    endif()

    message(STATUS "Matticraft launcher: ${_binary}")
endfunction()
