# MattiNative.cmake - ships the native libraries the client needs inside the
# executable.
#
# The port uses libraries that are part of the build environment: libglfw, the
# GL driver library, the Vulkan loader and SDL3. Distributing the client means
# shipping them, so every library is appended to the executable as its own ELF
# section with objcopy - the same mechanism the mods use for their manifest.
# Only glibc (and the dynamic loader itself) has to come from the host.
#
# Section layout (keep in sync with libmatti/matti/native/Library.h):
#
#   .matti_lib_glfw     libglfw
#   .matti_lib_gl       libGL / libEGL / libOSMesa
#   .matti_lib_vulkan   libvulkan
#   .matti_lib_sdl3     libSDL3
#
# The GLFW and GL bindings dlopen their library, so they load the embedded copy
# and never touch the host's search path. Vulkan and SDL3 are linked at build
# time; their embedded copies travel with the binary for the loader.

include_guard(GLOBAL)

# Keep in sync with libmatti/matti/native/Library.h.
set(MATTI_NATIVE_GLFW_SECTION ".matti_lib_glfw")
set(MATTI_NATIVE_GL_SECTION ".matti_lib_gl")
set(MATTI_NATIVE_VULKAN_SECTION ".matti_lib_vulkan")
set(MATTI_NATIVE_SDL3_SECTION ".matti_lib_sdl3")

# The directories the runtime libraries live in on this host. The Nix profiles
# are the important part: the dynamic loader does not search them.
function(matti_native_directories out_var)
    set(_matti_directories "")
    if(DEFINED ENV{NIX_PROFILES})
        string(REPLACE " " ";" _matti_profiles "$ENV{NIX_PROFILES}")
        foreach(_matti_profile IN LISTS _matti_profiles)
            if(_matti_profile STREQUAL "")
                continue()
            endif()
            list(APPEND _matti_directories "${_matti_profile}/lib")
        endforeach()
    endif()
    list(APPEND _matti_directories
            "/run/current-system/sw/lib"
            "/nix/var/nix/profiles/default/lib"
            "/nix/profile/lib"
            "$ENV{HOME}/.nix-profile/lib"
            "$ENV{HOME}/.local/state/nix/profile/lib"
            "/opt/homebrew/lib"
            "/opt/local/lib"
            "/usr/local/lib"
            "/usr/lib"
            "/usr/lib/x86_64-linux-gnu")

    set(${out_var} "${_matti_directories}" PARENT_SCOPE)
endfunction()

# matti_find_native_library(<out-var> <cache-var> <name>...)
#
# Searches the normal library paths first, then the Nix profiles.
function(matti_find_native_library out_var)
    list(GET ARGN 0 _matti_cache_var)
    list(REMOVE_AT ARGN 0)

    if(DEFINED ${_matti_cache_var} AND EXISTS "${${_matti_cache_var}}")
        set(${out_var} "${${_matti_cache_var}}" PARENT_SCOPE)
        return()
    endif()

    matti_native_directories(_matti_directories)
    find_library(${_matti_cache_var}
            NAMES ${ARGN}
            PATHS ${_matti_directories}
            NO_DEFAULT_PATH)
    if(NOT ${_matti_cache_var})
        find_library(${_matti_cache_var} NAMES ${ARGN})
    endif()

    if(${_matti_cache_var} AND EXISTS "${${_matti_cache_var}}")
        set(${out_var} "${${_matti_cache_var}}" PARENT_SCOPE)
    else()
        set(${out_var} "" PARENT_SCOPE)
    endif()
endfunction()

# matti_embed_native_library(<target> <section> <library-file>)
#
# Appends <library-file> to the linked executable <target> as the ELF section
# <section> (objcopy --add-section, contents,readonly).
function(matti_embed_native_library target section library)
    if(NOT EXISTS "${library}")
        return()
    endif()

    # Mach-O has no objcopy section injection; the .app bundle ships the
    # dylibs under Contents/Frameworks instead (packaging/macos). PE section
    # names are limited to 8 bytes, so the Windows packages ship their DLLs
    # next to the executable and the assets under share/matticraft.
    if(APPLE OR WIN32)
        return()
    endif()

    find_program(MATTI_OBJCOPY_EXE NAMES objcopy llvm-objcopy)
    if(NOT MATTI_OBJCOPY_EXE)
        message(WARNING "objcopy not found - ${library} is not embedded into ${target}")
        return()
    endif()

    add_custom_command(TARGET ${target} POST_BUILD
            COMMAND "${MATTI_OBJCOPY_EXE}"
                    --add-section "${section}=${library}"
                    --set-section-flags "${section}=contents,readonly"
                    "$<TARGET_FILE:${target}>"
            COMMENT "Embedding ${library} into ${target} (${section})"
            VERBATIM)
