/**
 * @file    parameters.c
 * @brief   Functions for reading and validating model parameter files
 *
 * This file contains the functionality for reading model parameters from
 * a configuration file. It parses the parameter file, validates parameter
 * values against allowed ranges, handles special cases for certain parameters,
 * and initializes both the SageConfig structure and global variables.
 *
 * The parameter system uses a table-driven approach where parameters are
 * defined in a central parameter table with their types, default values,
 * valid ranges, and descriptions. This allows for consistent parameter
 * handling and validation throughout the code.
 *
 * Key functions:
 * - read_parameter_file(): Main function that reads and processes the parameter
 * file
 *
 * The code includes special handling for parameters like output directories,
 * snapshot selections, file types, and handles error cases with appropriate
 * error messages.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "constants.h"
#include "prototypes.h"
#include "globals.h"
#include "types.h"
#include "error.h"
#include "config_reader.h"

/**
 * @brief   Reads and validates parameters from a YAML configuration file
 *
 * @param   fname   Path to the YAML configuration file
 *
 * This function reads model parameters from a YAML configuration file using
 * the modern configuration system. It replaces the legacy .par file reading
 * with a structured YAML-based approach that supports nested sections and
 * enhanced validation.
 *
 * The function performs the following tasks:
 * 1. Initializes the modern configuration reader
 * 2. Parses the YAML file into a structured configuration
 * 3. Validates all parameter values
 * 4. Handles special processing for snapshot lists and directories
 * 5. Synchronizes with global variables for backward compatibility
 *
 * If any errors are encountered during parsing or validation, the function
 * will terminate the program with a detailed error message.
 */
void read_parameter_file(char *fname) {
    config_t config;
    config_result_t result;
    int i;

#ifdef MPI
    if (ThisTask == 0)
#endif
        INFO_LOG("Reading YAML configuration file: %s", fname);

    // Read YAML configuration
    result = config_read_file(fname, &config);
    if (result != CONFIG_SUCCESS) {
        ERROR_LOG("Failed to read configuration file '%s': %s",
                  fname, config_result_string(result));
        FATAL_ERROR("Configuration file processing failed");
    }

    // The config_read_file function already synchronizes with global SageConfig
    // and validates the configuration, so we just need to do some post-processing

    // Add trailing slash to OutputDir if needed
    i = strlen(SageConfig.OutputDir);
    if (i > 0 && SageConfig.OutputDir[i - 1] != '/') {
        strcat(SageConfig.OutputDir, "/");
    }

    // Validate MAXSNAPS range
    if (!(SageConfig.LastSnapshotNr + 1 > 0 &&
          SageConfig.LastSnapshotNr + 1 < ABSOLUTEMAXSNAPS)) {
        ERROR_LOG("LastSnapshotNr = %d should be in range [0, %d)",
                  SageConfig.LastSnapshotNr, ABSOLUTEMAXSNAPS);
        FATAL_ERROR("Invalid LastSnapshotNr value");
    }

    // Validate NumOutputs parameter
    if (!(SageConfig.NOUT == -1 ||
          (SageConfig.NOUT > 0 && SageConfig.NOUT <= SageConfig.LastSnapshotNr + 1))) {
        ERROR_LOG("NumOutputs must be -1 (all snapshots) or between 1 and %i (LastSnapshotNr+1)",
                  SageConfig.LastSnapshotNr + 1);
        FATAL_ERROR("Invalid NumOutputs value");
    }

    // Handle output snapshot list
    if (SageConfig.NOUT == -1) {
        // Use all snapshots
        SageConfig.NOUT = SageConfig.MAXSNAPS;
        for (i = SageConfig.NOUT - 1; i >= 0; i--) {
            SageConfig.ListOutputSnaps[i] = i;
            ListOutputSnaps[i] = i; // Sync with global for backward compatibility
        }
        INFO_LOG("All %i snapshots selected for output", SageConfig.NOUT);
    } else {
        // Output snapshots are already parsed by config_reader from YAML
        INFO_LOG("%i snapshots selected for output:", SageConfig.NOUT);
        for (i = 0; i < SageConfig.NOUT; i++) {
            ListOutputSnaps[i] = SageConfig.ListOutputSnaps[i]; // Sync with global
            DEBUG_LOG("Selected snapshot %i: %i", i, SageConfig.ListOutputSnaps[i]);
        }
    }

    // TreeType validation (already handled in config_reader, but check HDF5 dependency)
    if (SageConfig.TreeType != lhalo_binary) {
#ifndef HDF5
        ERROR_LOG("TreeType requires HDF5 support, but SAGE was not "
                  "compiled with HDF5 option enabled");
        ERROR_LOG("Please check your file type and compiler options");
        FATAL_ERROR("HDF5 support required for selected TreeType");
#endif
    }

    // Clean up configuration resources
    config_free(&config);

    // Print success message
    INFO_LOG("YAML configuration file '%s' read successfully", fname);
}
