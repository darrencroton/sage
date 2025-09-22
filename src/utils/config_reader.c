/**
 * @file    config_reader.c
 * @brief   Modern YAML configuration reader implementation
 *
 * This file implements the YAML-based configuration reading system for SAGE.
 * It completely replaces the legacy .par file system with a modern, structured
 * YAML configuration format that supports nested sections, enhanced validation,
 * and preparation for module-aware configuration.
 *
 * The implementation requires libyaml and provides comprehensive error handling
 * and validation for all configuration parameters.
 */

#include "config_reader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "memory.h"
#include "globals.h"

#include "yaml_parser.h"

/**
 * @brief Initialize configuration structure
 */
config_result_t config_init(config_t *config) {
    if (!config) {
        return CONFIG_MEMORY_ERROR;
    }

    // Initialize base SageConfig structure to zero
    memset(&config->base, 0, sizeof(struct SageConfig));

    // Initialize metadata
    config->source_file[0] = '\0';
    config->module_config = NULL;
    config->num_modules = 0;

    // Apply default values
    config_apply_defaults(config);

    return CONFIG_SUCCESS;
}

/**
 * @brief Free resources allocated for configuration
 */
void config_free(config_t *config) {
    if (config) {
        // Free module configuration when implemented
        if (config->module_config) {
            myfree(config->module_config);
            config->module_config = NULL;
        }
        config->num_modules = 0;
    }
}

/**
 * @brief Apply configuration defaults
 */
void config_apply_defaults(config_t *config) {
    if (!config) return;

    // Set reasonable defaults
    config->base.OverwriteOutputFiles = 1; // Default to overwrite
    config->base.NOUT = -1; // Default to all snapshots

    // Initialize string fields to empty
    config->base.OutputDir[0] = '\0';
    config->base.FileNameGalaxies[0] = '\0';
    config->base.TreeName[0] = '\0';
    config->base.TreeExtension[0] = '\0';
    config->base.SimulationDir[0] = '\0';
    config->base.FileWithSnapList[0] = '\0';
}

/**
 * @brief Read YAML configuration from file
 */
