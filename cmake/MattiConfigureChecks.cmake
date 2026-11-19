# MattiConfigureChecks.cmake - autoconf-style feature detection.
#
# Included after project(). Every probe becomes a HAVE_* macro in the generated
# config.h and is shown in the configure summary, similar to ./configure.
#
# The check results are cached by CMake. To re-run a probe:
#     cmake -U 'HAVE_*' -S . -B build

include_guard(GLOBAL)

include(CheckCSourceCompiles)
include(CheckIncludeFile)
include(CheckLibraryExists)
include(CheckSymbolExists)
include(CheckTypeSize)
include(CMakePushCheckState)

# A CMakeCache.txt moved from another machine can carry check results from a
# different compiler/toolchain. MattiPlatform sets this flag when it had to
# drop a stale compiler path.
if(MATTI_COMPILER_CHANGED)
    message(STATUS "Compiler changed - re-running the feature checks")
    get_cmake_property(_matti_cache_variables CACHE_VARIABLES)
    foreach(_matti_cache_variable IN LISTS _matti_cache_variables)
        if(_matti_cache_variable MATCHES "^(HAVE_|SIZEOF_)")
            unset(${_matti_cache_variable} CACHE)
        endif()
    endforeach()
endif()

cmake_push_check_state(RESET)
# The library uses POSIX/GNU interfaces (strndup, getline, dlinfo, ...).
set(CMAKE_REQUIRED_DEFINITIONS "-D_GNU_SOURCE")

# ---------------------------------------------------------------------------
# Headers
# ---------------------------------------------------------------------------
set(MATTI_CHECK_HEADERS
        dlfcn.h
        dirent.h
        fcntl.h
        poll.h
        pthread.h
        sched.h
        sys/inotify.h
        sys/mman.h
        sys/random.h
        sys/socket.h
        sys/stat.h
        sys/time.h
        sys/types.h
        sys/wait.h
        unistd.h
        execinfo.h)

foreach(_matti_header IN LISTS MATTI_CHECK_HEADERS)
    string(MAKE_C_IDENTIFIER "${_matti_header}" _matti_id)
    string(TOUPPER "${_matti_id}" _matti_id)
    check_include_file("${_matti_header}" "HAVE_${_matti_id}")
endforeach()

# ---------------------------------------------------------------------------
# Functions / symbols
# ---------------------------------------------------------------------------
# "<function>|<header it is declared in>"
set(MATTI_CHECK_SYMBOLS
        "dlopen|dlfcn.h"
        "dlsym|dlfcn.h"
        "dlclose|dlfcn.h"
        "dlerror|dlfcn.h"
        "pthread_create|pthread.h"
        "pthread_setname_np|pthread.h"
        "pthread_getspecific|pthread.h"
        "pthread_once|pthread.h"
        "clock_gettime|time.h"
        "nanosleep|time.h"
        "strdup|string.h"
        "strndup|string.h"
        "getline|stdio.h"
        "realpath|stdlib.h"
        "posix_memalign|stdlib.h"
        "aligned_alloc|stdlib.h"
        "mmap|sys/mman.h"
        "mprotect|sys/mman.h"
        "inotify_init1|sys/inotify.h"
        "inotify_add_watch|sys/inotify.h"
        "inotify_rm_watch|sys/inotify.h"
        "socket|sys/socket.h"
        "getrandom|sys/random.h"
        "getentropy|unistd.h"
        "sched_getaffinity|sched.h"
        "sysconf|unistd.h"
        "backtrace|execinfo.h"
        "mkdtemp|stdlib.h"
        "fork|unistd.h"
        "waitpid|sys/wait.h"
        "kill|signal.h")

foreach(_matti_entry IN LISTS MATTI_CHECK_SYMBOLS)
    string(REPLACE "|" ";" _matti_parts "${_matti_entry}")
    list(GET _matti_parts 0 _matti_func)
    list(GET _matti_parts 1 _matti_header)
    string(MAKE_C_IDENTIFIER "${_matti_func}" _matti_id)
    string(TOUPPER "${_matti_id}" _matti_id)
    check_symbol_exists("${_matti_func}" "${_matti_header}" "HAVE_${_matti_id}")
