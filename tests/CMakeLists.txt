# Test configuration for SAGE
cmake_minimum_required(VERSION 3.12)

# Enable testing
enable_testing()

# Test sources
set(TEST_COMMON_SOURCES
    ../src/utils/config_reader.c
    ../src/utils/yaml_parser.c
    ../src/utils/error.c
    ../src/utils/memory.c
    ../src/core/globals.c
)

# Test executable for YAML configuration
add_executable(test_yaml_config
    test_yaml_config.c
    ${TEST_COMMON_SOURCES}
)

# Include directories for tests
target_include_directories(test_yaml_config PRIVATE
    ../src/core
    ../src/utils
    ../src/io
    ../src/physics
)

# Link YAML libraries to tests
if(YAML_FOUND)
    target_link_directories(test_yaml_config PRIVATE ${YAML_LIBRARY_DIRS})
    target_link_libraries(test_yaml_config ${YAML_LIBRARIES})
    target_include_directories(test_yaml_config PRIVATE ${YAML_INCLUDE_DIRS})
endif()

# Add test to CTest
add_test(NAME yaml_config_test COMMAND test_yaml_config)

# Set test properties
set_tests_properties(yaml_config_test PROPERTIES
    TIMEOUT 30
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)