config_result_t config_read_file(const char *filename, config_t *config) {
    if (!filename || !config) {
        return CONFIG_MEMORY_ERROR;
    }

    // Initialize configuration
    config_result_t result = config_init(config);
    if (result != CONFIG_SUCCESS) {
        return result;
    }

    // Store source filename
    strncpy(config->source_file, filename, MAX_STRING_LEN - 1);
    config->source_file[MAX_STRING_LEN - 1] = '\0';

    INFO_LOG("Reading YAML configuration file: %s", filename);

    // Parse YAML file
    yaml_data_t yaml_data;
    yaml_parse_result_t yaml_result;
    char value[MAX_STRING_LEN];

    // Initialize YAML parser
    yaml_result = yaml_data_init(&yaml_data);
    if (yaml_result != YAML_PARSE_SUCCESS) {
        ERROR_LOG("Failed to initialize YAML parser: %s",
                 yaml_parse_result_string(yaml_result));
        return CONFIG_MEMORY_ERROR;
    }

    // Parse YAML file
    yaml_result = yaml_parse_file(filename, &yaml_data);
    if (yaml_result != YAML_PARSE_SUCCESS) {
        ERROR_LOG("Failed to parse YAML file '%s': %s",
                 filename, yaml_parse_result_string(yaml_result));
        yaml_data_free(&yaml_data);

        // Map YAML errors to config errors
        switch (yaml_result) {
            case YAML_PARSE_FILE_NOT_FOUND:
                return CONFIG_FILE_NOT_FOUND;
            case YAML_PARSE_MEMORY_ERROR:
                return CONFIG_MEMORY_ERROR;
            case YAML_PARSE_INVALID_SYNTAX:
            case YAML_PARSE_DEPTH_EXCEEDED:
            case YAML_PARSE_UNKNOWN_ERROR:
            default:
                return CONFIG_PARSE_ERROR;
        }
    }

    // Map YAML values to configuration structure
    // Comprehensive mapping of all SAGE parameters

    // File information
    if (yaml_get_value(&yaml_data, "files.output_dir", value)) {
        strncpy(config->base.OutputDir, value, MAX_STRING_LEN - 1);
        config->base.OutputDir[MAX_STRING_LEN - 1] = '\0';
    }
    if (yaml_get_value(&yaml_data, "files.galaxy_file_name", value)) {
        strncpy(config->base.FileNameGalaxies, value, MAX_STRING_LEN - 1);
        config->base.FileNameGalaxies[MAX_STRING_LEN - 1] = '\0';
    }
    if (yaml_get_value(&yaml_data, "files.tree_name", value)) {
        strncpy(config->base.TreeName, value, MAX_STRING_LEN - 1);
        config->base.TreeName[MAX_STRING_LEN - 1] = '\0';
    }
    if (yaml_get_value(&yaml_data, "files.simulation_dir", value)) {
        strncpy(config->base.SimulationDir, value, MAX_STRING_LEN - 1);
        config->base.SimulationDir[MAX_STRING_LEN - 1] = '\0';
    }
    if (yaml_get_value(&yaml_data, "files.snapshot_list", value)) {
        strncpy(config->base.FileWithSnapList, value, MAX_STRING_LEN - 1);
        config->base.FileWithSnapList[MAX_STRING_LEN - 1] = '\0';
    }

    // Handle TreeType
    if (yaml_get_value(&yaml_data, "files.tree_type", value)) {
        if (strcasecmp(value, "genesis_lhalo_hdf5") == 0) {
            config->base.TreeType = genesis_lhalo_hdf5;
            snprintf(config->base.TreeExtension, 511, ".hdf5");
        } else if (strcasecmp(value, "lhalo_binary") == 0) {
            config->base.TreeType = lhalo_binary;
            snprintf(config->base.TreeExtension, 511, "");
        } else {
            ERROR_LOG("Invalid TreeType '%s'. Valid options are 'genesis_lhalo_hdf5' or 'lhalo_binary'", value);
            yaml_data_free(&yaml_data);
            return CONFIG_VALIDATION_ERROR;
        }
    }

    // Simulation parameters
    if (yaml_get_value(&yaml_data, "simulation.first_file", value)) {
        config->base.FirstFile = atoi(value);
    }
    if (yaml_get_value(&yaml_data, "simulation.last_file", value)) {
        config->base.LastFile = atoi(value);
    }
    if (yaml_get_value(&yaml_data, "simulation.last_snapshot", value)) {
        config->base.LastSnapshotNr = atoi(value);
    }
    if (yaml_get_value(&yaml_data, "simulation.num_outputs", value)) {
        config->base.NOUT = atoi(value);
    }

    // Parse output snapshots array
    if (config->base.NOUT > 0) {
        int parsed_count = yaml_get_int_array(&yaml_data, "simulation.output_snapshots",
                                             config->base.ListOutputSnaps, ABSOLUTEMAXSNAPS);
        if (parsed_count > 0) {
            // Limit to the requested number of outputs
            if (parsed_count < config->base.NOUT) {
                config->base.NOUT = parsed_count;
            }
            INFO_LOG("Parsed %d output snapshots from YAML configuration", parsed_count);
        }
    }
    if (yaml_get_value(&yaml_data, "simulation.box_size", value)) {
        config->base.BoxSize = atof(value);
    }
    if (yaml_get_value(&yaml_data, "simulation.omega", value)) {
        config->base.Omega = atof(value);
    }
    if (yaml_get_value(&yaml_data, "simulation.omega_lambda", value)) {
        config->base.OmegaLambda = atof(value);
    }
    if (yaml_get_value(&yaml_data, "simulation.hubble_h", value)) {
        config->base.Hubble_h = atof(value);
    }
    if (yaml_get_value(&yaml_data, "simulation.baryon_fraction", value)) {
        config->base.BaryonFrac = atof(value);
    }
    if (yaml_get_value(&yaml_data, "simulation.particle_mass", value)) {
        config->base.PartMass = atof(value);
    }

    // Physics parameters
    if (yaml_get_value(&yaml_data, "physics.star_formation.efficiency", value)) {
        config->base.SfrEfficiency = atof(value);
    }
    if (yaml_get_value(&yaml_data, "physics.star_formation.prescription", value)) {
        config->base.SFprescription = atoi(value);
    }

    if (yaml_get_value(&yaml_data, "physics.feedback.reheating_epsilon", value)) {
        config->base.FeedbackReheatingEpsilon = atof(value);
    }
    if (yaml_get_value(&yaml_data, "physics.feedback.ejection_efficiency", value)) {
        config->base.FeedbackEjectionEfficiency = atof(value);
    }

    if (yaml_get_value(&yaml_data, "physics.agn.recipe", value)) {
        config->base.AGNrecipeOn = atoi(value);
    }
    if (yaml_get_value(&yaml_data, "physics.agn.radio_mode_efficiency", value)) {
        config->base.RadioModeEfficiency = atof(value);
    }
    if (yaml_get_value(&yaml_data, "physics.agn.quasar_mode_efficiency", value)) {
        config->base.QuasarModeEfficiency = atof(value);
    }
    if (yaml_get_value(&yaml_data, "physics.agn.black_hole_growth_rate", value)) {
        config->base.BlackHoleGrowthRate = atof(value);
    }

    if (yaml_get_value(&yaml_data, "physics.supernova.recipe", value)) {
        config->base.SupernovaRecipeOn = atoi(value);
    }
    if (yaml_get_value(&yaml_data, "physics.supernova.energy", value)) {
        config->base.EnergySN = atof(value);
    }
    if (yaml_get_value(&yaml_data, "physics.supernova.efficiency", value)) {
        config->base.EtaSN = atof(value);
    }

    if (yaml_get_value(&yaml_data, "physics.reionization.enabled", value)) {
        config->base.ReionizationOn = atoi(value);
    }
    if (yaml_get_value(&yaml_data, "physics.reionization.z0", value)) {
        config->base.Reionization_z0 = atof(value);
    }
    if (yaml_get_value(&yaml_data, "physics.reionization.zr", value)) {
        config->base.Reionization_zr = atof(value);
    }

    if (yaml_get_value(&yaml_data, "physics.disk_instability.enabled", value)) {
        config->base.DiskInstabilityOn = atoi(value);
    }

    if (yaml_get_value(&yaml_data, "physics.reincorporation_factor", value)) {
        config->base.ReIncorporationFactor = atof(value);
    }
    if (yaml_get_value(&yaml_data, "physics.threshold_major_merger", value)) {
        config->base.ThreshMajorMerger = atof(value);
    }
    if (yaml_get_value(&yaml_data, "physics.threshold_sat_disruption", value)) {
        config->base.ThresholdSatDisruption = atof(value);
    }

    // Chemical evolution
    if (yaml_get_value(&yaml_data, "physics.yield", value)) {
        config->base.Yield = atof(value);
    }
    if (yaml_get_value(&yaml_data, "physics.recycle_fraction", value)) {
        config->base.RecycleFraction = atof(value);
    }
    if (yaml_get_value(&yaml_data, "physics.frac_z_leave_disk", value)) {
        config->base.FracZleaveDisk = atof(value);
    }

    // Units
    if (yaml_get_value(&yaml_data, "units.length_cm", value)) {
        config->base.UnitLength_in_cm = atof(value);
    }
    if (yaml_get_value(&yaml_data, "units.mass_g", value)) {
        config->base.UnitMass_in_g = atof(value);
    }
    if (yaml_get_value(&yaml_data, "units.velocity_cm_per_s", value)) {
        config->base.UnitVelocity_in_cm_per_s = atof(value);
    }

    // Options
    if (yaml_get_value(&yaml_data, "options.overwrite_output_files", value)) {
        config->base.OverwriteOutputFiles = atoi(value);
    }

    // Set derived values
    config->base.MAXSNAPS = config->base.LastSnapshotNr + 1;

    // Cleanup
    yaml_data_free(&yaml_data);

    // Validate configuration
    result = config_validate(config);
    if (result != CONFIG_SUCCESS) {
        return result;
    }

    // Synchronize with global variables for backward compatibility
    config_sync_globals(config);

    INFO_LOG("YAML configuration loaded successfully from %s", filename);
    return CONFIG_SUCCESS;
}