endforeach()

# ---------------------------------------------------------------------------
# Libraries
# ---------------------------------------------------------------------------
check_library_exists(dl dlopen "" HAVE_LIBDL)
check_library_exists(pthread pthread_create "" HAVE_LIBPTHREAD)
check_library_exists(rt clock_gettime "" HAVE_LIBRT)
check_library_exists(m pow "" HAVE_LIBM)
check_library_exists(z gzopen "" HAVE_LIBZ)

# ---------------------------------------------------------------------------
# Types
# ---------------------------------------------------------------------------
check_type_size("void *" SIZEOF_VOID_P)
check_type_size("long" SIZEOF_LONG)
check_type_size("size_t" SIZEOF_SIZE_T)
set(CMAKE_EXTRA_INCLUDE_FILES "sys/types.h")
check_type_size("off_t" SIZEOF_OFF_T)
unset(CMAKE_EXTRA_INCLUDE_FILES)

# ---------------------------------------------------------------------------
# Compiler / ELF toolchain capabilities (mods are ELF sections)
# ---------------------------------------------------------------------------
check_c_source_compiles([=[
__attribute__((section(".matti_probe")))
static const char matti_probe[] = "probe";

int main(void)
{
    return matti_probe[0] == 'p';
}
]=] HAVE_SECTION_ATTRIBUTE)

find_program(MATTI_OBJCOPY NAMES objcopy)
if(MATTI_OBJCOPY)
    set(HAVE_OBJCOPY 1)
else()
    set(HAVE_OBJCOPY 0)
    message(WARNING "objcopy not found - mods cannot get .matti sections (install binutils)")
endif()

# ---------------------------------------------------------------------------
# zlib (needed by java.util.zip, PNG decoding and NBT compression)
# ---------------------------------------------------------------------------
# Drop a cached zlib path left by a build tree from another distribution.
if(COMMAND matti_discard_missing_cache_path)
    foreach(_matti_zlib_var IN ITEMS
            ZLIB_INCLUDE_DIR
            ZLIB_LIBRARY
            ZLIB_LIBRARY_RELEASE
            ZLIB_LIBRARY_DEBUG)
        matti_discard_missing_cache_path(${_matti_zlib_var})
    endforeach()
endif()

# NixOS keeps zlib's headers in the package's dev output, which is not on the
# compiler's search path outside a nix-shell. Probe the profiles and the store
# before the package lookup and the header check, so a fresh build tree works
# without a cache from another machine.
if(NOT EXISTS "${ZLIB_INCLUDE_DIR}/zlib.h")
    file(GLOB _matti_zlib_dev_includes "/nix/store/*zlib*-dev/include")
    find_path(MATTI_ZLIB_INCLUDE_DIR
            NAMES zlib.h
            PATHS ${CMAKE_PREFIX_PATH} ${_matti_zlib_dev_includes}
            PATH_SUFFIXES include)
    if(MATTI_ZLIB_INCLUDE_DIR AND EXISTS "${MATTI_ZLIB_INCLUDE_DIR}/zlib.h")
        set(ZLIB_INCLUDE_DIR "${MATTI_ZLIB_INCLUDE_DIR}" CACHE PATH "zlib header directory" FORCE)
    endif()
endif()
if(ZLIB_INCLUDE_DIR AND EXISTS "${ZLIB_INCLUDE_DIR}/zlib.h")
    set(ZLIB_INCLUDE_DIRS "${ZLIB_INCLUDE_DIR}")
    set(CMAKE_REQUIRED_INCLUDES "${ZLIB_INCLUDE_DIR}")
endif()

find_package(ZLIB QUIET)

