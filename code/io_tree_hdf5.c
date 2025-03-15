/**
 * @file    io/tree_hdf5.c
 * @brief   Functions for reading HDF5 format merger tree files
 *
 * This file implements functionality for loading merger trees from
 * HDF5 format files. It handles the reading of tree metadata and
 * halo data for individual trees, providing an interface to the core
 * SAGE code that is independent of the specific file format.
 *
 * HDF5 format trees are a newer, more flexible format compared to
 * the traditional binary format. The HDF5 format allows for:
 * - Self-describing data with attributes and metadata
 * - Better portability across different systems
 * - Easier extensibility for future enhancements
 *
 * Key functions:
 * - load_tree_table_hdf5(): Reads tree metadata from an HDF5 file
 * - load_tree_hdf5(): Loads a specific tree's halo data
 * - close_hdf5_file(): Closes the HDF5 file
 * - read_attribute_int(): Helper for reading integer attributes
 * - read_dataset(): Helper for reading datasets of various types
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
#include "io_tree_hdf5.h"
#include "types.h"

// Local Variables //
static hid_t hdf5_file;

// Local Structs //

struct METADATA_NAMES {
  char name_NTrees[MAX_STRING_LEN + 1];
  char name_totNHalos[MAX_STRING_LEN + 1];
  char name_TreeNHalos[MAX_STRING_LEN + 1];
};

// Local Proto-Types //

int32_t fill_metadata_names(struct METADATA_NAMES *metadata_names,
                            enum Valid_TreeTypes my_TreeType);
int32_t read_attribute_int(hid_t my_hdf5_file, char *groupname, char *attr_name,
                           int *attribute);
int32_t read_dataset(hid_t my_hdf5_file, char *dataset_name, int32_t datatype,
                     void *buffer);

// External Functions //

/**
 * @brief   Loads merger tree metadata from an HDF5 file
 *
 * @param   filenr    File number to load
 *
 * This function opens and reads the metadata from an HDF5 merger tree file.
 * It extracts:
 * 1. The number of trees in the file
 * 2. The total number of halos across all trees
 * 3. The number of halos in each individual tree
 *
 * It allocates memory for tree metadata arrays and calculates the
 * starting index of each tree in the file. This information is used
 * later when loading individual trees.
 *
 * The function supports different HDF5 schemas through the fill_metadata_names
 * helper function, which determines the attribute names based on the tree type.
 */
void load_tree_table_hdf5(int filenr) {

  char buf[MAX_STRING_LEN + 1];
  int32_t totNHalos, i;
  int32_t status;

  struct METADATA_NAMES metadata_names;

  snprintf(buf, MAX_STRING_LEN, "%s/%s.%d%s", SimulationDir, TreeName, filenr,
           TreeExtension);
  hdf5_file = H5Fopen(buf, H5F_ACC_RDONLY, H5P_DEFAULT);

  if (hdf5_file < 0) {
    FATAL_ERROR("Failed to open HDF5 tree file '%s'", buf);
  }

  status = fill_metadata_names(&metadata_names, TreeType);
  if (status != EXIT_SUCCESS) {
    FATAL_ERROR("Failed to fill metadata names for tree type %d", TreeType);
  }

  status = read_attribute_int(hdf5_file, "/Header", metadata_names.name_NTrees,
                              &Ntrees);
  if (status != EXIT_SUCCESS) {
    FATAL_ERROR("Error %d while reading NTrees attribute from file '%s'",
                status, buf);
  }

  status = read_attribute_int(hdf5_file, "/Header",
                              metadata_names.name_totNHalos, &totNHalos);
  if (status != EXIT_SUCCESS) {
    FATAL_ERROR("Error %d while reading totNHalos attribute from file '%s'",
                status, buf);
  }

  printf("There are %d trees and %d total halos\n", Ntrees, totNHalos);

  TreeNHalos = mymalloc(sizeof(int) * Ntrees);

  status = read_attribute_int(hdf5_file, "/Header",
                              metadata_names.name_TreeNHalos, TreeNHalos);
  if (status != EXIT_SUCCESS) {
    fprintf(stderr, "Error while processing file %s\n", buf);
    fprintf(stderr, "Error code is %d\n", status);
    ABORT(0);
  }

  TreeFirstHalo = mymalloc(sizeof(int) * Ntrees);

  for (i = 0; i < 20; ++i)
    printf("Tree %d: NHalos %d\n", i, TreeNHalos[i]);

  if (Ntrees)
    TreeFirstHalo[0] = 0;
  for (i = 1; i < Ntrees; i++)
    TreeFirstHalo[i] = TreeFirstHalo[i - 1] + TreeNHalos[i - 1];
}

