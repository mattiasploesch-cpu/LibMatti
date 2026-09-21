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

add_library(matti STATIC
        ${LIBMATTI_SOURCES}
)

target_include_directories(matti
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
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