# NixOS splits zlib into a dev output (zlib.h, no lib/) and a library output
# (lib/libz.so). find_package(ZLIB) sees only the header through
# CMAKE_PREFIX_PATH and then reports "not found" for the library. Recover the
# matching library from the store so a fresh configure (CMakePresets) works
# without hard-coding a store hash.
if(NOT ZLIB_FOUND AND ZLIB_INCLUDE_DIR MATCHES "^/nix/store/")
    # The matching library output of the same zlib derivation: strip the -dev
    # suffix from the directory that holds zlib.h. Do not pick a random store
    # libz.so (a 32-bit build may come first in the glob).
    get_filename_component(_matti_zlib_prefix "${ZLIB_INCLUDE_DIR}/.." ABSOLUTE)
    string(REGEX REPLACE "-dev$" "" _matti_zlib_prefix "${_matti_zlib_prefix}")
    set(_matti_zlib_library "${_matti_zlib_prefix}/lib/libz.so")
    if(NOT EXISTS "${_matti_zlib_library}")
        file(GLOB _matti_zlib_candidates "/nix/store/*-zlib-*/lib/libz.so")
        set(_matti_zlib_library "")
        foreach(_matti_candidate IN LISTS _matti_zlib_candidates)
            if(NOT EXISTS "${_matti_candidate}")
                continue()
            endif()
            # EI_CLASS lives at byte 4 (hex 02 = 64-bit, 01 = 32-bit).
            file(READ "${_matti_candidate}" _matti_zlib_header LIMIT 5 HEX)
            string(SUBSTRING "${_matti_zlib_header}" 8 2 _matti_zlib_class)
            if((CMAKE_SIZEOF_VOID_P EQUAL 8 AND _matti_zlib_class STREQUAL "02") OR
               (CMAKE_SIZEOF_VOID_P EQUAL 4 AND _matti_zlib_class STREQUAL "01"))
                set(_matti_zlib_library "${_matti_candidate}")
                break()
            endif()
        endforeach()
    endif()
    if(EXISTS "${_matti_zlib_library}")
        set(ZLIB_LIBRARY "${_matti_zlib_library}" CACHE FILEPATH "" FORCE)
        set(ZLIB_LIBRARIES "${_matti_zlib_library}" CACHE STRING "" FORCE)
        set(ZLIB_INCLUDE_DIR "${ZLIB_INCLUDE_DIR}" CACHE PATH "" FORCE)
        set(ZLIB_INCLUDE_DIRS "${ZLIB_INCLUDE_DIR}" CACHE STRING "" FORCE)
        set(ZLIB_FOUND TRUE CACHE BOOL "" FORCE)
        if(NOT TARGET ZLIB::ZLIB)
            add_library(ZLIB::ZLIB UNKNOWN IMPORTED)
            set_target_properties(ZLIB::ZLIB PROPERTIES
                    IMPORTED_LOCATION "${_matti_zlib_library}"
                    INTERFACE_INCLUDE_DIRECTORIES "${ZLIB_INCLUDE_DIR}")
        endif()
    endif()
endif()

if(ZLIB_FOUND)
    set(HAVE_ZLIB 1)
    set(HAVE_ZLIB_H 1)
    set(HAVE_LIBZ 1)
else()
    check_include_file("zlib.h" HAVE_ZLIB_H)
    if(HAVE_ZLIB_H AND HAVE_LIBZ)
        set(HAVE_ZLIB 1)
    else()
        set(HAVE_ZLIB 0)
    endif()
endif()

if(NOT HAVE_ZLIB)
    message(FATAL_ERROR
            "zlib not found (zlib.h and -lz are required).\n"
            "Install the development package, for example:\n"
            "    Debian/Ubuntu: sudo apt install zlib1g-dev\n"
            "    Fedora:        sudo dnf install zlib-devel\n"
            "    Arch:          sudo pacman -S zlib\n"
            "    NixOS:         import nix/home.nix into Home Manager (see README)\n"
            "or point CMake at it with -DZLIB_ROOT=<prefix>.")
endif()

cmake_pop_check_state()

# ---------------------------------------------------------------------------
# Generated config.h
# ---------------------------------------------------------------------------
set(MATTI_CONFIG_HEADER "${CMAKE_BINARY_DIR}/config.h" CACHE INTERNAL
        "generated feature/config header")
