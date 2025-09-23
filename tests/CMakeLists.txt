# Test configuration for SAGE
cmake_minimum_required(VERSION 3.12)

# Test configuration
set(TEST_TIMEOUT 60)  # Increased timeout for more complex tests

# Common test utilities and sources
set(TEST_COMMON_SOURCES
    test_utils.c           # Test-specific utilities (includes myexit)
    ../src/utils/error.c
    ../src/utils/memory.c
    ../src/core/globals.c
)

# YAML-specific test sources (only if YAML is available)
set(YAML_TEST_SOURCES
    ../src/utils/config_reader.c
    ../src/utils/yaml_parser.c
)

# Function to create a test executable with common setup
function(add_sage_test test_name test_source)
    add_executable(${test_name}
        ${test_source}
        ${TEST_COMMON_SOURCES}
        ${ARGN}  # Additional sources
    )

    # Include directories for all tests
    target_include_directories(${test_name} PRIVATE
        ../src/core
        ../src/utils
        ../src/io
        ../src/physics
    )

    # Link math library
    target_link_libraries(${test_name} m)

    # Add test to CTest
    add_test(NAME ${test_name} COMMAND ${test_name})

    # Set common test properties
    set_tests_properties(${test_name} PROPERTIES
        TIMEOUT ${TEST_TIMEOUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endfunction()

# YAML Configuration Tests (only if YAML is available)
if(YAML_FOUND)
    add_sage_test(test_yaml_config test_yaml_config.c ${YAML_TEST_SOURCES})

    # Link YAML libraries to YAML-specific tests
    target_link_directories(test_yaml_config PRIVATE ${YAML_LIBRARY_DIRS})
    target_link_libraries(test_yaml_config ${YAML_LIBRARIES})
    target_include_directories(test_yaml_config PRIVATE ${YAML_INCLUDE_DIRS})

    message(STATUS "YAML configuration tests enabled")
else()
    message(STATUS "YAML configuration tests skipped (YAML not found)")
endif()

# Numeric Utility Tests
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_numeric.c")
    add_sage_test(test_numeric test_numeric.c ../src/utils/numeric.c)
    message(STATUS "Numeric utility tests enabled")
endif()

# Memory Management Tests
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_memory.c")
    add_sage_test(test_memory test_memory.c)
    message(STATUS "Memory management tests enabled")
endif()

# Physics Module Interface Tests
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_physics_module.c")
    add_sage_test(test_physics_module test_physics_module.c ../src/core/physics_module.c)
    message(STATUS "Physics module interface tests enabled")
endif()

# Print test summary
get_property(all_tests DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY TESTS)
if(all_tests)
    message(STATUS "Enabled tests: ${all_tests}")
else()
    message(STATUS "No tests found to enable")
endif()