/**
 * @file    core_io_tree.c
 * @brief   Functions for loading and managing merger trees
 *
 * This file contains the core functionality for loading merger trees from
 * various file formats, managing the tree data in memory, and preparing
 * output files for galaxy data. It serves as a central hub for different
 * tree file formats (binary, HDF5) and handles the allocation/deallocation
 * of tree-related data structures.
 *
 * Key functions:
 * - load_tree_table(): Loads tree metadata from input files
 * - load_tree(): Loads a specific merger tree into memory
 * - free_tree_table(): Frees memory allocated for tree metadata
 * - free_galaxies_and_tree(): Cleans up galaxy and tree data structures
 *
 * The code supports different tree formats through a plugin architecture,
 * with format-specific implementations in the io/ directory.
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

#include "core_allvars.h"
#include "core_proto.h"
#include "io_tree.h"
#include "io_util.h"
#include "util_error.h"

#include "io_tree_binary.h"
#ifdef HDF5
#include "io_tree_hdf5.h"
#endif

/* Global file endianness variable - initialized to host endianness by default
 */
static int file_endianness = SAGE_HOST_ENDIAN;

/* Table of active I/O buffers */
static IOBuffer *read_buffer = NULL;
static IOBuffer *write_buffer = NULL;

#ifndef MAX_BUF_SIZE
#define MAX_BUF_SIZE (3 * MAX_STRING_LEN + 40)
#endif

/**
 * @brief   Loads merger tree metadata and prepares output files
 *
 * @param   filenr       Current file number being processed
 * @param   my_TreeType  Type of merger tree format to load
 *
 * This function loads the table of merger trees from the specified file
 * and initializes data structures for processing these trees. It:
 *
 * 1. Calls the appropriate format-specific loader based on tree type
 * 2. Allocates memory for tracking galaxies per tree for each output snapshot
 * 3. Creates empty output files for each requested snapshot
 * 4. Initializes galaxy counters
 *
 * The function supports different tree formats (binary, HDF5) through a
 * dispatching mechanism to format-specific implementations.
 */
void load_tree_table(int filenr, enum Valid_TreeTypes my_TreeType) {
  int i, n;
  FILE *fd;
  char buf[MAX_BUF_SIZE + 1];

  switch (my_TreeType) {
#ifdef HDF5
  case genesis_lhalo_hdf5:
    load_tree_table_hdf5(filenr);
    break;
#endif

  case lhalo_binary:
    load_tree_table_binary(filenr);
    break;

  default:
    FATAL_ERROR("Unsupported tree type %d in load_tree_table(). Please add "
                "support in core_io_tree.c",
                my_TreeType);
  }

  for (n = 0; n < NOUT; n++) {
    TreeNgals[n] = mymalloc(sizeof(int) * Ntrees);
    if (TreeNgals[n] == NULL) {
      FATAL_ERROR("Memory allocation failed for TreeNgals[%d] array (%d trees, "
                  "%zu bytes)",
                  n, Ntrees, Ntrees * sizeof(int));
    }
    SimState.TreeNgals[n] = TreeNgals[n]; /* Update SimState pointer directly */

    for (i = 0; i < Ntrees; i++)
      TreeNgals[n][i] = 0;

    snprintf(buf, MAX_BUF_SIZE, "%s/%s_z%1.3f_%d", SageConfig.OutputDir,
             SageConfig.FileNameGalaxies, ZZ[ListOutputSnaps[n]], filenr);

    if (!(fd = fopen(buf, "w"))) {
      FATAL_ERROR("Failed to create output galaxy file '%s' for snapshot %d "
                  "(filenr %d)",
                  buf, ListOutputSnaps[n], filenr);
    }
    fclose(fd);
    TotGalaxies[n] = 0;
    SimState.TotGalaxies[n] = 0; /* Update SimState directly */
  }
}

/**
 * @brief   Frees memory allocated for the merger tree table
 *
 * @param   my_TreeType  Type of merger tree format being used
 *
 * This function releases all memory allocated for the merger tree metadata.
 * It frees:
 *
 * 1. Arrays tracking galaxies per tree for each output snapshot
 * 2. The array of first halo indices for each tree
 * 3. The array of halo counts per tree
 * 4. Format-specific resources (e.g., closing file handles)
 *
 * The function ensures proper cleanup of resources after processing
 * is complete, preventing memory leaks.
 */
