/**
 * @file    io/tree_binary.c
 * @brief   Functions for reading binary merger tree files
 *
 * This file implements functionality for loading merger trees from
 * binary format files. It handles the reading of tree metadata and
 * halo data for individual trees, providing an interface to the core
 * SAGE code that is independent of the specific file format.
 *
 * Binary format trees are the traditional SAGE input format, consisting
 * of a simple structure with tree counts, halo counts, and arrays of
 * halo data. This format is efficient to read but less flexible than
 * newer formats like HDF5.
 *
 * Key functions:
 * - load_tree_table_binary(): Reads tree metadata from a binary file
 * - load_tree_binary(): Loads a specific tree's halo data
 * - close_binary_file(): Closes the binary file
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "core_proto.h"
#include "globals.h"
#include "io_tree.h"
#include "io_tree_binary.h"
#include "io_util.h"
#include "types.h"
#include "util_error.h"

// Local Variables //

static FILE *load_fd;

// Local Proto-Types //

// External Functions //

#ifndef MAX_BUF_SIZE
#define MAX_BUF_SIZE (3 * MAX_STRING_LEN + 40)
#endif

/**
 * @brief   Loads merger tree metadata from a binary file
 *
 * @param   filenr    File number to load
 *
 * This function opens and reads the metadata from a binary merger tree file.
 * It extracts:
 * 1. The number of trees in the file
 * 2. The total number of halos across all trees
 * 3. The number of halos in each individual tree
 *
 * It allocates memory for tree metadata arrays and calculates the
 * starting index of each tree in the file. This information is used
 * later when loading individual trees.
 *
 * The function also updates the SimState structure to maintain consistency
 * with the global variables.
 */
void load_tree_table_binary(int32_t filenr) {
  int i, totNHalos;
  char buf[MAX_BUF_SIZE + 1];

  // Open the file
  snprintf(buf, MAX_BUF_SIZE, "%s/%s.%d%s", SageConfig.SimulationDir,
           SageConfig.TreeName, filenr, SageConfig.TreeExtension);
  if (!(load_fd = fopen(buf, "r"))) {
    FATAL_ERROR("Failed to open binary tree file '%s' (filenr %d)", buf,
                filenr);
  }

  // For simplicity, assume host endianness for legacy files
  set_file_endianness(SAGE_HOST_ENDIAN);
  DEBUG_LOG("Using legacy headerless file format (assuming %s endian)",
            (SAGE_HOST_ENDIAN == SAGE_LITTLE_ENDIAN) ? "little" : "big");

  // Read the tree metadata
  if (fread(&Ntrees, sizeof(int), 1, load_fd) != 1) {
    FATAL_ERROR("Failed to read Ntrees from file '%s'", buf);
  }
  SimState.Ntrees = Ntrees; /* Update SimState directly */

  if (fread(&totNHalos, sizeof(int), 1, load_fd) != 1) {
    FATAL_ERROR("Failed to read totNHalos from file '%s'", buf);
  }

  DEBUG_LOG("Reading %d trees with %d total halos", Ntrees, totNHalos);

  // Allocate arrays for tree data
  TreeNHalos = mymalloc(sizeof(int) * Ntrees);
  if (TreeNHalos == NULL) {
    FATAL_ERROR("Failed to allocate memory for TreeNHalos array");
  }
  SimState.TreeNHalos = TreeNHalos; /* Update SimState pointer directly */

  TreeFirstHalo = mymalloc(sizeof(int) * Ntrees);
  if (TreeFirstHalo == NULL) {
    FATAL_ERROR("Failed to allocate memory for TreeFirstHalo array");
  }
  SimState.TreeFirstHalo = TreeFirstHalo; /* Update SimState pointer directly */

  // Read the number of halos per tree - using direct fread for now
  if (fread(TreeNHalos, sizeof(int), Ntrees, load_fd) != Ntrees) {
    FATAL_ERROR("Failed to read tree halo counts from file '%s'", buf);
  }

  // Calculate starting indices for each tree
  if (Ntrees > 0) {
    TreeFirstHalo[0] = 0;
    for (i = 1; i < Ntrees; i++)
      TreeFirstHalo[i] = TreeFirstHalo[i - 1] + TreeNHalos[i - 1];
  }
}

/**
 * @brief   Loads a specific merger tree from a binary file
 *
 * @param   filenr    File number containing the tree
 * @param   treenr    Index of the tree to load
 *
 * This function reads the halo data for a specific merger tree from
 * an already-opened binary file. It:
 * 1. Allocates memory for the halos in this tree
 * 2. Reads the halo data from the file into the allocated memory
 *
 * The function assumes that load_tree_table_binary() has already been
 * called to load the tree metadata and that the file is properly positioned
 * for reading.
 *
 * The halos are stored in the global Halo array for processing by the
 * SAGE model.
 */
void load_tree_binary(int32_t filenr, int32_t treenr) {
  // must have an FD
  assert(load_fd);

  Halo = mymalloc(sizeof(struct halo_data) * TreeNHalos[treenr]);
  if (Halo == NULL) {
    FATAL_ERROR("Failed to allocate memory for Halo array with %d halos",
                TreeNHalos[treenr]);
  }

  // Use direct fread to avoid our problematic wrapper
  if (fread(Halo, sizeof(struct halo_data), TreeNHalos[treenr], load_fd) !=
      TreeNHalos[treenr]) {
    FATAL_ERROR("Failed to read halo data for tree %d", treenr);
  }
}

/**
 * @brief   Closes the binary merger tree file
 *
 * This function closes the file handle for the currently open binary
 * merger tree file. It's called when all trees have been processed
 * or when switching to a different file.
 *
 * The function checks if the file is actually open before attempting
 * to close it, and sets the file handle to NULL after closing to
 * prevent multiple close attempts.
 */
void close_binary_file(void) {
  if (load_fd) {
    fclose(load_fd);
    load_fd = NULL;
  }
}
// Local Functions //
