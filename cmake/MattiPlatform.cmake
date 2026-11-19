# MattiPlatform.cmake - host OS and toolchain detection.
#
# Included *before* project(). NixOS has no /usr/bin/gcc; the compiler of the
# running system generation lives in /run/current-system/sw/bin. On every other
# OS CMake's normal compiler detection is used unchanged.

include_guard(GLOBAL)

# ---------------------------------------------------------------------------
# Stale cache entries from a build tree created on another machine
# ---------------------------------------------------------------------------
# This repository is often used from a shared disk. A CMakeCache.txt written
# on a distribution with /usr/bin/gcc then points at paths that do not exist on
# NixOS (and vice versa). Drop those entries before project() looks at them.
function(matti_discard_missing_cache_path var)
    if(NOT DEFINED ${var})
        return()
    endif()
    set(_matti_value "${${var}}")
    if(_matti_value STREQUAL "" OR _matti_value MATCHES "-NOTFOUND$")
        return()
    endif()
    if(IS_ABSOLUTE "${_matti_value}" AND NOT EXISTS "${_matti_value}")
        message(STATUS "Ignoring ${var} from another machine: ${_matti_value}")
        if("${var}" STREQUAL "CMAKE_C_COMPILER")
            # The feature check results belong to the old compiler.
            set(MATTI_COMPILER_CHANGED TRUE PARENT_SCOPE)
        endif()
        unset(${var} CACHE)
        unset(${var})
    endif()
endfunction()

foreach(_matti_tool_var IN ITEMS
        CMAKE_C_COMPILER
        CMAKE_C_COMPILER_AR
        CMAKE_C_COMPILER_RANLIB
        CMAKE_AR
        CMAKE_RANLIB
        CMAKE_LINKER
        CMAKE_NM
        CMAKE_OBJCOPY
        CMAKE_OBJDUMP
        CMAKE_READELF
        CMAKE_STRIP
        CMAKE_ADDR2LINE
        CMAKE_MAKE_PROGRAM
        MATTI_OBJCOPY
        OBJCOPY_EXE
        GIT_EXECUTABLE)
    matti_discard_missing_cache_path(${_matti_tool_var})
endforeach()

# ---------------------------------------------------------------------------
# OS detection (/etc/os-release, the same source ./configure based systems use)
# ---------------------------------------------------------------------------
set(MATTI_OS_ID "")
set(MATTI_OS_NAME "")
set(MATTI_OS_VERSION "")
set(MATTI_OS_PRETTY_NAME "")

if(EXISTS "/etc/os-release")
    file(STRINGS "/etc/os-release" _matti_os_release_lines
            REGEX "^[A-Za-z_][A-Za-z0-9_]*=")
    foreach(_matti_os_release_line IN LISTS _matti_os_release_lines)
        if(_matti_os_release_line MATCHES "^([A-Za-z_][A-Za-z0-9_]*)=(.*)$")
            set(_matti_os_key "${CMAKE_MATCH_1}")
            set(_matti_os_value "${CMAKE_MATCH_2}")
            # os-release values may be quoted with " or '
            string(REGEX REPLACE "^\"(.*)\"$" "\\1" _matti_os_value "${_matti_os_value}")
            string(REGEX REPLACE "^'(.*)'$" "\\1" _matti_os_value "${_matti_os_value}")
            set("MATTI_OS_RELEASE_${_matti_os_key}" "${_matti_os_value}")
        endif()
    endforeach()
endif()

if(DEFINED MATTI_OS_RELEASE_ID AND NOT MATTI_OS_RELEASE_ID STREQUAL "")
    set(MATTI_OS_ID "${MATTI_OS_RELEASE_ID}")
endif()
if(DEFINED MATTI_OS_RELEASE_NAME AND NOT MATTI_OS_RELEASE_NAME STREQUAL "")
    set(MATTI_OS_NAME "${MATTI_OS_RELEASE_NAME}")
endif()
if(DEFINED MATTI_OS_RELEASE_VERSION AND NOT MATTI_OS_RELEASE_VERSION STREQUAL "")
    set(MATTI_OS_VERSION "${MATTI_OS_RELEASE_VERSION}")
endif()
if(DEFINED MATTI_OS_RELEASE_PRETTY_NAME AND NOT MATTI_OS_RELEASE_PRETTY_NAME STREQUAL "")
    set(MATTI_OS_PRETTY_NAME "${MATTI_OS_RELEASE_PRETTY_NAME}")
endif()

if(MATTI_OS_NAME STREQUAL "")
    set(MATTI_OS_NAME "${CMAKE_HOST_SYSTEM_NAME}")