#define READ_TREE_PROPERTY(sage_name, hdf5_name, type_int, data_type)          \
  {                                                                            \
    snprintf(dataset_name, MAX_STRING_LEN, "tree_%03d/%s", treenr,             \
             #hdf5_name);                                                      \
    status = read_dataset(hdf5_file, dataset_name, type_int, buffer);          \
    if (status != EXIT_SUCCESS) {                                              \
      ABORT(0);                                                                \
    }                                                                          \
    for (halo_idx = 0; halo_idx < NHalos_ThisTree; ++halo_idx) {               \
      Halo[halo_idx].sage_name = ((data_type *)buffer)[halo_idx];              \
    }                                                                          \
  }

#define READ_TREE_PROPERTY_MULTIPLEDIM(sage_name, hdf5_name, type_int,         \
                                       data_type)                              \
  {                                                                            \
    snprintf(dataset_name, MAX_STRING_LEN, "tree_%03d/%s", treenr,             \
             #hdf5_name);                                                      \
    status =                                                                   \
        read_dataset(hdf5_file, dataset_name, type_int, buffer_multipledim);   \
    if (status != EXIT_SUCCESS) {                                              \
      ABORT(0);                                                                \
    }                                                                          \
    for (halo_idx = 0; halo_idx < NHalos_ThisTree; ++halo_idx) {               \
      for (dim = 0; dim < NDIM; ++dim) {                                       \
        Halo[halo_idx].sage_name[dim] =                                        \
            ((data_type *)buffer_multipledim)[halo_idx * NDIM + dim];          \
      }                                                                        \
    }                                                                          \
  }

/**
 * @brief   Loads a specific merger tree from an HDF5 file
 *
 * @param   filenr    File number containing the tree
 * @param   treenr    Index of the tree to load
 *
 * This function reads the halo data for a specific merger tree from
 * an already-opened HDF5 file. It:
 * 1. Allocates memory for the halos in this tree
 * 2. Allocates buffers for reading HDF5 datasets
 * 3. Reads and processes each halo property
 *
 * The function uses macros to simplify the repetitive task of reading
 * various properties from the HDF5 file. It handles both scalar properties
 * and multi-dimensional properties (like positions and velocities).
 *
 * The halos are stored in the global Halo array for processing by the
 * SAGE model.
 */
void load_tree_hdf5(int32_t filenr, int32_t treenr) {

  char dataset_name[MAX_STRING_LEN + 1];
  int32_t NHalos_ThisTree, status, halo_idx, dim;

  double *buffer; // Buffer to hold the read HDF5 data.
                  // The largest data-type will be double.

  double *buffer_multipledim; // However also need a buffer three times as large
                              // to hold data such as position/velocity.

  if (hdf5_file <= 0) {
    char err_msg[MAX_STRING_LEN + 1];
    snprintf(err_msg, MAX_STRING_LEN,
             "The HDF5 file should still be opened when reading the halos in "
             "tree %d. Error code: %d",
             treenr, hdf5_file);
    fprintf(stderr, "%s\n", err_msg);
    ABORT(0);
  }

  NHalos_ThisTree = TreeNHalos[treenr];

  Halo = mymalloc(sizeof(struct halo_data) * NHalos_ThisTree);

  buffer = calloc(NHalos_ThisTree, sizeof(*(buffer)));
  if (buffer == NULL) {
    char err_msg[MAX_STRING_LEN + 1];
    snprintf(err_msg, MAX_STRING_LEN,
             "Could not allocate memory for the HDF5 buffer for tree %d (%d "
             "halos, %zu bytes)",
             treenr, NHalos_ThisTree, NHalos_ThisTree * sizeof(*(buffer)));
    fprintf(stderr, "%s\n", err_msg);
    ABORT(0);
  }

  buffer_multipledim =
      calloc(NHalos_ThisTree * NDIM, sizeof(*(buffer_multipledim)));
  if (buffer_multipledim == NULL) {
    char err_msg[MAX_STRING_LEN + 1];
    snprintf(err_msg, MAX_STRING_LEN,
             "Could not allocate memory for the HDF5 multiple dimension buffer "
             "for tree %d (%d halos, %zu bytes)",
             treenr, NHalos_ThisTree,
             NHalos_ThisTree * NDIM * sizeof(*(buffer_multipledim)));
    fprintf(stderr, "%s\n", err_msg);
    ABORT(0);
  }

  // We now need to read in all the halo fields for this tree.
  // To do so, we read the field into a buffer and then properly slot the field
  // into the Halo struct.

  /* Merger Tree Pointers */
  READ_TREE_PROPERTY(Descendant, Descendant, 0, int);
  READ_TREE_PROPERTY(FirstProgenitor, FirstProgenitor, 0, int);
  READ_TREE_PROPERTY(NextProgenitor, NextProgenitor, 0, int);
  READ_TREE_PROPERTY(FirstHaloInFOFgroup, FirstHaloInFOFgroup, 0, int);
  READ_TREE_PROPERTY(NextHaloInFOFgroup, NextHaloInFOFgroup, 0, int);

  /* Halo Properties */
  READ_TREE_PROPERTY(Len, Len, 0, int);
  READ_TREE_PROPERTY(M_Mean200, M_mean200, 1, float);
  READ_TREE_PROPERTY(Mvir, Mvir, 1, float);
  READ_TREE_PROPERTY(M_TopHat, M_TopHat, 1, float);
  READ_TREE_PROPERTY_MULTIPLEDIM(Pos, Pos, 1, float);
  READ_TREE_PROPERTY_MULTIPLEDIM(Vel, Vel, 1, float);
  READ_TREE_PROPERTY(VelDisp, VelDisp, 1, float);
  READ_TREE_PROPERTY(Vmax, Vmax, 1, float);
  READ_TREE_PROPERTY_MULTIPLEDIM(Spin, Spin, 1, float);
  READ_TREE_PROPERTY(MostBoundID, MostBoundID, 2, long long);

  /* File Position Info */
  READ_TREE_PROPERTY(SnapNum, SnapNum, 0, int);
  READ_TREE_PROPERTY(FileNr, Filenr, 0, int);
  READ_TREE_PROPERTY(SubhaloIndex, SubHaloIndex, 0, int);
  READ_TREE_PROPERTY(SubHalfMass, SubHalfMass, 0, int);

  free(buffer);
  free(buffer_multipledim);

#ifdef DEBUG_HDF5_READER
  int32_t i;
  for (i = 0; i < 20; ++i) {
    DEBUG_LOG("halo %d: Descendant %d FirstProg %d x %.4f y %.4f z %.4f", i,
              Halo[i].Descendant, Halo[i].FirstProgenitor, Halo[i].Pos[0],
              Halo[i].Pos[1], Halo[i].Pos[2]);
  }
  // Debug exit point
  FATAL_ERROR("Debug exit after showing first 20 halos");
#endif
}

#undef READ_TREE_PROPERTY
#undef READ_TREE_PROPERTY_MULTIPLEDIM

/**
 * @brief   Closes the HDF5 merger tree file
 *
 * This function closes the HDF5 file handle for the currently open
 * merger tree file. It's called when all trees have been processed
 * or when switching to a different file.
 *
 * Proper file closure is important to ensure all data is flushed to
 * disk and to free associated HDF5 resources.
 */
void close_hdf5_file(void) { H5Fclose(hdf5_file); }

// Local Functions //

/**
 * @brief   Fills in the metadata attribute names based on tree type
 *
 * @param   metadata_names    Pointer to metadata names structure to fill
 * @param   my_TreeType       Type of merger tree format
 * @return  EXIT_SUCCESS on success, EXIT_FAILURE on error
 *
 * This function determines the correct HDF5 attribute names to use
 * for reading tree metadata based on the tree type. Different tree
 * formats may use different naming conventions for the same data.
 *
 * Currently supports:
 * - genesis_lhalo_hdf5: The standard Genesis L-Galaxies HDF5 format
 *
 * The function returns an error if an unsupported tree type is specified.
 */
int32_t fill_metadata_names(struct METADATA_NAMES *metadata_names,
                            enum Valid_TreeTypes my_TreeType) {

  switch (my_TreeType) {

  case genesis_lhalo_hdf5:

    snprintf(metadata_names->name_NTrees, MAX_STRING_LEN,
             "Ntrees"); // Total number of trees within the file.
    snprintf(metadata_names->name_totNHalos, MAX_STRING_LEN,
             "totNHalos"); // Total number of halos within the file.
    snprintf(metadata_names->name_TreeNHalos, MAX_STRING_LEN,
             "TreeNHalos"); // Number of halos per tree within the file.
    return EXIT_SUCCESS;

  case lhalo_binary:
    ERROR_LOG("If the file is binary then this function should never be "
              "called. Something's gone wrong...");
    return EXIT_FAILURE;

  default:
    FATAL_ERROR(
        "Tree type %d has not been included in the switch statement for "
        "fill_metadata_names in io/tree_hdf5.c. Please add it there.",
        my_TreeType);
  }

  return EXIT_FAILURE;
}

/**
 * @brief   Reads an integer attribute from an HDF5 file
 *
 * @param   my_hdf5_file    HDF5 file handle
 * @param   groupname       Path to the group containing the attribute
 * @param   attr_name       Name of the attribute to read
 * @param   attribute       Pointer to store the read attribute value(s)
 * @return  EXIT_SUCCESS on success, error code on failure
 *
 * This function reads an integer attribute from an HDF5 file. It handles
 * the HDF5 API calls to open, read, and close the attribute, providing
 * error handling and appropriate error messages.
 *
 * The function can read both scalar attributes and array attributes,
 * depending on the provided attribute pointer.
 */
int32_t read_attribute_int(hid_t my_hdf5_file, char *groupname, char *attr_name,
                           int *attribute) {

  int32_t status;
  hid_t attr_id;

  attr_id = H5Aopen_by_name(my_hdf5_file, groupname, attr_name, H5P_DEFAULT,
                            H5P_DEFAULT);
  if (attr_id < 0) {
    ERROR_LOG("Could not open the attribute %s in group %s", attr_name,
              groupname);
    return attr_id;
  }

  status = H5Aread(attr_id, H5T_NATIVE_INT, attribute);
  if (status < 0) {
    ERROR_LOG("Could not read the attribute %s in group %s", attr_name,
              groupname);
    return status;
  }

  status = H5Aclose(attr_id);
  if (status < 0) {
    ERROR_LOG("Error when closing the HDF5 attribute");
    return status;
  }

  return EXIT_SUCCESS;
}

/**
 * @brief   Reads a dataset from an HDF5 file
 *
 * @param   my_hdf5_file    HDF5 file handle
 * @param   dataset_name    Path and name of the dataset to read
 * @param   datatype        Type of data (0=int, 1=float, 2=long long)
 * @param   buffer          Buffer to store the read data
 * @return  EXIT_SUCCESS on success, error code on failure
 *
 * This function reads a dataset from an HDF5 file into the provided buffer.
 * It handles different data types specified by the datatype parameter:
 * - 0: Integer data
 * - 1: Float data
 * - 2: Long long (64-bit integer) data
 *
 * The function provides error checking and appropriate error messages for
 * debugging purposes.
 */
int32_t read_dataset(hid_t my_hdf5_file, char *dataset_name, int32_t datatype,
                     void *buffer) {
  hid_t dataset_id;

  dataset_id = H5Dopen2(hdf5_file, dataset_name, H5P_DEFAULT);
  if (dataset_id < 0) {
    ERROR_LOG("Error %d when trying to open dataset %s", dataset_id,
              dataset_name);
    return dataset_id;
  }

  if (datatype == 0)
    H5Dread(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer);
  else if (datatype == 1)
    H5Dread(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
            buffer);

  else if (datatype == 2)
    H5Dread(dataset_id, H5T_NATIVE_LLONG, H5S_ALL, H5S_ALL, H5P_DEFAULT,
            buffer);

  return EXIT_SUCCESS;
}