endfunction()

# The Monocraft font the Rust UI draws with, embedded as an ELF section.
set(MATTI_NATIVE_UI_FONT_SECTION ".matti_font_monocraft")

# matti_embed_ui_assets(<target>)
#
# Ships the UI resources inside the executable: the Monocraft font and the GUI
# textures (panorama, logo, widgets.png, dirt) the C menus draw. Each texture
# becomes its own section .matti_asset_<mangled path>; matti/ui/texture.c looks
# it up after the game's own asset root, so an installed resource pack wins.
function(matti_embed_ui_assets target)
    set(_matti_font
            "${CMAKE_SOURCE_DIR}/library/src/libmatti/matti/ui/Monocraft.ttf")
    if(EXISTS "${_matti_font}")
        matti_embed_native_library(${target} "${MATTI_NATIVE_UI_FONT_SECTION}" "${_matti_font}")
    else()
        message(WARNING "Monocraft.ttf not found - ${target} falls back to text-less menus")
    endif()

    set(_matti_asset_root "${CMAKE_SOURCE_DIR}/library/src/libmatti/matti/ui/assets")
    if(EXISTS "${_matti_asset_root}")
        file(GLOB_RECURSE _matti_assets "${_matti_asset_root}/*")
        foreach(_matti_asset IN LISTS _matti_assets)
            if(IS_DIRECTORY "${_matti_asset}")
                continue()
            endif()
            file(RELATIVE_PATH _matti_relative "${_matti_asset_root}" "${_matti_asset}")
            string(REPLACE "/" "_" _matti_section "${_matti_relative}")
            string(REPLACE "." "_" _matti_section "${_matti_section}")
            string(REPLACE "-" "_" _matti_section "${_matti_section}")
            matti_embed_native_library(${target} ".matti_asset_${_matti_section}" "${_matti_asset}")
        endforeach()
    endif()
endfunction()

# matti_embed_native_bundle(<target>)
#
# Embeds every native library the client uses. Libraries that are not installed
# are skipped with a warning; the client then falls back to the runtime search.
function(matti_embed_native_bundle target)
    matti_find_native_library(_matti_glfw MATTI_GLFW_LIBRARY glfw glfw3)
    matti_find_native_library(_matti_gl MATTI_GL_LIBRARY GL)
    matti_find_native_library(_matti_vulkan MATTI_VULKAN_LIBRARY vulkan)
    matti_find_native_library(_matti_sdl3 MATTI_SDL3_LIBRARY SDL3 sdl3)

    # The Vulkan loader and SDL3 are usually not in the profile; take them from
    # the packages that CMake/pkg-config already resolved.
    if(NOT _matti_vulkan AND DEFINED Vulkan_LIBRARY AND EXISTS "${Vulkan_LIBRARY}")
        set(_matti_vulkan "${Vulkan_LIBRARY}")
    endif()
    if(NOT _matti_sdl3 AND DEFINED MATTI_SDL3_LIBRARY_DIRS)
        find_library(MATTI_SDL3_LIBRARY
                NAMES SDL3 sdl3
                PATHS ${MATTI_SDL3_LIBRARY_DIRS}
                NO_DEFAULT_PATH)
        if(MATTI_SDL3_LIBRARY)
            set(_matti_sdl3 "${MATTI_SDL3_LIBRARY}")
        endif()
    endif()

    if(_matti_glfw)
        matti_embed_native_library(${target} "${MATTI_NATIVE_GLFW_SECTION}" "${_matti_glfw}")
    else()
        message(WARNING "libglfw not found - ${target} relies on the system search at runtime")
    endif()
    if(_matti_gl)
        matti_embed_native_library(${target} "${MATTI_NATIVE_GL_SECTION}" "${_matti_gl}")
    else()
        message(WARNING "libGL not found - ${target} relies on the system search at runtime")
    endif()
    if(_matti_vulkan)
        matti_embed_native_library(${target} "${MATTI_NATIVE_VULKAN_SECTION}" "${_matti_vulkan}")
    endif()
    if(_matti_sdl3)
        matti_embed_native_library(${target} "${MATTI_NATIVE_SDL3_SECTION}" "${_matti_sdl3}")
    endif()
endfunction()
