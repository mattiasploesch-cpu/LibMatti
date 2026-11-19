# MattiRust.cmake - builds the Rust part of libmatti (library/src/libmatti/rs).
#
# C stays the API: the launcher, the mod loader, the early window and the
# renderer live in C. The Rust crate is the UI and exports a small extern "C"
# surface (see libmatti/rs/matti_rs.h) that the C code calls.
#
# cargo is invoked directly instead of pulling in Corrosion: the crate has no
# dependencies, and the artifacts land in the CMake build tree next to the C
# objects. The crate's staticlib is linked into libmatti; every artifact cargo
# produces (staticlib, rlib/cdylib, each [[bin]]) is additionally embedded into
# the client executables with objcopy, like the native libraries.

include_guard(GLOBAL)

find_program(MATTI_CARGO NAMES cargo)

# Lists the [[bin]] targets of the crate (cargo metadata, parsed with
# string(JSON)). Returns an empty list when the crate has none.
function(matti_rust_bin_names manifest out_var)
    set(_matti_bins "")
    if(NOT MATTI_CARGO)
        set(${out_var} "" PARENT_SCOPE)
        return()
    endif()

    execute_process(
            COMMAND "${MATTI_CARGO}" metadata --no-deps --format-version 1
                    --manifest-path "${manifest}"
            OUTPUT_VARIABLE _matti_metadata
            ERROR_QUIET
            RESULT_VARIABLE _matti_result)
    if(NOT _matti_result EQUAL 0)
        set(${out_var} "" PARENT_SCOPE)
        return()
    endif()

    string(JSON _matti_package_count ERROR_VARIABLE _matti_error LENGTH "${_matti_metadata}" packages)
    if(_matti_error OR _matti_package_count LESS 1)
        set(${out_var} "" PARENT_SCOPE)
        return()
    endif()

    string(JSON _matti_target_count LENGTH "${_matti_metadata}" packages 0 targets)
    if(_matti_target_count GREATER 0)
        math(EXPR _matti_last_target "${_matti_target_count} - 1")
        foreach(_matti_index RANGE ${_matti_last_target})
            string(JSON _matti_kind_count LENGTH "${_matti_metadata}" packages 0 targets ${_matti_index} kind)
            set(_matti_is_bin FALSE)
            if(_matti_kind_count GREATER 0)
                math(EXPR _matti_last_kind "${_matti_kind_count} - 1")
                foreach(_matti_kind_index RANGE ${_matti_last_kind})
                    string(JSON _matti_kind GET "${_matti_metadata}" packages 0 targets
                            ${_matti_index} kind ${_matti_kind_index})
                    if(_matti_kind STREQUAL "bin")
                        set(_matti_is_bin TRUE)
                    endif()
                endforeach()
            endif()

            if(_matti_is_bin)
                string(JSON _matti_name GET "${_matti_metadata}" packages 0 targets ${_matti_index} name)
                list(APPEND _matti_bins "${_matti_name}")
            endif()
        endforeach()
    endif()

    set(${out_var} "${_matti_bins}" PARENT_SCOPE)
endfunction()

