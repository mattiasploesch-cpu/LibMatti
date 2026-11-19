file(GLOB_RECURSE LIBMATTI_SOURCES
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

# The vendored stb_truetype (used by the LWJGL STBTruetype port) instantiates
# in exactly one translation unit, like the upstream "easy to use" section says.
list(APPEND LIBMATTI_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/libmatti/org/lwjgl/stb/stb_truetype_impl.c")

# Java: loader/build.gradle generates FMLVersionProperties with the gradle project version
# (gradleutils.version, a `git describe` over the cloned FML repository).
find_package(Git QUIET)
set(LIBMATTI_FML_VERSION "26.1.0")
if(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../vendor/FancyModLoader/.git")
    execute_process(
            COMMAND ${GIT_EXECUTABLE} describe --tags
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../vendor/FancyModLoader"
            OUTPUT_VARIABLE LIBMATTI_FML_GIT_DESCRIBE
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
            RESULT_VARIABLE LIBMATTI_FML_GIT_RESULT
    )
    if(LIBMATTI_FML_GIT_RESULT EQUAL 0 AND LIBMATTI_FML_GIT_DESCRIBE MATCHES "^([0-9]+\\.[0-9]+)")
        # Java: gradleutils strips the branch suffix to the base version
        set(LIBMATTI_FML_VERSION "${CMAKE_MATCH_1}.0")
    endif()
endif()
message(STATUS "FML version: ${LIBMATTI_FML_VERSION}")

# Every backend compiles once into an object library. The static archive keeps
# the existing clients working; the shared library is the one distributable
# "libmatti" file that carries all renderers, the Rust window and the assets
# (planported.MD, Punkt 1: eine Bibliotheksdatei pro Ziel).
add_library(matti_objects OBJECT
        ${LIBMATTI_SOURCES}
)
set_target_properties(matti_objects PROPERTIES
        POSITION_INDEPENDENT_CODE ON
)

# HAVE_* feature macros from the configure checks, force-included like the
# config.h of an autoconf build.
if(COMMAND matti_apply_config)
    matti_apply_config(matti_objects)
endif()

# Optional renderer backends. The sources are always compiled; the defines
# switch the real implementation on, otherwise the backend reports itself as
# unavailable. OpenGL needs no development headers (GLFW/GL are dlopen'ed).
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(MATTI_SDL3 QUIET IMPORTED_TARGET sdl3)
    pkg_check_modules(MATTI_VULKAN QUIET IMPORTED_TARGET vulkan)
endif()

# vcpkg/Homebrew/SDL3's own CMake package: used when pkg-config has no sdl3.
if(NOT MATTI_SDL3_FOUND)
    find_package(SDL3 CONFIG QUIET)
    if(SDL3_FOUND)
        set(MATTI_SDL3_FOUND TRUE)
        set(MATTI_SDL3_VERSION "${SDL3_VERSION}")
        if(TARGET SDL3::SDL3)
            set(MATTI_SDL3_TARGET SDL3::SDL3)
        elseif(TARGET SDL3::SDL3-shared)
            set(MATTI_SDL3_TARGET SDL3::SDL3-shared)
        endif()
    endif()
else()
    set(MATTI_SDL3_TARGET PkgConfig::MATTI_SDL3)
endif()

if(MATTI_SDL3_FOUND)
    target_compile_definitions(matti_objects PRIVATE MATTI_HAVE_SDL3=1)
    if(MATTI_SDL3_TARGET)
        target_link_libraries(matti_objects PUBLIC ${MATTI_SDL3_TARGET})
    endif()
    message(STATUS "SDL3 renderer backend: enabled (${MATTI_SDL3_VERSION})")
else()
    message(STATUS "SDL3 renderer backend: disabled (no pkg-config module or SDL3 CMake package)")
endif()

set(MATTI_VULKAN_ENABLED OFF)
if(MATTI_VULKAN_FOUND)
    set(MATTI_VULKAN_ENABLED ON)
    target_link_libraries(matti_objects PUBLIC PkgConfig::MATTI_VULKAN)
endif()
if(NOT MATTI_VULKAN_ENABLED)
    find_package(Vulkan QUIET)
    if(Vulkan_FOUND)
        set(MATTI_VULKAN_ENABLED ON)
        target_link_libraries(matti_objects PUBLIC Vulkan::Vulkan)
    endif()
endif()
if(MATTI_VULKAN_ENABLED)
    target_compile_definitions(matti_objects PRIVATE MATTI_HAVE_VULKAN=1)
    message(STATUS "Vulkan renderer backend: enabled")
else()
    message(STATUS "Vulkan renderer backend: disabled (no Vulkan headers/loader found)")
endif()


target_include_directories(matti_objects
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
# the vendored stb headers (the stb submodule, see .gitmodules)
target_include_directories(matti_objects
        SYSTEM PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/stb
)

target_compile_definitions(matti_objects
        PRIVATE
        LIBMATTI_FML_VERSION="${LIBMATTI_FML_VERSION}"
        # the lib reads theme resources (Monocraft.ttf) from the source tree
        MATTI_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

# Java: java.util.zip sits on zlib and java.lang.Math on libm; every consumer of the
# library links them transitively.
# find_package(stb)
# target_link_libraries(matti_objects PUBLIC stb::stb_truetype)

target_include_directories(matti_objects PRIVATE ${CMAKE_SOURCE_DIR}/vendor/stb)

find_package(ZLIB QUIET)
if(ZLIB_FOUND)
    target_link_libraries(matti_objects PUBLIC ZLIB::ZLIB)
    # On NixOS the zlib headers live in the package's dev output, a separate
    # store path from the library. Put it in the compile flags explicitly; the
    # imported target does not always carry it outside a nix-shell.
    if(ZLIB_INCLUDE_DIRS)
        target_compile_options(matti_objects PRIVATE "-I${ZLIB_INCLUDE_DIRS}")
    elseif(ZLIB_INCLUDE_DIR)
        target_compile_options(matti_objects PRIVATE "-I${ZLIB_INCLUDE_DIR}")
    endif()
else()
    # The configure checks verified that zlib.h and -lz are available even when
    # CMake did not find a zlib package config.
    target_link_libraries(matti_objects PUBLIC z)
endif()

# dlopen/dlsym (GL, GLFW, mods), pthreads and libm. CMAKE_DL_LIBS is "dl" on
# Linux, empty on platforms where the symbols live in libc.
if(TARGET Threads::Threads)
    target_link_libraries(matti_objects PUBLIC Threads::Threads)
endif()
if(CMAKE_DL_LIBS)
    target_link_libraries(matti_objects PUBLIC ${CMAKE_DL_LIBS})
endif()
if(UNIX AND NOT APPLE)
    target_link_libraries(matti_objects PUBLIC m)
endif()

# Rust: the UI lives in library/src/libmatti/rs and is reached through the C
# API only (libmatti/rs/matti_rs.h). The crate builds as a staticlib and is
# linked into matti, so every client gets it without extra wiring.
include(MattiRust)
matti_add_rust_staticlib(matti_rust
        MANIFEST "${CMAKE_CURRENT_SOURCE_DIR}/src/libmatti/rs/Cargo.toml"
        LIB_NAME matti_rs
        TARGET_DIR "${CMAKE_CURRENT_BINARY_DIR}/cargo")
# The Rust window is not part of matti_objects on purpose: a static archive
# cannot link another archive, and tests that never call the window should not
# carry the Rust half. Executables that call into the UI link matti_rust next
# to matti; the shared library contains both.

# The static archive the clients and tests link.
add_library(matti STATIC)
target_link_libraries(matti PUBLIC matti_objects)

# The single shared artifact: all layer/API backends, the C API, the embedded
# Rust window and the GUI assets. Consumers can dlopen it; the platform, the
# pointer width and the renderer selection are decided inside.
add_library(matti_shared SHARED)
target_link_libraries(matti_shared PRIVATE matti_objects matti_rust)
set_target_properties(matti_shared PROPERTIES
        OUTPUT_NAME matti
        VERSION 0.1.0
        SOVERSION 0
        POSITION_INDEPENDENT_CODE ON)
if(COMMAND matti_embed_ui_assets)
    matti_embed_ui_assets(matti_shared)
endif()
