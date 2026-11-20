file(GLOB_RECURSE MATTICRAFT_SOURCES
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

add_executable(vulkancraft
        ${MATTICRAFT_SOURCES}
)

# Mods are ELF shared objects that libmatti (FML) loads at runtime, so the
# running game process has to expose the library symbols they use. The mod .so
# resolves against the host, so the whole archive is linked in - with a plain
# static link the linker drops every object nothing in the executable references
# (for example a config spec a mod registers).
target_link_libraries(vulkancraft
        PRIVATE $<LINK_LIBRARY:WHOLE_ARCHIVE,matti> matti_rust
)

# The selftest phase (client/src/selftest.c) runs the harness binaries of the
# same build tree with their fixture files, so it needs both absolute paths.
target_compile_definitions(vulkancraft PRIVATE
        MATTI_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
        MATTI_BUILD_DIR="${CMAKE_BINARY_DIR}"
)

# Release builds compile the selftest phase out: the packages ship the game
# binary only, the harness paths point at the build machine and would exit
# 127 on every user machine. Debug releases (QA) keep the phase.
target_compile_definitions(vulkancraft PRIVATE
        $<$<CONFIG:Release>:MATTICRAFT_CLIENT_ONLY=1>
)

set_target_properties(vulkancraft PROPERTIES
        ENABLE_EXPORTS ON
)
matti_copy_bin(vulkancraft)

# Ship the native libraries inside the binary: only glibc comes from the host.
if(COMMAND matti_embed_native_bundle)
    matti_embed_native_bundle(vulkancraft)
endif()
if(COMMAND matti_embed_ui_assets)
    matti_embed_ui_assets(vulkancraft)
endif()
if(COMMAND matti_embed_rust_artifacts)
    matti_embed_rust_artifacts(vulkancraft matti_rust ".matti_rust")
endif()

# Demo mods: ELF shared objects (.so) whose manifest lives in the ELF section
# ".matti_manifest". The bootstrap launcher reads it exactly like the manifest
# of a jar. Built into client/demo-mods so the run configuration has a fixed path.
# set(DEMO_MODS_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/demo-mods")

# Building the run target also builds the demo mods.

# Demo mods loaded when running the matticraft target.
# VS_DEBUGGER_ENVIRONMENT is only honoured by the Visual Studio generator.
# CLion ignores it - for CLion use the shared run configuration in
# .idea/runConfigurations (it sets legacyClassPath to these mods).

# string(JOIN ":" MATTICRAFT_LEGACY_CLASSPATH ${MATTICRAFT_DEMO_MODS})
# set_target_properties(vulkancraft PROPERTIES
        # VS_DEBUGGER_ENVIRONMENT "legacyClassPath=${MATTICRAFT_LEGACY_CLASSPATH}"
# )