# matti_add_rust_staticlib(<target>
#     MANIFEST <path to Cargo.toml>
#     LIB_NAME <name of the staticlib>
#     [TARGET_DIR <cargo target dir>]   default: ${CMAKE_CURRENT_BINARY_DIR}/cargo
#     [PROFILE <dev|release>]           default: release for Release builds
# )
#
# Creates an imported STATIC target <target> that depends on a custom target
# which runs cargo, so linking it builds the crate first. Every built artifact
# is recorded in the MATTI_RUST_ARTIFACTS property for
# matti_embed_rust_artifacts().
function(matti_add_rust_staticlib target)
    cmake_parse_arguments(ARG "" "MANIFEST;LIB_NAME;TARGET_DIR;PROFILE" "" ${ARGN})

    if(NOT ARG_MANIFEST)
        message(FATAL_ERROR "matti_add_rust_staticlib(${target}): MANIFEST is required")
    endif()
    if(NOT ARG_LIB_NAME)
        message(FATAL_ERROR "matti_add_rust_staticlib(${target}): LIB_NAME is required")
    endif()
    if(NOT ARG_TARGET_DIR)
        set(ARG_TARGET_DIR "${CMAKE_CURRENT_BINARY_DIR}/cargo")
    endif()

    if(NOT MATTI_CARGO)
        message(FATAL_ERROR
                "cargo not found - the Rust part of libmatti needs a Rust toolchain.\n"
                "Install one (NixOS: import nix/home.nix, it provides rustc and cargo)\n"
                "or point CMake at it with -DMATTI_CARGO=<path to cargo>.")
    endif()

    if(ARG_PROFILE)
        set(_matti_rust_profile "${ARG_PROFILE}")
    elseif(CMAKE_BUILD_TYPE MATCHES "^(Release|RelWithDebInfo|MinSizeRel)$")
        set(_matti_rust_profile "release")
    else()
        set(_matti_rust_profile "dev")
    endif()

    # cargo's output folder is named after the profile ("dev" lands in debug/).
    if(_matti_rust_profile STREQUAL "release")
        set(_matti_rust_profile_dir "release")
    else()
        set(_matti_rust_profile_dir "debug")
    endif()

    set(_matti_cargo_target "")
    set(_matti_cargo_env "CARGO_TARGET_DIR=${ARG_TARGET_DIR}")
    if(ANDROID)
        if(CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
            set(_matti_cargo_target "aarch64-linux-android")
        elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "x86_64")
            set(_matti_cargo_target "x86_64-linux-android")
        else()
            message(FATAL_ERROR
                    "matti_add_rust_staticlib(${target}): unsupported Android ABI ${CMAKE_ANDROID_ARCH_ABI}")
        endif()
    elseif(WIN32 AND CMAKE_CROSSCOMPILING)
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)$")
            set(_matti_cargo_target "aarch64-pc-windows-gnullvm")
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86|i[3-6]86)$")
            set(_matti_cargo_target "i686-pc-windows-gnu")
        else()
            set(_matti_cargo_target "x86_64-pc-windows-gnu")
        endif()
    endif()

    set(_matti_cargo_target_args "")
    if(_matti_cargo_target)
        set(_matti_rust_profile_dir "${_matti_cargo_target}/${_matti_rust_profile_dir}")
        set(_matti_cargo_target_args --target "${_matti_cargo_target}")
        string(TOUPPER "${_matti_cargo_target}" _matti_target_env)
        string(REPLACE "-" "_" _matti_target_env "${_matti_target_env}")
        list(APPEND _matti_cargo_env
                "CARGO_TARGET_${_matti_target_env}_LINKER=${CMAKE_C_COMPILER}")
        if(CMAKE_AR)
            list(APPEND _matti_cargo_env
                    "CARGO_TARGET_${_matti_target_env}_AR=${CMAKE_AR}")
        endif()
    endif()

    # MSVC writes <name>.lib without the "lib" prefix. MinGW and the GNU
    # Windows targets use the usual lib<name>.a.
    if(WIN32 AND NOT _matti_cargo_target MATCHES "windows-gnu")
        set(_matti_rust_lib
                "${ARG_TARGET_DIR}/${_matti_rust_profile_dir}/${ARG_LIB_NAME}.lib")
    else()
        set(_matti_rust_lib
                "${ARG_TARGET_DIR}/${_matti_rust_profile_dir}/${CMAKE_STATIC_LIBRARY_PREFIX}${ARG_LIB_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}")
    endif()

    # The crate's binaries: cargo builds them with the library, and each one is
    # shipped inside the executable (matti_embed_rust_artifacts).
    matti_rust_bin_names("${ARG_MANIFEST}" _matti_rust_bins)
    set(_matti_rust_bin_paths "")
    foreach(_matti_rust_bin IN LISTS _matti_rust_bins)
        list(APPEND _matti_rust_bin_paths
                "${ARG_TARGET_DIR}/${_matti_rust_profile_dir}/${_matti_rust_bin}${CMAKE_EXECUTABLE_SUFFIX}")
    endforeach()

    get_filename_component(_matti_rust_crate_dir "${ARG_MANIFEST}" DIRECTORY)
    file(GLOB_RECURSE _matti_rust_sources
            CONFIGURE_DEPENDS
            "${_matti_rust_crate_dir}/src/*.rs")
    list(APPEND _matti_rust_sources "${ARG_MANIFEST}")
    if(EXISTS "${_matti_rust_crate_dir}/Cargo.lock")
        list(APPEND _matti_rust_sources "${_matti_rust_crate_dir}/Cargo.lock")
    endif()

    add_custom_command(
            OUTPUT "${_matti_rust_lib}" ${_matti_rust_bin_paths}
            COMMAND ${CMAKE_COMMAND} -E env
                    ${_matti_cargo_env}
                    "${MATTI_CARGO}" build
                    --manifest-path "${ARG_MANIFEST}"
                    --profile "${_matti_rust_profile}"
                    ${_matti_cargo_target_args}
            DEPENDS ${_matti_rust_sources}
            COMMENT "Building Rust crate ${ARG_LIB_NAME} (${_matti_rust_profile})"
            VERBATIM)

    add_custom_target(${target}_build DEPENDS "${_matti_rust_lib}" ${_matti_rust_bin_paths})

    add_library(${target} STATIC IMPORTED GLOBAL)
    set_target_properties(${target} PROPERTIES
            IMPORTED_LOCATION "${_matti_rust_lib}")
    add_dependencies(${target} ${target}_build)

    # Every artifact that travels with the client: the staticlib, an rlib or
    # cdylib when the crate is configured for one, and each binary.
    set(_matti_rust_artifacts "${_matti_rust_lib}" ${_matti_rust_bin_paths})
    file(GLOB _matti_rust_lib_variants
            "${ARG_TARGET_DIR}/${_matti_rust_profile_dir}/lib${ARG_LIB_NAME}.*")
    foreach(_matti_rust_variant IN LISTS _matti_rust_lib_variants)
        if(_matti_rust_variant MATCHES "\\.(a|rlib|so|dylib|dll)$")
            list(APPEND _matti_rust_artifacts "${_matti_rust_variant}")
        endif()
    endforeach()
    list(REMOVE_DUPLICATES _matti_rust_artifacts)
    set_target_properties(${target} PROPERTIES
            MATTI_RUST_ARTIFACTS "${_matti_rust_artifacts}")