void free_tree_table(enum Valid_TreeTypes my_TreeType) {
  int n;

  for (n = NOUT - 1; n >= 0; n--) {
    myfree(TreeNgals[n]);
    TreeNgals[n] = NULL;
    SimState.TreeNgals[n] = NULL; /* Update SimState pointer */
  }

  myfree(TreeFirstHalo);
  TreeFirstHalo = NULL;
  SimState.TreeFirstHalo = NULL; /* Update SimState pointer */

  myfree(TreeNHalos);
  TreeNHalos = NULL;
  SimState.TreeNHalos = NULL; /* Update SimState pointer */

  // Don't forget to free the open file handle

  switch (my_TreeType) {
#ifdef HDF5
  case genesis_lhalo_hdf5:
    close_hdf5_file();
    break;
#endif

  case lhalo_binary:
    close_binary_file();
    break;

  default:
    FATAL_ERROR("Unsupported tree type %d in free_tree_table(). Please add "
                "support in core_io_tree.c",
                my_TreeType);
  }
}

/**
 * @brief   Loads a specific merger tree into memory
 *
 * @param   filenr       Current file number being processed
 * @param   treenr       Index of the tree to load
 * @param   my_TreeType  Type of merger tree format to load
 *
 * This function loads a single merger tree from the input file and allocates
 * memory for processing its halos and galaxies. It:
 *
 * 1. Calls the appropriate format-specific loader based on tree type
 * 2. Calculates the maximum number of galaxies for this tree
 * 3. Allocates memory for halo auxiliary data
 * 4. Allocates memory for galaxy data structures
 * 5. Initializes the halo auxiliary data
 *
 * The memory allocation is proportional to the number of halos in the tree,
 * ensuring efficient memory usage while providing sufficient space for
 * galaxies that will be created during processing.
 */
void load_tree(int filenr, int treenr, enum Valid_TreeTypes my_TreeType) {
  int32_t i;

  switch (my_TreeType) {

#ifdef HDF5
  case genesis_lhalo_hdf5:
    load_tree_hdf5(filenr, treenr);
    break;
#endif
  case lhalo_binary:
    load_tree_binary(filenr, treenr);
    break;

  default:
    FATAL_ERROR("Unsupported tree type %d in load_tree(). Please add support "
                "in core_io_tree.c",
                my_TreeType);
  }

  /* Calculate MaxGals based on number of halos with a sensible minimum */
  MaxGals = (int)(MAXGALFAC * TreeNHalos[treenr]);
  if (MaxGals < MIN_GALAXY_ARRAY_GROWTH)
    MaxGals = MIN_GALAXY_ARRAY_GROWTH;

  /* Start with a reasonable size for FoF_MaxGals based on tree characteristics
   */
  FoF_MaxGals = INITIAL_FOF_GALAXIES;
  if ((int)(0.1 * MaxGals) > FoF_MaxGals)
    FoF_MaxGals = (int)(0.1 * MaxGals);

  /* Update SimulationState */
  SimState.MaxGals = MaxGals;
  SimState.FoF_MaxGals = FoF_MaxGals;
  sync_sim_state_to_globals();

  HaloAux = mymalloc(sizeof(struct halo_aux_data) * TreeNHalos[treenr]);
  if (HaloAux == NULL) {
    FATAL_ERROR(
        "Memory allocation failed for HaloAux array (%d halos, %zu bytes)",
        TreeNHalos[treenr], TreeNHalos[treenr] * sizeof(struct halo_aux_data));
  }

  HaloGal = mymalloc(sizeof(struct GALAXY) * MaxGals);
  if (HaloGal == NULL) {
    FATAL_ERROR(
        "Memory allocation failed for HaloGal array (%d galaxies, %zu bytes)",
        MaxGals, MaxGals * sizeof(struct GALAXY));
  }

  Gal = mymalloc(sizeof(struct GALAXY) * FoF_MaxGals);
  if (Gal == NULL) {
    FATAL_ERROR(
        "Memory allocation failed for Gal array (%d galaxies, %zu bytes)",
        FoF_MaxGals, FoF_MaxGals * sizeof(struct GALAXY));
  }

  for (i = 0; i < TreeNHalos[treenr]; i++) {
    HaloAux[i].DoneFlag = 0;
    HaloAux[i].HaloFlag = 0;
    HaloAux[i].NGalaxies = 0;
  }
}

