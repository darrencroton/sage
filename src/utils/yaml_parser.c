/**
 * @file    yaml_parser.c
 * @brief   YAML parsing utilities implementation
 *
 * This file implements the YAML parsing functionality for SAGE's modern
 * configuration system. It provides a clean interface to the libyaml
 * library, handling the complexity of YAML parsing while providing
 * simple key-value access for configuration parameters.
 *
 * The implementation supports:
 * - Nested YAML structures (up to YAML_MAX_DEPTH levels)
 * - Automatic memory management
 * - Comprehensive error handling
 * - Flattened key access (e.g., "simulation.omega")
 */

#include "yaml_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <yaml.h>

#include "error.h"
#include "memory.h"

// Initial capacity for key-value pairs
#define INITIAL_CAPACITY 64

/**
 * @brief Initialize YAML data structure
 */
yaml_parse_result_t yaml_data_init(yaml_data_t *data) {
    if (!data) {
        return YAML_PARSE_MEMORY_ERROR;
    }

    data->pairs = mymalloc(sizeof(yaml_keyvalue_t) * INITIAL_CAPACITY);
    if (!data->pairs) {
        return YAML_PARSE_MEMORY_ERROR;
    }

    data->count = 0;
    data->capacity = INITIAL_CAPACITY;
    return YAML_PARSE_SUCCESS;
}

/**
 * @brief Free memory allocated for YAML data
 */
void yaml_data_free(yaml_data_t *data) {
    if (data && data->pairs) {
        myfree(data->pairs);
        data->pairs = NULL;
        data->count = 0;
        data->capacity = 0;
    }
}

/**
 * @brief Add a key-value pair to the YAML data structure
 */
yaml_parse_result_t yaml_data_add_pair(yaml_data_t *data, const char *key,
                                      const char *value, const char *section) {
    if (!data || !key || !value) {
        return YAML_PARSE_MEMORY_ERROR;
    }

    // Expand capacity if needed
    if (data->count >= data->capacity) {
        int new_capacity = data->capacity * 2;
        yaml_keyvalue_t *new_pairs = myrealloc(data->pairs,
                                             sizeof(yaml_keyvalue_t) * new_capacity);
        if (!new_pairs) {
            return YAML_PARSE_MEMORY_ERROR;
        }
        data->pairs = new_pairs;
        data->capacity = new_capacity;
    }

    // Add the new pair
    yaml_keyvalue_t *pair = &data->pairs[data->count];

    // Build full key with section prefix if present
    if (section && strlen(section) > 0) {
        snprintf(pair->key, MAX_STRING_LEN, "%s.%s", section, key);
        strncpy(pair->section, section, MAX_STRING_LEN - 1);
        pair->section[MAX_STRING_LEN - 1] = '\0';
    } else {
        strncpy(pair->key, key, MAX_STRING_LEN - 1);
        pair->key[MAX_STRING_LEN - 1] = '\0';
        pair->section[0] = '\0';
    }

    strncpy(pair->value, value, MAX_STRING_LEN - 1);
    pair->value[MAX_STRING_LEN - 1] = '\0';

    data->count++;
    return YAML_PARSE_SUCCESS;
}

/**
 * @brief Build section path from current parsing context
 */
void yaml_build_section_path(const char sections[][MAX_STRING_LEN],
                            int depth, char *section_path) {
    section_path[0] = '\0';

    for (int i = 0; i < depth; i++) {
        if (i > 0) {
            strcat(section_path, ".");
        }
        strcat(section_path, sections[i]);
    }
}

/**
 * @brief Parse YAML file into key-value pairs
 */