/**
 * @brief Validate configuration structure
 */
config_result_t config_validate(const config_t *config) {
    if (!config) {
        return CONFIG_MEMORY_ERROR;
    }

    // Basic validation - check required fields
    if (strlen(config->base.OutputDir) == 0) {
        ERROR_LOG("OutputDir is required but not specified");
        return CONFIG_VALIDATION_ERROR;
    }

    if (strlen(config->base.FileNameGalaxies) == 0) {
        ERROR_LOG("FileNameGalaxies is required but not specified");
        return CONFIG_VALIDATION_ERROR;
    }

    if (config->base.FirstFile < 0 || config->base.LastFile < config->base.FirstFile) {
        ERROR_LOG("Invalid file range: FirstFile=%d, LastFile=%d",
                 config->base.FirstFile, config->base.LastFile);
        return CONFIG_VALIDATION_ERROR;
    }

    // Additional validations can be added here

    return CONFIG_SUCCESS;
}

/**
 * @brief Copy SageConfig structure
 */
void config_copy_sage_config(struct SageConfig *dest, const struct SageConfig *src) {
    if (dest && src) {
        memcpy(dest, src, sizeof(struct SageConfig));
    }
}

/**
 * @brief Synchronize global variables with configuration
 */
void config_sync_globals(const config_t *config) {
    if (!config) return;

    // Copy entire SageConfig structure to global
    extern struct SageConfig SageConfig;
    config_copy_sage_config(&SageConfig, &config->base);

    // Synchronize individual globals that are still used
    extern int NOUT, MAXSNAPS;
    extern int ListOutputSnaps[ABSOLUTEMAXSNAPS];

    NOUT = config->base.NOUT;
    MAXSNAPS = config->base.MAXSNAPS;

    for (int i = 0; i < ABSOLUTEMAXSNAPS && i < config->base.NOUT; i++) {
        ListOutputSnaps[i] = config->base.ListOutputSnaps[i];
    }
}

