# Matticraft mods - a standalone project.
#
# It is deliberately NOT added by the root CMakeLists.txt: mods are separate
# sub-projects that only include the library headers, while libmatti (FML) loads
# the built .so at runtime. Configure and build it on its own (the CLion run
# configuration "build mods" and the matticraft target do exactly this):
#
#     cmake -S mods -B mods/build
#     cmake --build mods/build
#
# Every folder below mods/ that carries its own CMakeLists.txt is added here,
# so a new mod is built by just creating mods/<my-mod>/ with a CMakeLists.txt -
# nothing needs to be registered anywhere.

cmake_minimum_required(VERSION 3.20)

project(MatticraftMods C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# The repository root (the folder that contains library/, client/ and mods/).
get_filename_component(MATTI_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)

# Include root of libmatti. A mod includes "libmatti/..." like any library user,
# but it does not link the library - the running game provides it.
set(MATTI_LIBRARY_DIR "${MATTI_ROOT}/library/src" CACHE PATH "include root of libmatti")# Where built mods land. Default is this folder, which is also the mods folder
# the game scans when it runs with --gameDir <repo root>.
set(MATTI_MODS_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE PATH "output folder for built mods")

# The build helpers (matti_add_mod & co.) are shared with the tests project.
include(cmake/MattiMod.cmake)

# Every subfolder with a CMakeLists.txt is a mod and gets built. The glob runs
# at configure time: after creating a new mod folder, re-run CMake (the IDE
# reloads the project automatically).
file(GLOB modDirs RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/*")
foreach(modDir IN LISTS modDirs)
    # mods/import holds the .matti packages of new_mod PRs (binary only), and
    # mods/build is the build output - neither is a mod project.
    if(modDir STREQUAL "import" OR modDir STREQUAL "build")
        continue()
    endif()
    if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${modDir}"
            AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${modDir}/CMakeLists.txt")
        add_subdirectory(${modDir})
    endif()
endforeach()
