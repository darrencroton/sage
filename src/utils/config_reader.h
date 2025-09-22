/**
 * @file    config_reader.h
 * @brief   Modern YAML configuration reader for SAGE
 *
 * This module provides the modern YAML-based configuration system for SAGE.
 * It replaces the legacy .par format with a more structured, human-readable
 * YAML configuration format that supports nested sections and enhanced
 * validation.
 *
 * Key features:
 * - YAML-based configuration with nested sections
 * - Comprehensive parameter validation
 * - Clean, structured configuration format
 * - Preparation for module-aware configuration
 * - Enhanced error reporting and debugging
 *
 * This completely replaces the legacy parameter file reading system.
 */

#ifndef CONFIG_READER_H
#define CONFIG_READER_H

#include "constants.h"
#include "types.h"

// Configuration reading result codes
typedef enum {
    CONFIG_SUCCESS = 0,
    CONFIG_FILE_NOT_FOUND,
    CONFIG_PARSE_ERROR,
    CONFIG_VALIDATION_ERROR,
    CONFIG_MEMORY_ERROR,
    CONFIG_YAML_NOT_AVAILABLE,
    CONFIG_UNKNOWN_ERROR
} config_result_t;

// Enhanced configuration structure (extends SageConfig)
typedef struct {
    // Base SageConfig structure
    struct SageConfig base;

    // Additional metadata
    char source_file[MAX_STRING_LEN]; // Source file path

    // Future extensibility for module configuration
    // (to be implemented in later phases)
    void *module_config;             // Reserved for future use
    int num_modules;                 // Reserved for future use
} config_t;

// Function declarations

/**
 * @brief Initialize configuration structure
 * @param config Pointer to config_t structure to initialize
 * @return CONFIG_SUCCESS on success, error code otherwise
 */
config_result_t config_init(config_t *config);

/**
 * @brief Free resources allocated for configuration
 * @param config Pointer to config_t structure to free
 */
void config_free(config_t *config);

/**
 * @brief Read YAML configuration from file
 * @param filename Path to YAML configuration file (.yaml or .yml)
 * @param config Pointer to config_t structure to populate
 * @return CONFIG_SUCCESS on success, error code otherwise
 */
config_result_t config_read_file(const char *filename, config_t *config);

/**
 * @brief Validate configuration structure
 * @param config Pointer to config_t structure to validate
 * @return CONFIG_SUCCESS if valid, error code otherwise
 */
config_result_t config_validate(const config_t *config);

/**
 * @brief Convert configuration result to human-readable string
 * @param result Configuration result code
 * @return Constant string describing the result
 */
const char *config_result_string(config_result_t result);

/**
 * @brief Print configuration summary
 * @param config Pointer to config_t structure to print
 */
void config_print_summary(const config_t *config);

// Internal helper functions

/**
 * @brief Copy SageConfig structure
 * @param dest Destination SageConfig structure
 * @param src Source SageConfig structure
 */
void config_copy_sage_config(struct SageConfig *dest, const struct SageConfig *src);

/**
 * @brief Apply configuration defaults
 * @param config Pointer to config_t structure
 */
void config_apply_defaults(config_t *config);

/**
 * @brief Synchronize global variables with configuration
 * @param config Pointer to config_t structure
 *
 * This function updates global variables to maintain backward compatibility
 * with code that still accesses globals directly.
 */
void config_sync_globals(const config_t *config);

#endif /* CONFIG_READER_H */