endfunction()

# matti_embed_rust_artifacts(<executable> <crate-target> <section-prefix>)
#
# Embeds every artifact of the crate into <executable> as its own ELF section:
#
#     <section-prefix>_<sanitized file name>
#
# The Rust code itself is linked through the staticlib; the sections make the
# binaries and libraries part of the shipped executable so nothing is missing
# from the artefact.
function(matti_embed_rust_artifacts executable crate_target section_prefix)
    if(APPLE OR WIN32)
        return()
    endif()

    get_target_property(_matti_artifacts ${crate_target} MATTI_RUST_ARTIFACTS)
    if(NOT _matti_artifacts)
        return()
    endif()

    find_program(MATTI_OBJCOPY_EXE NAMES objcopy)
    if(NOT MATTI_OBJCOPY_EXE)
        message(WARNING "objcopy not found - Rust artefacts are not embedded into ${executable}")
        return()
    endif()

    # The artefacts exist only after cargo ran, so the executable waits for the
    # crate target before its POST_BUILD step looks at them.
    add_dependencies(${executable} ${crate_target}_build)

    foreach(_matti_artifact IN LISTS _matti_artifacts)
        get_filename_component(_matti_artifact_name "${_matti_artifact}" NAME)
        string(MAKE_C_IDENTIFIER "${_matti_artifact_name}" _matti_entry)
        set(_matti_section "${section_prefix}_${_matti_entry}")

        add_custom_command(TARGET ${executable} POST_BUILD
                COMMAND "${MATTI_OBJCOPY_EXE}"
                        --add-section "${_matti_section}=${_matti_artifact}"
                        --set-section-flags "${_matti_section}=contents,readonly"
                        "$<TARGET_FILE:${executable}>"
                COMMENT "Embedding Rust artefact ${_matti_artifact_name} into ${executable} (${_matti_section})"
                VERBATIM)
    endforeach()
endfunction()