endif()
if(MATTI_OS_PRETTY_NAME STREQUAL "")
    if(MATTI_OS_VERSION STREQUAL "")
        set(MATTI_OS_PRETTY_NAME "${MATTI_OS_NAME}")
    else()
        set(MATTI_OS_PRETTY_NAME "${MATTI_OS_NAME} ${MATTI_OS_VERSION}")
    endif()
endif()

set(MATTI_IS_NIXOS FALSE)
if(MATTI_OS_ID STREQUAL "nixos"
        OR EXISTS "/etc/NIXOS"
        OR EXISTS "/run/current-system")
    set(MATTI_IS_NIXOS TRUE)
endif()

set(MATTI_OS_LINUX FALSE)
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    set(MATTI_OS_LINUX TRUE)
endif()

# CMAKE_HOST_SYSTEM_PROCESSOR is empty before project(), so ask uname while the
# detection still runs pre-project.
set(MATTI_OS_ARCH "${CMAKE_HOST_SYSTEM_PROCESSOR}")
if(MATTI_OS_ARCH STREQUAL "")
    execute_process(COMMAND uname -m
            OUTPUT_VARIABLE MATTI_OS_ARCH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET)
endif()
if(MATTI_OS_ARCH STREQUAL "")
    set(MATTI_OS_ARCH "unknown")
endif()

message(STATUS "Detected host OS: ${MATTI_OS_PRETTY_NAME} "
        "(${CMAKE_HOST_SYSTEM_NAME} ${MATTI_OS_ARCH})")

# ---------------------------------------------------------------------------
# Nix profiles (Home Manager / system profile)
# ---------------------------------------------------------------------------
# Home Manager installs development packages into ~/.nix-profile; CMake does
# not search that prefix by default. Adding the profiles makes find_package(),
# find_path() and find_library() see packages installed with home.packages.
if(MATTI_IS_NIXOS)
    set(_matti_nix_profiles "")
    if(DEFINED ENV{NIX_PROFILES})
        string(REPLACE " " ";" _matti_nix_profiles "$ENV{NIX_PROFILES}")
    endif()
    list(APPEND _matti_nix_profiles
            "/nix/var/nix/profiles/default"
            "$ENV{HOME}/.nix-profile"
            "/run/current-system/sw")

    foreach(_matti_profile IN LISTS _matti_nix_profiles)
        if(_matti_profile STREQUAL "")
            continue()
        endif()
        if(EXISTS "${_matti_profile}/include" OR EXISTS "${_matti_profile}/lib")
            list(APPEND CMAKE_PREFIX_PATH "${_matti_profile}")
        endif()
    endforeach()
    list(REMOVE_DUPLICATES CMAKE_PREFIX_PATH)
endif()

# ---------------------------------------------------------------------------
# Compiler selection
# ---------------------------------------------------------------------------
if(MATTI_IS_NIXOS)
    if(CMAKE_C_COMPILER)
        # -DCMAKE_C_COMPILER=... or a parent project already selected one.
        message(STATUS "NixOS: using selected C compiler ${CMAKE_C_COMPILER}")
    elseif(DEFINED ENV{CC} AND NOT "$ENV{CC}" STREQUAL "")
        # CMake initialises CMAKE_C_COMPILER from CC during project().
        message(STATUS "NixOS: using CC from the environment ($ENV{CC})")
    else()
        # The compiler wrapper of the running NixOS generation. Prefer it over
        # whatever happens to be first in PATH (which may belong to a shell).
        unset(MATTI_NIXOS_CC CACHE)
        find_program(MATTI_NIXOS_CC
                NAMES gcc cc
                PATHS "/run/current-system/sw/bin"
                NO_DEFAULT_PATH)
        if(MATTI_NIXOS_CC)
            message(STATUS "NixOS: using compiler from /run/current-system: ${MATTI_NIXOS_CC}")
        else()
            # gcc may only be provided by the Home Manager profile or another dev environment.
            find_program(MATTI_NIXOS_CC NAMES gcc cc)
            if(MATTI_NIXOS_CC)
                message(STATUS "NixOS: no compiler in /run/current-system/sw/bin, "
                        "using ${MATTI_NIXOS_CC} from PATH")
            endif()
        endif()

        if(MATTI_NIXOS_CC)
            set(CMAKE_C_COMPILER "${MATTI_NIXOS_CC}" CACHE FILEPATH "C compiler" FORCE)
        else()
            message(FATAL_ERROR
                    "NixOS detected, but no C compiler was found.\n"
                    "Import the project's modular Home Manager module:\n"
                    "    home-manager.users.<name>.imports = [ /path/to/LibMatti/nix/home.nix ];\n"
                    "or add gcc to environment.systemPackages.")
        endif()
    endif()
else()
    message(STATUS "Using CMake's normal compiler detection")
endif()