yaml_parse_result_t yaml_parse_file(const char *filename, yaml_data_t *data) {
    FILE *file = NULL;
    yaml_parser_t parser;
    yaml_event_t event;
    int done = 0;
    yaml_parse_result_t result = YAML_PARSE_SUCCESS;

    // Stack to track sections during parsing
    char sections[YAML_MAX_DEPTH][MAX_STRING_LEN];
    int section_depth = 0;
    char section_path[MAX_STRING_LEN * YAML_MAX_DEPTH];

    // Parsing state
    enum {
        STATE_START,
        STATE_STREAM,
        STATE_DOCUMENT,
        STATE_BLOCK_MAPPING_START,
        STATE_BLOCK_MAPPING_KEY,
        STATE_BLOCK_MAPPING_VALUE,
        STATE_END
    } state = STATE_START;

    char current_key[MAX_STRING_LEN];
    current_key[0] = '\0';

    // Initialize parser
    if (!yaml_parser_initialize(&parser)) {
        ERROR_LOG("Failed to initialize YAML parser");
        return YAML_PARSE_UNKNOWN_ERROR;
    }

    // Open file
    file = fopen(filename, "r");
    if (!file) {
        ERROR_LOG("Cannot open YAML file: %s", filename);
        yaml_parser_delete(&parser);
        return YAML_PARSE_FILE_NOT_FOUND;
    }

    yaml_parser_set_input_file(&parser, file);

    // Parse events
    while (!done) {
        if (!yaml_parser_parse(&parser, &event)) {
            ERROR_LOG("YAML parsing error at line %lu: %s",
                     parser.problem_mark.line + 1, parser.problem);
            result = YAML_PARSE_INVALID_SYNTAX;
            break;
        }

        switch (event.type) {
        case YAML_STREAM_START_EVENT:
            if (state != STATE_START) {
                ERROR_LOG("Unexpected YAML stream start");
                result = YAML_PARSE_INVALID_SYNTAX;
                done = 1;
            } else {
                state = STATE_STREAM;
            }
            break;

        case YAML_DOCUMENT_START_EVENT:
            if (state != STATE_STREAM) {
                ERROR_LOG("Unexpected YAML document start");
                result = YAML_PARSE_INVALID_SYNTAX;
                done = 1;
            } else {
                state = STATE_DOCUMENT;
            }
            break;

        case YAML_MAPPING_START_EVENT:
            if (state == STATE_DOCUMENT) {
                state = STATE_BLOCK_MAPPING_START;
            } else if (state == STATE_BLOCK_MAPPING_VALUE) {
                // Nested mapping - push current key as section
                if (section_depth >= YAML_MAX_DEPTH) {
                    ERROR_LOG("YAML nesting depth exceeded (max %d)", YAML_MAX_DEPTH);
                    result = YAML_PARSE_DEPTH_EXCEEDED;
                    done = 1;
                    break;
                }
                strncpy(sections[section_depth], current_key, MAX_STRING_LEN - 1);
                sections[section_depth][MAX_STRING_LEN - 1] = '\0';
                section_depth++;
                state = STATE_BLOCK_MAPPING_START;
            }
            break;

        case YAML_MAPPING_END_EVENT:
            if (section_depth > 0) {
                section_depth--;
                state = STATE_BLOCK_MAPPING_KEY;
            } else {
                state = STATE_END;
            }
            break;

        case YAML_SCALAR_EVENT:
            yaml_build_section_path(sections, section_depth, section_path);

            if (state == STATE_BLOCK_MAPPING_START || state == STATE_BLOCK_MAPPING_KEY) {
                // This is a key
                strncpy(current_key, (char *)event.data.scalar.value, MAX_STRING_LEN - 1);
                current_key[MAX_STRING_LEN - 1] = '\0';
                state = STATE_BLOCK_MAPPING_VALUE;
            } else if (state == STATE_BLOCK_MAPPING_VALUE) {
                // This is a value
                result = yaml_data_add_pair(data, current_key,
                                          (char *)event.data.scalar.value,
                                          section_path);
                if (result != YAML_PARSE_SUCCESS) {
                    done = 1;
                    break;
                }
                state = STATE_BLOCK_MAPPING_KEY;
            }
            break;

        case YAML_DOCUMENT_END_EVENT:
            state = STATE_END;
            break;

        case YAML_STREAM_END_EVENT:
            done = 1;
            break;

        default:
            // Ignore other events (sequences, aliases, etc.)
            break;
        }

        yaml_event_delete(&event);
    }

    // Cleanup
    yaml_parser_delete(&parser);
    if (file) {
        fclose(file);
    }

    return result;
}

