file(GLOB_RECURSE LIBMATTI_SOURCES
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

# The vendored stb_truetype (used by the LWJGL STBTruetype port) instantiates
# in exactly one translation unit, like the upstream "easy to use" section says.
list(APPEND LIBMATTI_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/libmatti/org/lwjgl/stb/stb_truetype_impl.c")

# Java: loader/build.gradle generates FMLVersionProperties with the gradle project version
# (gradleutils.version, a `git describe` over the cloned FML repository).
find_package(Git QUIET)
set(LIBMATTI_FML_VERSION "26.1.0")
if(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../vendor/FancyModLoader/.git")
    execute_process(
            COMMAND ${GIT_EXECUTABLE} describe --tags
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../vendor/FancyModLoader"
            OUTPUT_VARIABLE LIBMATTI_FML_GIT_DESCRIBE
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
            RESULT_VARIABLE LIBMATTI_FML_GIT_RESULT
    )
    if(LIBMATTI_FML_GIT_RESULT EQUAL 0 AND LIBMATTI_FML_GIT_DESCRIBE MATCHES "^([0-9]+\\.[0-9]+)")
        # Java: gradleutils strips the branch suffix to the base version
        set(LIBMATTI_FML_VERSION "${CMAKE_MATCH_1}.0")
    endif()
endif()
message(STATUS "FML version: ${LIBMATTI_FML_VERSION}")

# Java: the vanilla resources ride the jar (the classpath pack). The port
# packs library/resources/ into the gzip blob the executable links in:
# embed_resources builds the deterministic tar + gzip, generates the C array
# (EmbeddedPackResources_data.c/.h into the generated include dir) and the
# library links the blob object - the runtime inflates it into RAM only, no
# --assetsRoot, nothing on disk.
set(LIBMATTI_RESOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources")
set(LIBMATTI_EMBED_TOOL "${CMAKE_SOURCE_DIR}/tools/embed_resources.c")
set(LIBMATTI_EMBED_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(LIBMATTI_EMBED_DATA_C "${LIBMATTI_EMBED_GENERATED_DIR}/EmbeddedPackResources_data.c")
set(LIBMATTI_EMBED_DATA_H "${LIBMATTI_EMBED_GENERATED_DIR}/libmatti/net/minecraft/server/packs/embedded/EmbeddedPackResources_data.h")
set(LIBMATTI_EMBED_BLOB "${LIBMATTI_EMBED_GENERATED_DIR}/EmbeddedPackResources.bin")

add_executable(embed_resources EXCLUDE_FROM_ALL "${LIBMATTI_EMBED_TOOL}")
# the tool rides zlib (the gzip stream it writes, like the java.util.zip port)
find_package(ZLIB QUIET)
if(ZLIB_FOUND)
    target_link_libraries(embed_resources PRIVATE ZLIB::ZLIB)
else()
    target_link_libraries(embed_resources PRIVATE z)
endif()

add_custom_command(
        OUTPUT "${LIBMATTI_EMBED_DATA_C}" "${LIBMATTI_EMBED_DATA_H}" "${LIBMATTI_EMBED_BLOB}"
        COMMAND $<TARGET_FILE:embed_resources> "${LIBMATTI_RESOURCES_DIR}" "${LIBMATTI_EMBED_GENERATED_DIR}"
        DEPENDS embed_resources "${LIBMATTI_EMBED_TOOL}"
        COMMENT "Packing library/resources into the embedded resource blob"
        VERBATIM
)
add_custom_target(embed_resources_blob
        DEPENDS "${LIBMATTI_EMBED_DATA_C}" "${LIBMATTI_EMBED_DATA_H}" "${LIBMATTI_EMBED_BLOB}"
)

add_library(matti STATIC
        ${LIBMATTI_SOURCES}
        "${LIBMATTI_EMBED_DATA_C}"
)
add_dependencies(matti embed_resources_blob)

target_include_directories(matti
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        # the generated EmbeddedPackResources_data.h (the blob declaration)
        ${LIBMATTI_EMBED_GENERATED_DIR}
        # the vendored stb headers (the stb submodule, see .gitmodules)
        SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/stb
)

target_compile_definitions(matti
        PRIVATE
        LIBMATTI_FML_VERSION="${LIBMATTI_FML_VERSION}"
        # the lib reads theme resources (Monocraft.ttf) from the source tree
        MATTI_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

# Java: java.util.zip sits on zlib and java.lang.Math on libm; every consumer of the
# library links them transitively.
find_package(ZLIB QUIET)
if(ZLIB_FOUND)
    target_link_libraries(matti PUBLIC ZLIB::ZLIB)
else()
    target_link_libraries(matti PUBLIC z)
endif()
target_link_libraries(matti PUBLIC m)