/**
 * @brief   Frees memory allocated for galaxies and the current merger tree
 *
 * This function releases all memory allocated for galaxy and halo data
 * structures after a merger tree has been processed. It frees:
 *
 * 1. The temporary galaxy array used during processing (Gal)
 * 2. The permanent galaxy array for output (HaloGal)
 * 3. The halo auxiliary data array (HaloAux)
 * 4. The halo data array (Halo)
 *
 * This cleanup is performed after each tree is fully processed, allowing
 * the memory to be reused for the next tree.
 */
void free_galaxies_and_tree(void) {
  myfree(Gal);
  myfree(HaloGal);
  myfree(HaloAux);
  myfree(Halo);
}

/**
 * @brief   Set the endianness for binary file operations
 *
 * @param   endianness    The endianness to use (SAGE_LITTLE_ENDIAN or
 * SAGE_BIG_ENDIAN)
 *
 * This function sets the global endianness value used for all binary file
 * operations. It's typically called after detecting the endianness of a file.
 * The functions myfread and myfwrite will use this value to perform any
 * necessary byte swapping.
 */
void set_file_endianness(int endianness) {
  if (endianness != SAGE_LITTLE_ENDIAN && endianness != SAGE_BIG_ENDIAN) {
    WARNING_LOG("Invalid endianness value %d. Using host endianness (%d).",
                endianness, SAGE_HOST_ENDIAN);
    file_endianness = SAGE_HOST_ENDIAN;
  } else {
    file_endianness = endianness;
  }
}

/**
 * @brief   Get the current file endianness setting
 *
 * @return  Current file endianness (SAGE_LITTLE_ENDIAN or SAGE_BIG_ENDIAN)
 *
 * This function returns the global endianness value currently used for
 * binary file operations.
 */
int get_file_endianness(void) { return file_endianness; }

/**
 * @brief   Initializes I/O buffering for SAGE
 *
 * This function sets up buffers for reading and writing operations.
 * It should be called once during program initialization.
 */
void init_io_buffering(void) {
  /* Release any existing buffers */
  if (read_buffer != NULL) {
    free_buffer(read_buffer);
    read_buffer = NULL;
  }

  if (write_buffer != NULL) {
    free_buffer(write_buffer);
    write_buffer = NULL;
  }

  /* Buffers will be created on-demand when files are opened */
}

/**
 * @brief   Cleans up I/O buffering resources
 *
 * This function frees buffers used for I/O operations.
 * It should be called before program termination.
 */
void cleanup_io_buffering(void) {
  /* Global buffer references are just shortcuts - not allocations to free */
  read_buffer = NULL;
  write_buffer = NULL;

  /* The actual buffers will be freed when file handles are closed */
}

/**
 * @brief   Enhanced wrapper for the fread function with buffering and
 * endianness handling
 *
 * @param   ptr      Pointer to the data buffer
 * @param   size     Size of each element
 * @param   nmemb    Number of elements to read
 * @param   stream   File stream to read from
 * @return  Number of elements successfully read
 *
 * This function provides a wrapper around the standard fread function
 * with added buffering, endianness conversion, and error handling.
 */
size_t myfread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
  size_t items_read;
  IOBuffer *buffer;

  if (ptr == NULL || stream == NULL) {
    IO_ERROR_LOG(IO_ERROR_READ_FAILED, "myfread", NULL,
                 "NULL pointer passed to myfread");
    return 0;
  }

  /* Get buffer for this file */
  buffer = get_buffer(stream);
  if (buffer == NULL) {
    /* No buffer yet, create one */
    size_t buffer_size = get_optimal_buffer_size(stream);
    buffer = create_buffer(buffer_size, IO_BUFFER_READ, stream);
    if (buffer == NULL) {
      /* Fall back to direct read if buffer creation fails */
      WARNING_LOG("Failed to create read buffer - using direct read");
      items_read = fread(ptr, size, nmemb, stream);

      /* Perform endianness conversion if needed */
      if (items_read > 0 && !is_same_endian(file_endianness)) {
        /* Only swap if element size is appropriate */
        if (size == 2 || size == 4 || size == 8) {
          swap_bytes_if_needed(ptr, size, items_read, file_endianness);
        }
      }
      return items_read;
    }
    register_buffer(stream, buffer);
    if (read_buffer == NULL) {
      read_buffer = buffer; /* Store global reference to the main read buffer */
    }
  }

  /* Use buffered read */
  items_read = buffered_read(buffer, ptr, size, nmemb);

  /* Perform endianness conversion if needed */
  if (items_read > 0 && !is_same_endian(file_endianness)) {
    /* Only swap if element size is appropriate */
    if (size == 2 || size == 4 || size == 8) {
      swap_bytes_if_needed(ptr, size, items_read, file_endianness);
    }
  }

  return items_read;
}

