# Matticraft development environment for Home Manager.
#
# Use it as a separate module (copy it to /etc/nixos/matticraft-dev.nix):
#
#     imports = [
#       ./matticraft-dev.nix
#     ];
#
# or merge its home.packages list into your existing home.nix.
#
# The module installs the build toolchain (C and Rust), zlib and the runtime
# libraries into the user profile.
# CMake adds the Home Manager profile to CMAKE_PREFIX_PATH automatically, so
# the normal build works without nix-shell:
#
#     cmake -S . -B cmake-build-debug
#     cmake --build cmake-build-debug -j
{ pkgs, ... }:

{
  home.packages = with pkgs; [
    cmake
    gcc
    gnumake
    binutils
    pkg-config

    # Rust: library/src/libmatti/rs (the UI) is built by cargo as part of the
    # normal CMake build, see cmake/MattiRust.cmake.
    rustc
    cargo

    # zlib is a multi-output package: the headers live in the dev output, the
    # shared library in the default output.
    zlib
    zlib.dev

    # Runtime libraries loaded via dlopen by the client.
    glfw
    libGL
    vulkan-loader
  ];
}