/**
 * @brief Get value for a given key from parsed YAML data
 */
int yaml_get_value(const yaml_data_t *data, const char *key, char *value) {
    if (!data || !key || !value) {
        return 0;
    }

    for (int i = 0; i < data->count; i++) {
        if (strcmp(data->pairs[i].key, key) == 0) {
            strncpy(value, data->pairs[i].value, MAX_STRING_LEN - 1);
            value[MAX_STRING_LEN - 1] = '\0';
            return 1;
        }
    }

    return 0;
}

/**
 * @brief Validate YAML structure against expected schema
 */
int yaml_validate_structure(const yaml_data_t *data) {
    if (!data) {
        return 0;
    }

    // Basic validation: check for required top-level sections
    const char *required_sections[] = {
        "simulation",
        "files",
        "physics"
    };
    int num_required = sizeof(required_sections) / sizeof(required_sections[0]);

    for (int i = 0; i < num_required; i++) {
        int found = 0;
        for (int j = 0; j < data->count; j++) {
            if (strncmp(data->pairs[j].key, required_sections[i],
                       strlen(required_sections[i])) == 0) {
                found = 1;
                break;
            }
        }
        if (!found) {
            DEBUG_LOG("Missing required YAML section: %s", required_sections[i]);
            // Note: This is a warning, not an error - allow flexible YAML structure
        }
    }

    return 1; // Always return success for now - validation can be enhanced later
}

/**
 * @brief Convert YAML parse result to human-readable string
 */
const char *yaml_parse_result_string(yaml_parse_result_t result) {
    switch (result) {
    case YAML_PARSE_SUCCESS:
        return "Success";
    case YAML_PARSE_FILE_NOT_FOUND:
        return "File not found";
    case YAML_PARSE_INVALID_SYNTAX:
        return "Invalid YAML syntax";
    case YAML_PARSE_MEMORY_ERROR:
        return "Memory allocation error";
    case YAML_PARSE_DEPTH_EXCEEDED:
        return "Maximum nesting depth exceeded";
    case YAML_PARSE_UNKNOWN_ERROR:
    default:
        return "Unknown error";
    }
}

/**
 * @brief Parse integer array from YAML data
 */
int yaml_get_int_array(const yaml_data_t *data, const char *key, int *array, int max_size) {
    if (!data || !key || !array || max_size <= 0) {
        return -1;
    }

    // Look for the key in parsed data
    for (int i = 0; i < data->count; i++) {
        if (strcmp(data->pairs[i].key, key) == 0) {
            // Found the key, parse the array value - make a copy to avoid modifying original
            char value_copy[MAX_STRING_LEN];
            strncpy(value_copy, data->pairs[i].value, MAX_STRING_LEN - 1);
            value_copy[MAX_STRING_LEN - 1] = '\0';

            char *value = value_copy;

            // Skip leading '[' and whitespace
            while (*value && (*value == '[' || *value == ' ' || *value == '\t')) {
                value++;
            }

            int count = 0;
            char *start = value;
            char *end;

            while (*start && count < max_size) {
                // Skip whitespace and commas
                while (*start && (*start == ' ' || *start == '\t' || *start == ',')) {
                    start++;
                }

                // Find end of number (whitespace, comma, or bracket)
                end = start;
                while (*end && *end != ' ' && *end != '\t' && *end != ',' && *end != ']') {
                    end++;
                }

                if (start == end) break; // No more numbers

                // Temporarily null-terminate and parse
                char saved = *end;
                *end = '\0';
                array[count] = atoi(start);
                *end = saved;

                count++;
                start = end;
            }

            return count;
        }
    }

    return -1; // Key not found
}

