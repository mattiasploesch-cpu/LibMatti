cmake_minimum_required(VERSION 3.20)

project(Matticraft C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

enable_testing()

add_subdirectory(library)
add_subdirectory(client)
add_subdirectory(tests)

# The mods are a separate project: they only include the library headers, the
# running game provides the symbols. This target configures and builds the mods
# project (cmake -S mods -B mods/build && cmake --build mods/build); the built
# .so files land in mods/, the folder --gameDir <repo root> scans.
add_custom_target(build_mods
        COMMAND ${CMAKE_COMMAND} -S "${CMAKE_CURRENT_SOURCE_DIR}/mods" -B "${CMAKE_CURRENT_SOURCE_DIR}/mods/build"
        COMMAND ${CMAKE_COMMAND} --build "${CMAKE_CURRENT_SOURCE_DIR}/mods/build"
        COMMENT "Building the mods project (output: mods/*.so)"
        VERBATIM
)

# One target for everything: building "matticraft" builds the library, the
# game, every test harness with its fixture mods and the mods project. Running
# it executes the selftest phase (every harness) and then the launcher.
add_dependencies(matticraft build_mods
        test_modscan test_soscan test_mixinfacade test_m4proof)

# The plain client run (run configuration "runClient"): the same binary as
# matticraft but without the test harnesses as dependencies - the selftest
# phase is skipped via --no-tests, so no harness binary is needed.
add_executable(runClient)

target_link_libraries(runClient PRIVATE
        $<LINK_LIBRARY:WHOLE_ARCHIVE,matti>
)
target_sources(runClient PRIVATE
        client/src/main.c
        client/src/selftest.c
        client/src/demo-service.c
)
target_include_directories(runClient PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/client/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/library/src"
)
target_compile_definitions(runClient PRIVATE
        MATTI_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
        MATTI_BUILD_DIR="${CMAKE_BINARY_DIR}"
        # the selftest phase probes for the harness binaries; runClient ships without them
        MATTICRAFT_CLIENT_ONLY=1
)
set_target_properties(runClient PROPERTIES
        ENABLE_EXPORTS ON
)

# The demo mods come with the client.
add_dependencies(runClient demo_app demo_lib build_mods)
