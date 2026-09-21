file(GLOB_RECURSE MATTICRAFT_SOURCES
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

add_executable(matticraft
        ${MATTICRAFT_SOURCES}
)

# Mods are ELF shared objects that libmatti (FML) loads at runtime, so the
# running game process has to expose the library symbols they use. The mod .so
# resolves against the host, so the whole archive is linked in - with a plain
# static link the linker drops every object nothing in the executable references
# (for example a config spec a mod registers).
target_link_libraries(matticraft
        PRIVATE $<LINK_LIBRARY:WHOLE_ARCHIVE,matti>
)

# The selftest phase (client/src/selftest.c) runs the harness binaries of the
# same build tree with their fixture files, so it needs both absolute paths.
target_compile_definitions(matticraft PRIVATE
        MATTI_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
        MATTI_BUILD_DIR="${CMAKE_BINARY_DIR}"
)

set_target_properties(matticraft PROPERTIES
        ENABLE_EXPORTS ON
)

# Demo mods: ELF shared objects (.so) whose manifest lives in the ELF section
# ".matti_manifest". The bootstrap launcher reads it exactly like the manifest
# of a jar. Built into client/demo-mods so the run configuration has a fixed path.
set(DEMO_MODS_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/demo-mods")

add_library(demo_app SHARED "${CMAKE_CURRENT_SOURCE_DIR}/demo-mods/src/demo-app.c")
add_library(demo_lib SHARED "${CMAKE_CURRENT_SOURCE_DIR}/demo-mods/src/demo-lib.c")

set_target_properties(demo_app PROPERTIES
        PREFIX ""
        OUTPUT_NAME "demo-app-1.0"
        LIBRARY_OUTPUT_DIRECTORY "${DEMO_MODS_OUTPUT_DIR}"
)
set_target_properties(demo_lib PROPERTIES
        PREFIX ""
        OUTPUT_NAME "demo-lib-1.0"
        LIBRARY_OUTPUT_DIRECTORY "${DEMO_MODS_OUTPUT_DIR}"
)

# Building the run target also builds the demo mods.
add_dependencies(matticraft demo_app demo_lib)

# Demo mods loaded when running the matticraft target.
# VS_DEBUGGER_ENVIRONMENT is only honoured by the Visual Studio generator.
# CLion ignores it - for CLion use the shared run configuration in
# .idea/runConfigurations (it sets legacyClassPath to these mods).
set(MATTICRAFT_DEMO_MODS
        "${DEMO_MODS_OUTPUT_DIR}/demo-app-1.0.so"
        "${DEMO_MODS_OUTPUT_DIR}/demo-lib-1.0.so"
)
string(JOIN ":" MATTICRAFT_LEGACY_CLASSPATH ${MATTICRAFT_DEMO_MODS})
set_target_properties(matticraft PROPERTIES
        VS_DEBUGGER_ENVIRONMENT "legacyClassPath=${MATTICRAFT_LEGACY_CLASSPATH}"
)
