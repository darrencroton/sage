# SAGE CMake Build System
# Replaces legacy Makefile with modern CMake infrastructure
# Maintains full compatibility with existing build configurations

cmake_minimum_required(VERSION 3.12)
project(SAGE LANGUAGES C)

# Set C standard and compiler requirements
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Build configuration - match Makefile exactly
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -Wall")
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -Wall")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG -Wall")

# Default to Debug build (matches Makefile's -g -O0)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose build type" FORCE)
endif()

# Source files - reorganized into logical directory structure
set(SAGE_CORE_SOURCES
    src/core/main.c
    src/utils/util_parameters.c
    src/utils/util_error.c
    src/utils/util_integration.c
    src/utils/util_numeric.c
    src/utils/util_version.c
    src/io/io_util.c
    src/core/core_read_parameter_file.c
    src/core/core_init.c
    src/io/io_tree.c
    src/core/core_cool_func.c
    src/core/core_build_model.c
    src/io/io_save_binary.c
    src/utils/util_memory.c
    src/core/core_allvars.c
    src/core/core_simulation_state.c
    src/physics/model_infall.c
    src/physics/model_cooling_heating.c
    src/physics/model_starformation_and_feedback.c
    src/physics/model_disk_instability.c
    src/physics/model_reincorporation.c
    src/physics/model_mergers.c
    src/physics/model_misc.c
    src/io/io_tree_binary.c
)

# Header files - reorganized into logical directory structure
set(SAGE_HEADERS
    src/core/core_allvars.h
    src/io/io_tree.h
    src/io/io_save_binary.h
    src/utils/util_memory.h
    src/utils/util_integration.h
    src/utils/util_numeric.h
    src/utils/util_version.h
    src/io/io_util.h
    src/core/core_proto.h
    src/core/core_simulation.h
    src/utils/util_parameters.h
    src/utils/util_error.h
    src/core/config.h
    src/core/constants.h
    src/core/globals.h
    src/core/types.h
    src/io/io_tree_binary.h
)

# Git version tracking - replaces Makefile git version generation
set(GIT_VERSION_IN "${CMAKE_SOURCE_DIR}/src/core/git_version.h.in")
set(GIT_VERSION_H "${CMAKE_SOURCE_DIR}/src/core/git_version.h")

# Generate git version header
find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_BRANCH_NAME
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
else()
    set(GIT_COMMIT_HASH "unknown")
    set(GIT_BRANCH_NAME "unknown")
endif()

# Configure git version header
configure_file(${GIT_VERSION_IN} ${GIT_VERSION_H} @ONLY)

# Optional MPI support
option(USE_MPI "Enable MPI support for parallel processing" OFF)
if(USE_MPI)
    find_package(MPI REQUIRED COMPONENTS C)
    if(MPI_C_FOUND)
        add_compile_definitions(MPI)
        message(STATUS "MPI support enabled")
    else()
        message(FATAL_ERROR "MPI requested but not found")
    endif()
endif()

# Optional HDF5 support
option(USE_HDF5 "Enable HDF5 support for tree format" OFF)
if(USE_HDF5)
    # Try system HDF5 first
    find_package(HDF5 COMPONENTS C)

    if(NOT HDF5_FOUND)
        # Fallback to Makefile's hardcoded path if system HDF5 not found
        set(HDF5_ROOT "/usr/local/x86_64/gnu/hdf5-1.8.17-openmpi-1.10.2-psm")
        find_package(HDF5 COMPONENTS C)
    endif()

    if(HDF5_FOUND)
        add_compile_definitions(HDF5)
        # Add HDF5-specific sources (from Makefile conditional OBJS)
        list(APPEND SAGE_CORE_SOURCES
            src/io/io_tree_hdf5.c
            src/io/io_save_hdf5.c)
        list(APPEND SAGE_HEADERS
            src/io/io_tree_hdf5.h
            src/io/io_save_hdf5.h)
        message(STATUS "HDF5 support enabled")
        message(STATUS "HDF5 version: ${HDF5_VERSION}")
        message(STATUS "HDF5 libraries: ${HDF5_C_LIBRARIES}")
    else()
        message(FATAL_ERROR "HDF5 requested but not found")
    endif()
endif()

# Create executable target
add_executable(sage ${SAGE_CORE_SOURCES})

# Move the executable to the source directory
# SAGE must be run from the source directory where parameter file paths are expected
add_custom_command(TARGET sage POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rename $<TARGET_FILE:sage> ${CMAKE_SOURCE_DIR}/sage
    COMMENT "Moving sage executable to source directory"
)

# Link math library (from Makefile LIBS)
target_link_libraries(sage m)

# Link MPI if enabled
if(USE_MPI AND MPI_C_FOUND)
    target_link_libraries(sage MPI::MPI_C)
    set_target_properties(sage PROPERTIES
        COMPILE_FLAGS "${MPI_C_COMPILE_FLAGS}"
        LINK_FLAGS "${MPI_C_LINK_FLAGS}")
endif()

# Link HDF5 if enabled
if(USE_HDF5 AND HDF5_FOUND)
    target_link_libraries(sage ${HDF5_C_LIBRARIES})
    target_include_directories(sage PRIVATE ${HDF5_C_INCLUDE_DIRS})
    target_compile_definitions(sage PRIVATE ${HDF5_C_DEFINITIONS})
endif()

# Include directories - updated for new structure
target_include_directories(sage PRIVATE src/core src/io src/physics src/utils)

# Ensure git version header is treated as dependency
add_custom_target(git_version_header ALL
    COMMAND ${CMAKE_COMMAND} -E echo "Updating git version header"
    DEPENDS ${GIT_VERSION_H}
)
add_dependencies(sage git_version_header)

# Make git version header generation always run (like Makefile .PHONY)
# This ensures git information is current on every build
set_source_files_properties(${GIT_VERSION_H} PROPERTIES GENERATED TRUE)
add_custom_target(git_version_always
    COMMAND ${CMAKE_COMMAND} -E echo "Generating Git version header"
    COMMAND ${CMAKE_COMMAND} -E echo "Git commit: ${GIT_COMMIT_HASH}"
    COMMAND ${CMAKE_COMMAND} -E echo "Git branch: ${GIT_BRANCH_NAME}"
    DEPENDS ${GIT_VERSION_IN}
    BYPRODUCTS ${GIT_VERSION_H}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
add_dependencies(sage git_version_always)


# Print configuration summary
message(STATUS "")
message(STATUS "SAGE Build Configuration:")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "  C Flags: ${CMAKE_C_FLAGS}")
message(STATUS "  MPI Support: ${USE_MPI}")
message(STATUS "  HDF5 Support: ${USE_HDF5}")
message(STATUS "  Source files: ${SAGE_CORE_SOURCES}")
message(STATUS "")