set(MATTI_CONFIG_INCLUDE_DIR "${CMAKE_BINARY_DIR}" CACHE INTERNAL
        "include directory of the generated config.h")

configure_file("${CMAKE_CURRENT_LIST_DIR}/config.h.in"
        "${MATTI_CONFIG_HEADER}" @ONLY)

# Add the generated config header to a target. Every C file sees the HAVE_*
# macros through -include, like autoconf's config.h include.
function(matti_apply_config target)
    if(NOT EXISTS "${MATTI_CONFIG_HEADER}")
        message(FATAL_ERROR
                "matti_apply_config(${target}): include(MattiConfigureChecks) first")
    endif()

    target_include_directories(${target} PRIVATE "${MATTI_CONFIG_INCLUDE_DIR}")
    target_compile_definitions(${target} PRIVATE HAVE_CONFIG_H=1)

    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE
                -include "${MATTI_CONFIG_HEADER}")
    endif()
endfunction()

# ---------------------------------------------------------------------------
# Configure summary
# ---------------------------------------------------------------------------
function(_matti_yesno out var)
    if(${var})
        set(${out} "yes" PARENT_SCOPE)
    else()
        set(${out} "no" PARENT_SCOPE)
    endif()
endfunction()

_matti_yesno(_matti_nixos MATTI_IS_NIXOS)
_matti_yesno(_matti_dlopen HAVE_DLOPEN)
_matti_yesno(_matti_dlopen_h HAVE_DLFCN_H)
_matti_yesno(_matti_pthread HAVE_PTHREAD_CREATE)
_matti_yesno(_matti_inotify HAVE_INOTIFY_INIT1)
_matti_yesno(_matti_getrandom HAVE_GETRANDOM)
_matti_yesno(_matti_strndup HAVE_STRNDUP)
_matti_yesno(_matti_getline HAVE_GETLINE)
_matti_yesno(_matti_realpath HAVE_REALPATH)
_matti_yesno(_matti_mmap HAVE_MMAP)
_matti_yesno(_matti_backtrace HAVE_BACKTRACE)
_matti_yesno(_matti_objcopy HAVE_OBJCOPY)
_matti_yesno(_matti_section HAVE_SECTION_ATTRIBUTE)

if(ZLIB_FOUND)
    set(_matti_zlib "${ZLIB_VERSION_STRING} (${ZLIB_LIBRARIES})")
else()
    set(_matti_zlib "system -lz")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "")
    set(_matti_build_type "default")
else()
    set(_matti_build_type "${CMAKE_BUILD_TYPE}")
endif()

message(STATUS "")
message(STATUS "Matticraft configure summary")
message(STATUS "-----------------------------")
message(STATUS "  Host ............. ${MATTI_OS_PRETTY_NAME} "
        "(${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR})")
message(STATUS "  NixOS ............ ${_matti_nixos}")
message(STATUS "  C compiler ....... ${CMAKE_C_COMPILER}")
message(STATUS "  Compiler ID ...... ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "  Build type ....... ${_matti_build_type}")
message(STATUS "  zlib ............. ${_matti_zlib}")
message(STATUS "  dlfcn.h/dlopen ... ${_matti_dlopen_h}/${_matti_dlopen}")
message(STATUS "  pthread_create ... ${_matti_pthread}")
message(STATUS "  inotify_init1 .... ${_matti_inotify}")
message(STATUS "  getrandom ........ ${_matti_getrandom}")
message(STATUS "  strndup .......... ${_matti_strndup}")
message(STATUS "  getline .......... ${_matti_getline}")
message(STATUS "  realpath ......... ${_matti_realpath}")
message(STATUS "  mmap ............. ${_matti_mmap}")
message(STATUS "  backtrace ........ ${_matti_backtrace}")
message(STATUS "  objcopy .......... ${_matti_objcopy}")
message(STATUS "  section attr ..... ${_matti_section}")
message(STATUS "  config header .... ${MATTI_CONFIG_HEADER}")
message(STATUS "")
