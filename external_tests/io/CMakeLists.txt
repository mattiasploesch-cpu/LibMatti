file(GLOB_RECURSE IO_SOURCES
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

add_executable(etest_io
        ${IO_SOURCES}
)

target_link_libraries(etest_io
        PRIVATE $<LINK_LIBRARY:WHOLE_ARCHIVE,matti>
)

target_compile_definitions(etest_io PRIVATE
        MATTI_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
        MATTI_BUILD_DIR="${CMAKE_BINARY_DIR}"
)

set_target_properties(etest_io PROPERTIES
        ENABLE_EXPORTS ON
)

