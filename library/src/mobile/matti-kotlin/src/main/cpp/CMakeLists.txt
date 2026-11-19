cmake_minimum_required(VERSION 3.22)
project(matti_mobile C)

# M2 scaffold: LIBMATTI_ROOT is the repository root, set by Gradle. The main
# project builds the one libmatti (C) plus the Rust window staticlib for the
# Android ABI; the platform/bit-width dispatch happens inside that library.
set(LIBMATTI_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../../../../.." CACHE PATH "LibMatti root")
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
