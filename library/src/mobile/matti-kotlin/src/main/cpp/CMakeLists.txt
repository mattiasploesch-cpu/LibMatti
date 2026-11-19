cmake_minimum_required(VERSION 3.22)
project(matti_mobile C)

# M2 scaffold: LIBMATTI_ROOT is the repository root, set by Gradle. The main
# project builds the one libmatti (C) plus the Rust window staticlib for the
# Android ABI; the platform/bit-width dispatch happens inside that library.
set(LIBMATTI_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../../../../.." CACHE PATH "LibMatti root")
# The root project sets this path; the mobile wrapper includes the root
# CMakeLists.txt directly, so it must provide the helper modules itself.
list(APPEND CMAKE_MODULE_PATH "${LIBMATTI_ROOT}/cmake")
if(EXISTS "${LIBMATTI_ROOT}/CMakeLists.txt")
    add_subdirectory("${LIBMATTI_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/libmatti")
endif()

add_library(matti_mobile SHARED libmatti_jni.c)
target_include_directories(matti_mobile PRIVATE "${LIBMATTI_ROOT}/library/src")

if(TARGET matti)
    target_link_libraries(matti_mobile PRIVATE matti)
    if(TARGET matti_rust)
        target_link_libraries(matti_mobile PRIVATE matti_rust)
    endif()
endif()

if(ANDROID)
    # EGL/GLES3 are loaded from the NDK platform libraries, not dlopen'ed.
    target_link_libraries(matti_mobile PRIVATE EGL GLESv3 android log)
endif()

# The Android package has no source tree at runtime: embed Monocraft and the
# GUI PNGs into the JNI library exactly like the desktop executables do.
if(COMMAND matti_embed_ui_assets)
    matti_embed_ui_assets(matti_mobile)
endif()
