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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>

#include "globals.h"
#include "types.h"
#include "config.h"
#include "core_proto.h"
#include "util_error.h"
#include "io_tree_binary.h"

// Local Variables //

static FILE *load_fd;

// Local Proto-Types //

// External Functions //

#ifndef MAX_BUF_SIZE
#define MAX_BUF_SIZE (3*MAX_STRING_LEN+40)
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
void load_tree_table_binary(int32_t filenr)
{
  int i, totNHalos;
  char buf[MAX_BUF_SIZE+1];

        // open the file each time this function is called
  snprintf(buf, MAX_BUF_SIZE, "%s/%s.%d%s", SageConfig.SimulationDir, SageConfig.TreeName, filenr, SageConfig.TreeExtension);
  if(!(load_fd = fopen(buf, "r")))
  {
    FATAL_ERROR("Failed to open binary tree file '%s' (filenr %d)", buf, filenr);
  }

  myfread(&Ntrees, 1, sizeof(int), load_fd);
  SimState.Ntrees = Ntrees; /* Update SimState directly */
  
  myfread(&totNHalos, 1, sizeof(int), load_fd);

  TreeNHalos = mymalloc(sizeof(int) * Ntrees);
  SimState.TreeNHalos = TreeNHalos; /* Update SimState pointer directly */
  
  TreeFirstHalo = mymalloc(sizeof(int) * Ntrees);
  SimState.TreeFirstHalo = TreeFirstHalo; /* Update SimState pointer directly */

  myfread(TreeNHalos, Ntrees, sizeof(int), load_fd);

  if(Ntrees)
    TreeFirstHalo[0] = 0;
  for(i = 1; i < Ntrees; i++)
    TreeFirstHalo[i] = TreeFirstHalo[i - 1] + TreeNHalos[i - 1];

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
void load_tree_binary(int32_t filenr, int32_t treenr)
{
  // must have an FD
  assert(load_fd );

  Halo = mymalloc(sizeof(struct halo_data) * TreeNHalos[treenr]);

  myfread(Halo, TreeNHalos[treenr], sizeof(struct halo_data), load_fd);

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
void close_binary_file(void)
{
  if(load_fd)
  {
    fclose(load_fd);
    load_fd = NULL;
  }
}
// Local Functions //