/**
 * @brief   Enhanced wrapper for the fwrite function with buffering and
 * endianness handling
 *
 * @param   ptr      Pointer to the data buffer
 * @param   size     Size of each element
 * @param   nmemb    Number of elements to write
 * @param   stream   File stream to write to
 * @return  Number of elements successfully written
 *
 * This function provides a wrapper around the standard fwrite function
 * with added buffering, endianness conversion, and error handling.
 */
size_t myfwrite(void *ptr, size_t size, size_t nmemb, FILE *stream) {
  void *tmp_buffer = NULL;
  size_t items_written;
  IOBuffer *buffer;

  if (ptr == NULL || stream == NULL) {
    IO_ERROR_LOG(IO_ERROR_WRITE_FAILED, "myfwrite", NULL,
                 "NULL pointer passed to myfwrite");
    return 0;
  }

  /* Create a clean copy of the data for writing */
  tmp_buffer = malloc(size * nmemb);
  if (tmp_buffer == NULL) {
    WARNING_LOG("Failed to allocate temporary buffer for write operation");
    return 0;
  }

  /* First zero the buffer to avoid any garbage data */
  memset(tmp_buffer, 0, size * nmemb);

  /* Then copy the data to ensure clean transfer */
  memcpy(tmp_buffer, ptr, size * nmemb);

  /* Perform endianness conversion if needed */
  if (!is_same_endian(file_endianness) &&
      (size == 2 || size == 4 || size == 8)) {
    /* Swap bytes in temporary buffer */
    swap_bytes_if_needed(tmp_buffer, size, nmemb, file_endianness);
  }

  /* STEP 1: Try using our simplified buffer approach for better performance */
  buffer = get_buffer(stream);
  if (buffer != NULL) {
    /* Use our simplified buffer implementation */
    items_written = buffered_write(buffer, tmp_buffer, size, nmemb);

    /* For large or critical data, force an immediate flush */
    if (size > 100 || nmemb > 100) {
      buffered_flush(stream);
    }
  } else {
    /* Fallback to direct write if no buffer */
    items_written = fwrite(tmp_buffer, size, nmemb, stream);
  }

  /* Free temporary buffer */
  free(tmp_buffer);

  return items_written;
}

/**
 * @brief   Enhanced wrapper for the fseek function with buffering support
 *
 * @param   stream   File stream to seek within
 * @param   offset   Offset from the position specified by whence
 * @param   whence   Position from which to seek (SEEK_SET, SEEK_CUR, SEEK_END)
 * @return  0 on success, non-zero on error
 *
 * This function provides a wrapper around the standard fseek function
 * with added buffer management.
 */
int myfseek(FILE *stream, long offset, int whence) {
  IOBuffer *buffer;
  int result;

  if (stream == NULL) {
    IO_ERROR_LOG(IO_ERROR_SEEK_FAILED, "myfseek", NULL,
                 "NULL stream pointer passed to myfseek");
    return -1;
  }

  /* Get buffer for this file */
  buffer = get_buffer(stream);
  if (buffer == NULL) {
    /* No buffer, use direct seek */
    result = fseek(stream, offset, whence);
    if (result != 0) {
      IO_ERROR_LOG(IO_ERROR_SEEK_FAILED, "myfseek", NULL,
                   "fseek failed with error %d", result);
    }
    return result;
  }

  /* Use buffered seek */
  result = buffered_seek(buffer, offset, whence);
  if (result != 0) {
    IO_ERROR_LOG(IO_ERROR_SEEK_FAILED, "myfseek", NULL,
                 "buffered_seek failed with error %d", result);
  }
  return result;
}
