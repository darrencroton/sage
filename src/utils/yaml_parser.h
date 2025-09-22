/**
 * @file    yaml_parser.h
 * @brief   YAML parsing utilities for SAGE configuration system
 *
 * This module provides utilities for parsing YAML configuration files
 * as part of the modern configuration abstraction layer. It wraps the
 * libyaml library to provide high-level parsing functions specifically
 * designed for SAGE's configuration needs.
 *
 * Key features:
 * - Parse YAML files into key-value pairs
 * - Support for nested configuration sections
 * - Error handling and validation
 * - Memory-safe parsing with automatic cleanup
 *
 * This module requires libyaml and is essential for SAGE's modern
 * configuration system.
 */

#ifndef YAML_PARSER_H
#define YAML_PARSER_H

#include <yaml.h>
#include "constants.h"

// Maximum nesting depth for YAML parsing
#define YAML_MAX_DEPTH 8

// YAML parsing result codes
typedef enum {
    YAML_PARSE_SUCCESS = 0,
    YAML_PARSE_FILE_NOT_FOUND,
    YAML_PARSE_INVALID_SYNTAX,
    YAML_PARSE_MEMORY_ERROR,
    YAML_PARSE_DEPTH_EXCEEDED,
    YAML_PARSE_UNKNOWN_ERROR
} yaml_parse_result_t;

// Structure to hold a key-value pair from YAML
typedef struct {
    char key[MAX_STRING_LEN];
    char value[MAX_STRING_LEN];
    char section[MAX_STRING_LEN]; // For nested sections like "simulation.omega"
} yaml_keyvalue_t;

// Structure to hold all parsed YAML data
typedef struct {
    yaml_keyvalue_t *pairs;
    int count;
    int capacity;
} yaml_data_t;

// Function declarations

/**
 * @brief Initialize YAML data structure
 * @param data Pointer to yaml_data_t structure to initialize
 * @return YAML_PARSE_SUCCESS on success, error code otherwise
 */
yaml_parse_result_t yaml_data_init(yaml_data_t *data);

/**
 * @brief Free memory allocated for YAML data
 * @param data Pointer to yaml_data_t structure to free
 */
void yaml_data_free(yaml_data_t *data);

/**
 * @brief Parse YAML file into key-value pairs
 * @param filename Path to YAML file to parse
 * @param data Pointer to yaml_data_t structure to store results
 * @return YAML_PARSE_SUCCESS on success, error code otherwise
 */
yaml_parse_result_t yaml_parse_file(const char *filename, yaml_data_t *data);

/**
 * @brief Get value for a given key from parsed YAML data
 * @param data Pointer to yaml_data_t structure
 * @param key Key to search for (can include section, e.g., "simulation.omega")
 * @param value Buffer to store the value (must be at least MAX_STRING_LEN)
 * @return 1 if key found, 0 otherwise
 */
int yaml_get_value(const yaml_data_t *data, const char *key, char *value);

/**
 * @brief Parse integer array from YAML data
 * @param data Pointer to yaml_data_t structure
 * @param key Key to search for array (e.g., "simulation.output_snapshots")
 * @param array Buffer to store parsed integers
 * @param max_size Maximum number of integers to parse
 * @return Number of integers parsed, or -1 on error
 */
int yaml_get_int_array(const yaml_data_t *data, const char *key, int *array, int max_size);

/**
 * @brief Validate YAML structure against expected schema
 * @param data Pointer to yaml_data_t structure
 * @return 1 if valid, 0 otherwise
 */
int yaml_validate_structure(const yaml_data_t *data);

/**
 * @brief Convert YAML parse result to human-readable string
 * @param result YAML parse result code
 * @return Constant string describing the result
 */
const char *yaml_parse_result_string(yaml_parse_result_t result);

// Internal helper functions (not part of public API)

/**
 * @brief Add a key-value pair to the YAML data structure
 * @param data Pointer to yaml_data_t structure
 * @param key Key string
 * @param value Value string
 * @param section Current section path
 * @return YAML_PARSE_SUCCESS on success, error code otherwise
 */
yaml_parse_result_t yaml_data_add_pair(yaml_data_t *data, const char *key,
                                      const char *value, const char *section);

/**
 * @brief Build section path from current parsing context
 * @param sections Array of section names
 * @param depth Current nesting depth
 * @param section_path Buffer to store the built path
 */
void yaml_build_section_path(const char sections[][MAX_STRING_LEN],
                            int depth, char *section_path);

#endif /* YAML_PARSER_H */