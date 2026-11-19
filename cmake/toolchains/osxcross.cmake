# macOS cross build from Linux using osxcross.
#
#   export OSXCROSS_ROOT=$HOME/osxcross
#   export OSXCROSS_TARGET=arm64-apple-darwin23   (or x86_64-apple-darwin23)
#   cmake -S . -B build/macos-osxcross \
#         -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/osxcross.cmake
#
# The Apple SDK is not redistributable; it must be provided by the user
# (see osxcross' README). Without it this toolchain cannot be used.
set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_SYSTEM_PROCESSOR arm64)

if(NOT OSXCROSS_ROOT)
    message(FATAL_ERROR "OSXCROSS_ROOT is not set (point it at the osxcross checkout)")
endif()
if(NOT OSXCROSS_TARGET)
    set(OSXCROSS_TARGET "arm64-apple-darwin23")
endif()
if(NOT OSXCROSS_SDK)
    set(OSXCROSS_SDK "MacOSX14.0.sdk")
endif()

set(CMAKE_C_COMPILER "${OSXCROSS_ROOT}/target/bin/${OSXCROSS_TARGET}-clang")
set(CMAKE_OSX_SYSROOT "${OSXCROSS_ROOT}/target/SDK/${OSXCROSS_SDK}")
set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "" FORCE)
set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "" FORCE)
set(CMAKE_FIND_ROOT_PATH "${OSXCROSS_ROOT}/target")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
