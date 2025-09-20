/**
 * @file    core_read_parameter_file.c
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

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "constants.h"
#include "core_proto.h"
#include "globals.h"
#include "types.h"
#include "util_error.h"
#include "util_parameters.h"

/**
 * @brief   Reads and validates parameters from a configuration file
 *
 * @param   fname   Path to the parameter file
 *
 * This function reads model parameters from a configuration file, validates
 * them against allowed ranges, and initializes both the SageConfig structure
 * and global variables. It performs the following tasks:
 *
 * 1. Opens and reads the parameter file line by line
 * 2. For each parameter, finds its definition in the parameter table
 * 3. Validates the parameter value against its allowed range
 * 4. Stores the value in the appropriate location (SageConfig and globals)
 * 5. Handles special cases for parameters like output directories and snapshot
 * lists
 * 6. Performs post-processing for certain parameter combinations
 *
 * The function uses a table-driven approach where parameters are defined
 * centrally with their types, defaults, and valid ranges. This ensures
 * consistent parameter handling throughout the code.
 *
 * If any errors are encountered (missing required parameters, invalid values),
 * the function will terminate the program with an appropriate error message.
 */
void read_parameter_file(char *fname) {
  FILE *fd;
  char buf[MAX_STRING_LEN * 3]; // Buffer for reading lines
  char buf1[MAX_STRING_LEN];    // Parameter name
  char buf2[MAX_STRING_LEN];    // Parameter value
  char buf3[MAX_STRING_LEN];    // Extra (for comments)
  int i, done;
  int errorFlag = 0;
  char my_treetype[MAX_STRING_LEN]; // Special handling for tree type

  // Get parameter table
  ParameterDefinition *param_table = get_parameter_table();
  int num_params = get_parameter_table_size();

  // Array to track which parameters have been read
  int *param_read = mymalloc(sizeof(int) * num_params);
  for (i = 0; i < num_params; i++) {
    param_read[i] = 0;
  }

  // Special handling for TreeType parameter
  for (i = 0; i < num_params; i++) {
    if (strcmp(param_table[i].name, "TreeType") == 0) {
      param_table[i].address = my_treetype;
      break;
    }
  }

#ifdef MPI
  if (ThisTask == 0)
#endif
    INFO_LOG("Reading parameter file: %s", fname);

  fd = fopen(fname, "r");
  if (fd == NULL) {
    ERROR_LOG("Parameter file '%s' not found or cannot be opened.", fname);
    errorFlag = 1;
    goto error_exit;
  }

  // Read parameter file
  while (!feof(fd)) {
    buf[0] = 0;
    fgets(buf, sizeof(buf), fd);

    // Skip if line is too short or starts with comment character
    if (sscanf(buf, "%s%s%s", buf1, buf2, buf3) < 2)
      continue;

    if (buf1[0] == '%' || buf1[0] == '-' || buf1[0] == '#')
      continue;

    // Look up parameter in the table
    for (i = 0; i < num_params; i++) {
      if (strcmp(buf1, param_table[i].name) == 0) {
        param_read[i] = 1;

        // Print parameter value being read
#ifdef MPI
        if (ThisTask == 0)
#endif
          DEBUG_LOG("%-35s = %-20s", buf1, buf2);

        // Store parameter value based on type
        switch (param_table[i].type) {
        case DOUBLE: {
          double val = atof(buf2);
          // Validate value
          if (!is_parameter_valid(&param_table[i], &val)) {
            ERROR_LOG("Parameter '%s' value %g is outside valid range [%g, %g]",
                      param_table[i].name, val, param_table[i].min_value,
                      param_table[i].max_value > 0 ? param_table[i].max_value
                                                   : HUGE_VAL);
            errorFlag = 1;
          }
          *((double *)param_table[i].address) = val;
          break;
        }
        case STRING:
          strcpy((char *)param_table[i].address, buf2);
          break;
        case INT: {
          int val = atoi(buf2);
          // Validate value
          if (!is_parameter_valid(&param_table[i], &val)) {
            ERROR_LOG("Parameter '%s' value %d is outside valid range [%g, %g]",
                      param_table[i].name, val, param_table[i].min_value,
                      param_table[i].max_value > 0 ? param_table[i].max_value
                                                   : INT_MAX);
            errorFlag = 1;
          }
          *((int *)param_table[i].address) = val;
          break;
        }
        default:
          ERROR_LOG("Unknown parameter type for parameter '%s'",
                    param_table[i].name);
          errorFlag = 1;
        }
        break;
      }
    }
  }

  fclose(fd);

  // Check for missing required parameters
  for (i = 0; i < num_params; i++) {
    if (param_read[i] == 0 && param_table[i].required) {
      ERROR_LOG("Required parameter '%s' (%s) missing in parameter file '%s'",
                param_table[i].name, param_table[i].description, fname);
      errorFlag = 1;
    }
  }

  // Add trailing slash to OutputDir if needed
  i = strlen(SageConfig.OutputDir);
  if (i > 0 && SageConfig.OutputDir[i - 1] != '/')
    strcat(SageConfig.OutputDir, "/");

  // Special handling for MAXSNAPS
  if (!(SageConfig.LastSnapShotNr + 1 > 0 &&
        SageConfig.LastSnapShotNr + 1 < ABSOLUTEMAXSNAPS)) {
    ERROR_LOG("LastSnapshotNr = %d should be in range [0, %d)",
              SageConfig.LastSnapShotNr, ABSOLUTEMAXSNAPS);
    errorFlag = 1;
  }

  // Set MAXSNAPS in both SageConfig and global variable
  SageConfig.MAXSNAPS = SageConfig.LastSnapShotNr + 1;
  MAXSNAPS =
      SageConfig.MAXSNAPS; // Synchronize with global for backward compatibility

  // Special handling for NumOutputs parameter
  if (!(SageConfig.NOUT == -1 ||
        (SageConfig.NOUT > 0 && SageConfig.NOUT <= ABSOLUTEMAXSNAPS))) {
    ERROR_LOG("NumOutputs must be -1 (all snapshots) or between 1 and %i",
              ABSOLUTEMAXSNAPS);
    errorFlag = 1;
  }

  // Handle output snapshot list
  if (!errorFlag) {
    if (SageConfig.NOUT == -1) {
      SageConfig.NOUT = SageConfig.MAXSNAPS;
      for (i = SageConfig.NOUT - 1; i >= 0; i--) {
        SageConfig.ListOutputSnaps[i] = i;
        ListOutputSnaps[i] = i; // Sync with global for backward compatibility
      }
      INFO_LOG("All %i snapshots selected for output", SageConfig.NOUT);
    } else {
      INFO_LOG("%i snapshots selected for output:", SageConfig.NOUT);
      // reopen the parameter file
      fd = fopen(fname, "r");

      done = 0;
      while (!feof(fd) && !done) {
        // scan down to find the line with the snapshots
        fscanf(fd, "%s", buf1);
        if (strcmp(buf1, "->") == 0) {
          // read the snapshots into both the SageConfig structure and global
          // array
          for (i = 0; i < SageConfig.NOUT; i++) {
            if (fscanf(fd, "%d", &SageConfig.ListOutputSnaps[i]) != 1) {
              ERROR_LOG("Could not read output snapshot list. Expected %d "
                        "values after '->' but couldn't read value %d",
                        SageConfig.NOUT, i + 1);
              errorFlag = 1;
              break;
            }
            ListOutputSnaps[i] =
                SageConfig.ListOutputSnaps[i]; // Sync with global
            DEBUG_LOG("Selected snapshot %i: %i", i,
                      SageConfig.ListOutputSnaps[i]);
          }
          done = 1;
        }
      }

      fclose(fd);
      if (!done && !errorFlag) {
        ERROR_LOG("Could not find output snapshot list (expected line starting "
                  "with '->') in parameter file");
        errorFlag = 1;
      }
      // Removed newline print
    }
  }

  // Sync the global variable with the config structure
  NOUT = SageConfig.NOUT;

  // Handle TreeType
  if (!errorFlag) {
    // Check file type is valid.
    if (strcasecmp(my_treetype, "lhalo_binary") != 0) {
      snprintf(SageConfig.TreeExtension, 511, ".hdf5");
#ifndef HDF5
      ERROR_LOG("TreeType '%s' requires HDF5 support, but SAGE was not "
                "compiled with HDF5 option enabled",
                my_treetype);
      ERROR_LOG("Please check your file type and compiler options");
      errorFlag = 1;
#endif
    }

    // Recast the local treetype string to a global TreeType enum.
    if (strcasecmp(my_treetype, "genesis_lhalo_hdf5") == 0) {
      SageConfig.TreeType = genesis_lhalo_hdf5;
    } else if (strcasecmp(my_treetype, "lhalo_binary") == 0) {
      SageConfig.TreeType = lhalo_binary;
    } else {
      ERROR_LOG("TreeType '%s' is not supported. Valid options are "
                "'genesis_lhalo_hdf5' or 'lhalo_binary'",
                my_treetype);
      errorFlag = 1;
    }
  }

error_exit:
  // Free memory and exit if errors
  myfree(param_read);

  if (errorFlag) {
    FATAL_ERROR("Parameter file processing failed with one or more errors");
  }

  // Print success message
  INFO_LOG("Parameter file '%s' read successfully", fname);
}
