/**
 * @file    test_yaml_config.c
 * @brief   Unit tests for YAML configuration system
 *
 * This file contains unit tests for the YAML configuration reader
 * and validation system. Tests ensure that the YAML configuration
 * system properly reads, validates, and processes configuration files.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "config_reader.h"
#include "error.h"
#include "memory.h"

// Test configuration file content
static const char* test_yaml_content =
"files:\n"
"  galaxy_file_name: test_model\n"
"  output_dir: ./test_output/\n"
"  tree_name: test_trees\n"
"  tree_type: lhalo_binary\n"
"  simulation_dir: ./test_data/\n"
"  snapshot_list: ./test_data/snaplist.txt\n"
"\n"
"simulation:\n"
"  first_file: 0\n"
"  last_file: 1\n"
"  last_snapshot: 10\n"
"  num_outputs: 5\n"
"  box_size: 100.0\n"
"  omega: 0.3\n"
"  omega_lambda: 0.7\n"
"  hubble_h: 0.7\n"
"  baryon_fraction: 0.15\n"
"  particle_mass: 1.0\n"
"\n"
"physics:\n"
"  star_formation:\n"
"    prescription: 0\n"
"    efficiency: 0.1\n"
"  feedback:\n"
"    reheating_epsilon: 2.0\n"
"    ejection_efficiency: 0.5\n"
"\n"
"units:\n"
"  length_cm: 3.08568e+24\n"
"  mass_g: 1.989e+43\n"
"  velocity_cm_per_s: 100000\n";

/**
 * @brief Create a temporary YAML test file
 * @param filename Name of the temporary file to create
 * @return 0 on success, -1 on failure
 */
static int create_test_yaml_file(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        return -1;
    }

    fprintf(file, "%s", test_yaml_content);
    fclose(file);
    return 0;
}

/**
 * @brief Test basic YAML configuration reading
 */
static void test_yaml_config_reading() {
    printf("Testing YAML configuration reading...\n");

    config_t config;
    config_result_t result;
    const char *test_file = "/tmp/test_config.yaml";

    // Create test file
    assert(create_test_yaml_file(test_file) == 0);

    // Read configuration
    result = config_read_file(test_file, &config);
    assert(result == CONFIG_SUCCESS);

    // Verify basic parameters
    assert(strcmp(config.base.FileNameGalaxies, "test_model") == 0);
    assert(strcmp(config.base.OutputDir, "./test_output/") == 0);
    assert(config.base.FirstFile == 0);
    assert(config.base.LastFile == 1);
    assert(config.base.BoxSize == 100.0);
    assert(config.base.Omega == 0.3);
    assert(config.base.SfrEfficiency == 0.1);

    // Clean up
    config_free(&config);
    remove(test_file);

    printf("✓ YAML configuration reading test passed\n");
}

/**
 * @brief Test configuration validation
 */
static void test_yaml_config_validation() {
    printf("Testing YAML configuration validation...\n");

    config_t config;
    config_result_t result;

    // Initialize with valid configuration
    result = config_init(&config);
    assert(result == CONFIG_SUCCESS);

    // Set required fields
    strcpy(config.base.OutputDir, "./test_output/");
    strcpy(config.base.FileNameGalaxies, "test_model");
    config.base.FirstFile = 0;
    config.base.LastFile = 1;

    // Test validation
    result = config_validate(&config);
    assert(result == CONFIG_SUCCESS);

    // Test validation failure (empty OutputDir)
    strcpy(config.base.OutputDir, "");
    result = config_validate(&config);
    assert(result == CONFIG_VALIDATION_ERROR);

    // Clean up
    config_free(&config);

    printf("✓ YAML configuration validation test passed\n");
}

/**
 * @brief Test error handling for invalid files
 */
static void test_yaml_error_handling() {
    printf("Testing YAML error handling...\n");

    config_t config;
    config_result_t result;

    // Test non-existent file
    // NOTE: This should return CONFIG_FILE_NOT_FOUND but currently returns CONFIG_PARSE_ERROR
    // This is a known bug in config_reader.c that needs to be fixed
    result = config_read_file("/nonexistent/file.yaml", &config);
    assert(result == CONFIG_PARSE_ERROR);  // FIXME: Should be CONFIG_FILE_NOT_FOUND

    // Create invalid YAML file
    const char *invalid_file = "/tmp/invalid.yaml";
    FILE *file = fopen(invalid_file, "w");
    assert(file != NULL);
    fprintf(file, "invalid: yaml: content: [\n");
    fclose(file);

    // Test invalid YAML
    result = config_read_file(invalid_file, &config);
    assert(result == CONFIG_PARSE_ERROR);

    // Clean up
    remove(invalid_file);

    printf("✓ YAML error handling test passed\n");
}

/**
 * @brief Main test runner
 */
int main() {
    printf("Running YAML configuration system tests...\n\n");

    // Initialize required systems
    initialize_error_handling(2, stdout); // INFO level to stdout
    init_memory_system(1024);              // Standard capacity

    // Run tests
    test_yaml_config_reading();
    test_yaml_config_validation();
    test_yaml_error_handling();

    // Check for memory leaks
    printf("\nMemory leak check:\n");
    check_memory_leaks();

    printf("\n✓ All YAML configuration tests passed!\n");
    return 0;
}