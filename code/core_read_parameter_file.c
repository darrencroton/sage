#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <ctype.h>
#include <limits.h>

#include "globals.h"
#include "types.h"
#include "config.h"
#include "constants.h"
#include "core_proto.h"
#include "parameter_table.h"

void read_parameter_file(char *fname)
{
  FILE *fd;
  char buf[MAX_STRING_LEN * 3];  // Buffer for reading lines
  char buf1[MAX_STRING_LEN];     // Parameter name
  char buf2[MAX_STRING_LEN];     // Parameter value
  char buf3[MAX_STRING_LEN];     // Extra (for comments)
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
  if(ThisTask == 0)
#endif
    printf("\nReading parameter file: %s\n\n", fname);

  fd = fopen(fname, "r");
  if (fd == NULL) {
    ERROR_LOG("Parameter file '%s' not found or cannot be opened.", fname);
    errorFlag = 1;
    goto error_exit;
  }

  // Read parameter file
  while(!feof(fd))
  {
    buf[0] = 0;
    fgets(buf, sizeof(buf), fd);
    
    // Skip if line is too short or starts with comment character
    if(sscanf(buf, "%s%s%s", buf1, buf2, buf3) < 2)
      continue;
      
    if(buf1[0] == '%' || buf1[0] == '-' || buf1[0] == '#')
      continue;
      
    // Look up parameter in the table
    int param_found = 0;
    for (i = 0; i < num_params; i++) {
      if (strcmp(buf1, param_table[i].name) == 0) {
        param_found = 1;
        param_read[i] = 1;
        
        // Print parameter value being read
#ifdef MPI
        if(ThisTask == 0)
#endif
          printf("%-35s = %-20s\n", buf1, buf2);
        
        // Store parameter value based on type
        switch (param_table[i].type) {
          case DOUBLE: {
            double val = atof(buf2);
            // Validate value
            if (!is_parameter_valid(&param_table[i], &val)) {
              ERROR_LOG("Parameter '%s' value %g is outside valid range [%g, %g]", 
                     param_table[i].name, val, 
                     param_table[i].min_value, 
                     param_table[i].max_value > 0 ? param_table[i].max_value : HUGE_VAL);
              errorFlag = 1;
            }
            *((double *) param_table[i].address) = val;
            break;
          }
          case STRING:
            strcpy((char *) param_table[i].address, buf2);
            break;
          case INT: {
            int val = atoi(buf2);
            // Validate value
            if (!is_parameter_valid(&param_table[i], &val)) {
              ERROR_LOG("Parameter '%s' value %d is outside valid range [%g, %g]", 
                     param_table[i].name, val, 
                     param_table[i].min_value, 
                     param_table[i].max_value > 0 ? param_table[i].max_value : INT_MAX);
              errorFlag = 1;
            }
            *((int *) param_table[i].address) = val;
            break;
          }
          default:
            ERROR_LOG("Unknown parameter type for parameter '%s'", param_table[i].name);
            errorFlag = 1;
        }
        break;
      }
    }
    
    if (!param_found) {
      ERROR_LOG("Parameter '%s' in file '%s' is not recognized", buf1, fname);
      errorFlag = 1;
    }
  }
  
  fclose(fd);
  
  // Check for missing required parameters
  for (i = 0; i < num_params; i++) {
    if (param_read[i] == 0 && param_table[i].required) {
      ERROR_LOG("Required parameter '%s' (%s) missing in parameter file '%s'", 
             param_table[i].name, 
             param_table[i].description,
             fname);
      errorFlag = 1;
    }
  }
  
  // Add trailing slash to OutputDir if needed
  i = strlen(SageConfig.OutputDir);
  if (i > 0 && SageConfig.OutputDir[i - 1] != '/')
    strcat(SageConfig.OutputDir, "/");
  
  // Special handling for MAXSNAPS
  if (!(SageConfig.LastSnapShotNr+1 > 0 && SageConfig.LastSnapShotNr+1 < ABSOLUTEMAXSNAPS)) {
    ERROR_LOG("LastSnapshotNr = %d should be in range [0, %d)", 
            SageConfig.LastSnapShotNr, ABSOLUTEMAXSNAPS);
    errorFlag = 1;
  }
  MAXSNAPS = SageConfig.LastSnapShotNr + 1;
  
  // Special handling for NumOutputs parameter
  if (!(SageConfig.NOUT == -1 || (SageConfig.NOUT > 0 && SageConfig.NOUT <= ABSOLUTEMAXSNAPS))) {
    ERROR_LOG("NumOutputs must be -1 (all snapshots) or between 1 and %i", ABSOLUTEMAXSNAPS);
    errorFlag = 1;
  }
  
  // Handle output snapshot list
  if (!errorFlag) {
    if (SageConfig.NOUT == -1) {
      SageConfig.NOUT = MAXSNAPS;
      for (i = SageConfig.NOUT - 1; i >= 0; i--)
        ListOutputSnaps[i] = i;
      printf("All %i snapshots selected for output\n", SageConfig.NOUT);
    } else {
      printf("%i snapshots selected for output: ", SageConfig.NOUT);
      // reopen the parameter file
      fd = fopen(fname, "r");
      
      done = 0;
      while (!feof(fd) && !done) {
        // scan down to find the line with the snapshots
        fscanf(fd, "%s", buf1);
        if (strcmp(buf1, "->") == 0) {
          // read the snapshots into ListOutputSnaps
          for (i = 0; i < SageConfig.NOUT; i++) {
            if (fscanf(fd, "%d", &ListOutputSnaps[i]) != 1) {
              ERROR_LOG("Could not read output snapshot list. Expected %d values after '->' but couldn't read value %d",
                    SageConfig.NOUT, i+1);
              errorFlag = 1;
              break;
            }
            printf("%i ", ListOutputSnaps[i]);
          }
          done = 1;
        }
      }
      
      fclose(fd);
      if (!done && !errorFlag) {
        ERROR_LOG("Could not find output snapshot list (expected line starting with '->') in parameter file");
        errorFlag = 1;
      }
      printf("\n");
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
      ERROR_LOG("TreeType '%s' requires HDF5 support, but SAGE was not compiled with HDF5 option enabled", my_treetype);
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
      ERROR_LOG("TreeType '%s' is not supported. Valid options are 'genesis_lhalo_hdf5' or 'lhalo_binary'", 
              my_treetype);
      errorFlag = 1;
    }
  }

error_exit:
  // Free memory and exit if errors
  myfree(param_read);
  
  if (errorFlag) {
    ABORT(1);
  }
  
  // Print success message
  INFO_LOG("Parameter file '%s' read successfully", fname);
}