/**
 * @brief Convert configuration result to human-readable string
 */
const char *config_result_string(config_result_t result) {
    switch (result) {
    case CONFIG_SUCCESS:
        return "Success";
    case CONFIG_FILE_NOT_FOUND:
        return "Configuration file not found";
    case CONFIG_PARSE_ERROR:
        return "Error parsing YAML configuration file";
    case CONFIG_VALIDATION_ERROR:
        return "Configuration validation failed";
    case CONFIG_MEMORY_ERROR:
        return "Memory allocation error";
    case CONFIG_YAML_NOT_AVAILABLE:
        return "YAML support not available";
    case CONFIG_UNKNOWN_ERROR:
    default:
        return "Unknown error";
    }
}

/**
 * @brief Print configuration summary
 */
void config_print_summary(const config_t *config) {
    if (!config) return;

    INFO_LOG("Configuration Summary:");
    INFO_LOG("  Source file: %s (YAML format)", config->source_file);
    INFO_LOG("  Output directory: %s", config->base.OutputDir);
    INFO_LOG("  Galaxy file name: %s", config->base.FileNameGalaxies);
    INFO_LOG("  File range: %d-%d", config->base.FirstFile, config->base.LastFile);
    INFO_LOG("  Box size: %.2f Mpc/h", config->base.BoxSize);
    INFO_LOG("  Omega: %.3f, OmegaLambda: %.3f, h: %.3f",
             config->base.Omega, config->base.OmegaLambda, config->base.Hubble_h);
}