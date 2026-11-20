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
set(MATTI_LAUNCHER_LIB_DIR "" CACHE PATH
        "Optional directory with libX11/libwayland/libGL runtime libraries to ship")
option(MATTI_LAUNCHER_BUNDLE_LIBS
        "Collect X11/Wayland/GL runtime libraries next to the launcher" ON)

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

    set(_target_dir "${CMAKE_BINARY_DIR}/cargo/launchers")
    set(_binary "${_target_dir}/${_profile_dir}/${_bin_name}${CMAKE_EXECUTABLE_SUFFIX}")
    set(_staged "${CMAKE_BINARY_DIR}/bin/${_bin_name}${CMAKE_EXECUTABLE_SUFFIX}")
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

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
    if(UNIX AND NOT APPLE)
        list(APPEND _env "RUSTFLAGS=-C link-arg=-Wl,-rpath,$ORIGIN/lib -C link-arg=-Wl,-rpath,$ORIGIN/../lib")
    endif()

    add_custom_command(
            OUTPUT "${_binary}"
            COMMAND ${CMAKE_COMMAND} -E env ${_env}
                    "${MATTI_CARGO}" build
                    --manifest-path "${_manifest}"
                    --package "${_package}"
                    --bin "${_bin_name}"
                    --profile "${_profile}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/bin"
            COMMAND ${CMAKE_COMMAND} -E copy "${_binary}" "${_staged}"
            BYPRODUCTS "${_staged}"
            DEPENDS ${_sources}
            COMMENT "Building Matticraft launcher ${_bin_name} (${_profile})"
            USES_TERMINAL
            VERBATIM)

    add_custom_target(launcher ALL DEPENDS "${_binary}")

    if(MATTI_LAUNCHER_LIB_DIR)
        if(EXISTS "${MATTI_LAUNCHER_LIB_DIR}")
            file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin/lib")
            file(GLOB _runtime_libs
                    "${MATTI_LAUNCHER_LIB_DIR}/libX11.so*"
                    "${MATTI_LAUNCHER_LIB_DIR}/libX11-xcb.so*"
                    "${MATTI_LAUNCHER_LIB_DIR}/libXcursor.so*"
                    "${MATTI_LAUNCHER_LIB_DIR}/libXrandr.so*"
                    "${MATTI_LAUNCHER_LIB_DIR}/libXi.so*"
                    "${MATTI_LAUNCHER_LIB_DIR}/libXext.so*"
                    "${MATTI_LAUNCHER_LIB_DIR}/libXau.so*"
                    "${MATTI_LAUNCHER_LIB_DIR}/libXdmcp.so*"
                    "${MATTI_LAUNCHER_LIB_DIR}/libXrender.so*"
                    "${MATTI_LAUNCHER_LIB_DIR}/libXfixes.so*"
                    "${MATTI_LAUNCHER_LIB_DIR}/libxcb.so*"
                    "${MATTI_LAUNCHER_LIB_DIR}/libxcb-render.so*"
                    "${MATTI_LAUNCHER_LIB_DIR}/libxcb-shm.so*"
                    "${MATTI_LAUNCHER_LIB_DIR}/libxkbcommon.so*"
                    "${MATTI_LAUNCHER_LIB_DIR}/libwayland-client.so*"
                    "${MATTI_LAUNCHER_LIB_DIR}/libwayland-cursor.so*"
                    "${MATTI_LAUNCHER_LIB_DIR}/libwayland-egl.so*"
                    "${MATTI_LAUNCHER_LIB_DIR}/libGL.so*"
                    "${MATTI_LAUNCHER_LIB_DIR}/libEGL.so*")
            foreach(_lib IN LISTS _runtime_libs)
                add_custom_command(TARGET launcher POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_lib}" "${CMAKE_BINARY_DIR}/bin/lib/"
                        VERBATIM)
            endforeach()
            install(DIRECTORY "${CMAKE_BINARY_DIR}/bin/lib/"
                    DESTINATION ${CMAKE_INSTALL_BINDIR}/lib)
        else()
            message(WARNING "MATTI_LAUNCHER_LIB_DIR does not exist: ${MATTI_LAUNCHER_LIB_DIR}")
        endif()
    endif()

    if(MATTI_LAUNCHER_BUNDLE_LIBS AND UNIX AND NOT APPLE)
        add_custom_command(TARGET launcher POST_BUILD
                COMMAND bash "${CMAKE_SOURCE_DIR}/packaging/linux/bundle-launcher-libs.sh"
                        "${_staged}" "${CMAKE_BINARY_DIR}/bin/lib"
                COMMENT "Bundling launcher runtime libraries"
                VERBATIM)
    endif()
    if(EXISTS "${CMAKE_BINARY_DIR}/bin/lib")
        install(DIRECTORY "${CMAKE_BINARY_DIR}/bin/lib/"
                DESTINATION ${CMAKE_INSTALL_BINDIR}/lib)
    endif()

    install(PROGRAMS "${_staged}"
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

    message(STATUS "Matticraft launcher: ${_staged}")
endfunction()
