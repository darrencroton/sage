/**
 * @file    io/io_save_hdf5.c
 * @brief   Functions for saving galaxy data to HDF5 output files
 *
 * This file implements functionality for writing galaxy data to HDF5 format
 * output files. It handles the creation of HDF5 file structures, the definition
 * of galaxy property tables, and the writing of galaxy data and metadata.
 *
 * The HDF5 format provides several advantages over plain binary files:
 * - Self-describing data with attributes and metadata
 * - Better portability across different systems
 * - Built-in compression and chunking for efficient storage and access
 * - Support for direct access to specific data elements
 *
 * Key functions:
 * - calc_hdf5_props(): Defines the HDF5 table structure for galaxy properties
 * - prep_hdf5_file(): Creates and initializes an HDF5 output file
 * - write_hdf5_galaxy(): Writes a single galaxy to an HDF5 file
 * - write_hdf5_attrs(): Writes metadata attributes to an HDF5 file
 * - write_master_file(): Creates a master file with links to all output files
 */

#include <hdf5.h>
#include <hdf5_hl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "core_proto.h"
#include "io_save_hdf5.h"
#include "util_error.h"

#define TRUE 1
#define FALSE 0

/**
 * @brief   Defines the HDF5 table structure for galaxy properties
 *
 * This function sets up the HDF5 table structure for storing galaxy properties
 * in the output files. It:
 * 1. Defines the total number of galaxy properties to be saved
 * 2. Allocates memory for property metadata arrays
 * 3. Calculates memory offsets for each property in the GALAXY_OUTPUT struct
 * 4. Defines field names and data types for each property
 *
 * The function handles all galaxy properties, including scalars (masses, rates)
 * and arrays (positions, velocities, spins). It configures the HDF5 table
 * to match the layout of the GALAXY_OUTPUT struct for efficient I/O.
 */
void calc_hdf5_props(void) {

  /*
   * Prepare an HDF5 to receive the output galaxy data.
   * Here we store the data in an hdf5 table for easily appending new data.
   */

  struct GALAXY_OUTPUT galout;

  int i; // dummy

  // If we are calculating any magnitudes then increment the number of
  // output properties appropriately.
  HDF5_n_props = 36;

  // Size of a single galaxy entry.
  HDF5_dst_size = sizeof(struct GALAXY_OUTPUT);

  // Create datatypes for different size arrays
  hid_t array3f_tid = H5Tarray_create(H5T_NATIVE_FLOAT, 1, (hsize_t[]){3});

  // Calculate the offsets of our struct members in memory
  HDF5_dst_offsets = mymalloc(sizeof(size_t) * HDF5_n_props);
  // Calculate the sizes of our struct members in memory.
  HDF5_dst_sizes = mymalloc(sizeof(size_t) * HDF5_n_props);
  // Give each galaxy property a field name in the table
  HDF5_field_names = mymalloc(sizeof(const char *) * HDF5_n_props);
  // Assign a type to each galaxy property field in the table.
  HDF5_field_types = mymalloc(sizeof(hid_t) * HDF5_n_props);

  i = 0; // Initialise dummy counter

  // Go through each galaxy property and calculate everything we need...
  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, Type);
  HDF5_dst_sizes[i] = sizeof(galout.Type);
  HDF5_field_names[i] = "Type";
  HDF5_field_types[i++] = H5T_NATIVE_INT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, GalaxyIndex);
  HDF5_dst_sizes[i] = sizeof(galout.GalaxyIndex);
  HDF5_field_names[i] = "GalaxyIndex";
  HDF5_field_types[i++] = H5T_NATIVE_LLONG;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, HaloIndex);
  HDF5_dst_sizes[i] = sizeof(galout.HaloIndex);
  HDF5_field_names[i] = "HaloIndex";
  HDF5_field_types[i++] = H5T_NATIVE_INT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, FOFHaloIndex);
  HDF5_dst_sizes[i] = sizeof(galout.FOFHaloIndex);
  HDF5_field_names[i] = "FOFHaloIndex";
  HDF5_field_types[i++] = H5T_NATIVE_INT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, TreeIndex);
  HDF5_dst_sizes[i] = sizeof(galout.TreeIndex);
  HDF5_field_names[i] = "TreeIndex";
  HDF5_field_types[i++] = H5T_NATIVE_INT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, SnapNum);
  HDF5_dst_sizes[i] = sizeof(galout.SnapNum);
  HDF5_field_names[i] = "SnapNum";
  HDF5_field_types[i++] = H5T_NATIVE_INT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, CentralGal);
  HDF5_dst_sizes[i] = sizeof(galout.CentralGal);
  HDF5_field_names[i] = "CentralGal";
  HDF5_field_types[i++] = H5T_NATIVE_INT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, CentralMvir);
  HDF5_dst_sizes[i] = sizeof(galout.CentralMvir);
  HDF5_field_names[i] = "CentralMvir";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, Pos);
  HDF5_dst_sizes[i] = sizeof(galout.Pos);
  HDF5_field_names[i] = "Pos";
  HDF5_field_types[i++] = array3f_tid;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, Vel);
  HDF5_dst_sizes[i] = sizeof(galout.Vel);
  HDF5_field_names[i] = "Vel";
  HDF5_field_types[i++] = array3f_tid;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, Spin);
  HDF5_dst_sizes[i] = sizeof(galout.Spin);
  HDF5_field_names[i] = "Spin";
  HDF5_field_types[i++] = array3f_tid;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, Len);
  HDF5_dst_sizes[i] = sizeof(galout.Len);
  HDF5_field_names[i] = "Len";
  HDF5_field_types[i++] = H5T_NATIVE_INT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, Mvir);
  HDF5_dst_sizes[i] = sizeof(galout.Mvir);
  HDF5_field_names[i] = "Mvir";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, Rvir);
  HDF5_dst_sizes[i] = sizeof(galout.Rvir);
  HDF5_field_names[i] = "Rvir";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, Vvir);
  HDF5_dst_sizes[i] = sizeof(galout.Vvir);
  HDF5_field_names[i] = "Vvir";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, Vmax);
  HDF5_dst_sizes[i] = sizeof(galout.Vmax);
  HDF5_field_names[i] = "Vmax";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, VelDisp);
  HDF5_dst_sizes[i] = sizeof(galout.VelDisp);
  HDF5_field_names[i] = "VelDisp";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, ColdGas);
  HDF5_dst_sizes[i] = sizeof(galout.ColdGas);
  HDF5_field_names[i] = "ColdGas";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, StellarMass);
  HDF5_dst_sizes[i] = sizeof(galout.StellarMass);
  HDF5_field_names[i] = "StellarMass";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, BulgeMass);
  HDF5_dst_sizes[i] = sizeof(galout.BulgeMass);
  HDF5_field_names[i] = "BulgeMass";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, HotGas);
  HDF5_dst_sizes[i] = sizeof(galout.HotGas);
  HDF5_field_names[i] = "HotGas";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, EjectedMass);
  HDF5_dst_sizes[i] = sizeof(galout.EjectedMass);
  HDF5_field_names[i] = "EjectedMass";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, BlackHoleMass);
  HDF5_dst_sizes[i] = sizeof(galout.BlackHoleMass);
  HDF5_field_names[i] = "BlackHoleMass";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, ICS);
  HDF5_dst_sizes[i] = sizeof(galout.ICS);
  HDF5_field_names[i] = "ICS";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, MetalsColdGas);
  HDF5_dst_sizes[i] = sizeof(galout.MetalsColdGas);
  HDF5_field_names[i] = "MetalsColdGas";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, MetalsStellarMass);
  HDF5_dst_sizes[i] = sizeof(galout.MetalsStellarMass);
  HDF5_field_names[i] = "MetalsStellarMass";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, MetalsBulgeMass);
  HDF5_dst_sizes[i] = sizeof(galout.MetalsBulgeMass);
  HDF5_field_names[i] = "MetalsBulgeMass";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, MetalsHotGas);
  HDF5_dst_sizes[i] = sizeof(galout.MetalsHotGas);
  HDF5_field_names[i] = "MetalsHotGas";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, MetalsEjectedMass);
  HDF5_dst_sizes[i] = sizeof(galout.MetalsEjectedMass);
  HDF5_field_names[i] = "MetalsEjectedMass";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, MetalsICS);
  HDF5_dst_sizes[i] = sizeof(galout.MetalsICS);
  HDF5_field_names[i] = "MetalsICS";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, Sfr);
  HDF5_dst_sizes[i] = sizeof(galout.Sfr);
  HDF5_field_names[i] = "Sfr";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, SfrBulge);
  HDF5_dst_sizes[i] = sizeof(galout.SfrBulge);
  HDF5_field_names[i] = "SfrBulge";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, SfrICS);
  HDF5_dst_sizes[i] = sizeof(galout.SfrICS);
  HDF5_field_names[i] = "SfrICS";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, DiskScaleRadius);
  HDF5_dst_sizes[i] = sizeof(galout.DiskScaleRadius);
  HDF5_field_names[i] = "DiskScaleRadius";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, Cooling);
  HDF5_dst_sizes[i] = sizeof(galout.Cooling);
  HDF5_field_names[i] = "Cooling";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, Heating);
  HDF5_dst_sizes[i] = sizeof(galout.Heating);
  HDF5_field_names[i] = "Heating";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  /* Validate property count */
  if (i != HDF5_n_props) {
    FATAL_ERROR("HDF5 property count mismatch. Expected %d properties but "
                "processed %d properties",
                HDF5_n_props, i);
  }
}

/**
 * @brief   Creates and initializes an HDF5 output file
 *
 * @param   fname   Path to the output file
 *
 * This function creates and initializes a new HDF5 file for galaxy output.
 * It:
 * 1. Creates the file with default HDF5 properties
 * 2. Creates a group for each output snapshot
 * 3. Creates a table within each group to store galaxy data
 * 4. Configures table properties like chunking for optimal performance
 *
 * The created file structure allows easy organization of galaxies by snapshot,
 * and efficient appending of new galaxy records as they are processed.
 */
void prep_hdf5_file(char *fname) {

  hsize_t chunk_size =
      1000; // This value can have a significant impact on read performance!
  int *fill_data = NULL;
  hid_t file_id, snap_group_id;
  char target_group[100];
  hid_t status;
  int i_snap;

  DEBUG_LOG("Creating new HDF5 file '%s'", fname);
  file_id = H5Fcreate(fname, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

  // Create a group for each output snapshot
  for (i_snap = 0; i_snap < NOUT; i_snap++) {
    sprintf(target_group, "Snap%03d", ListOutputSnaps[i_snap]);
    snap_group_id =
        H5Gcreate(file_id, target_group, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    // Make the table
    status =
        H5TBmake_table("Galaxy Table", snap_group_id, "Galaxies", HDF5_n_props,
                       0, HDF5_dst_size, HDF5_field_names, HDF5_dst_offsets,
                       HDF5_field_types, chunk_size, fill_data, 0, NULL);

    H5Gclose(snap_group_id);
  }

  // Close the HDF5 file.
  status = H5Fclose(file_id);
}

/**
 * @brief   Writes a single galaxy to an HDF5 file
 *
 * @param   galaxy_output   Pointer to galaxy data to write
 * @param   n               Snapshot index in ListOutputSnaps
 * @param   filenr          File number to write to
 *
 * This function writes a single galaxy to the appropriate HDF5 file.
 * It:
 * 1. Opens the target HDF5 file
 * 2. Navigates to the correct snapshot group
 * 3. Appends the galaxy record to the galaxy table
 * 4. Properly closes all HDF5 objects
 *
 * The function is designed to be called for each individual galaxy
 * as it is processed, enabling incremental output without requiring
 * all galaxies to be held in memory.
 */
void write_hdf5_galaxy(struct GALAXY_OUTPUT *galaxy_output, int n, int filenr) {

  /*
   * Write a single galaxy to the hdf5 file table.
   */

  herr_t status;
  hid_t file_id, group_id;
  char target_group[100];
  char fname[1000];

  // Generate the filename to be opened.
  sprintf(fname, "%s/%s_%03d.hdf5", OutputDir, FileNameGalaxies, filenr);

  DEBUG_LOG("Opening HDF5 file '%s' for writing galaxy data", fname);
  // Open the file.
  file_id = H5Fopen(fname, H5F_ACC_RDWR, H5P_DEFAULT);

  // Open the relevant group.
  sprintf(target_group, "Snap%03d", ListOutputSnaps[n]);
  group_id = H5Gopen(file_id, target_group, H5P_DEFAULT);

  // Write the galaxy.
  status = H5TBappend_records(group_id, "Galaxies", 1, HDF5_dst_size,
                              HDF5_dst_offsets, HDF5_dst_sizes, galaxy_output);

  // Close the group
  status = H5Gclose(group_id);

  // Close the file.
  status = H5Fclose(file_id);
}

#ifdef MINIMIZE_IO
void write_hdf5_galsnap_data(int n, int filenr) {

  /*
   * Write a batch of galaxies to the output HDF5 table.
   */

  herr_t status;
  hid_t file_id, group_id;
  char target_group[100];
  char fname[1000];

  // Generate the filename to be opened.
  sprintf(fname, "%s/%s_%03d.hdf5", OutputDir, FileNameGalaxies, filenr);

  // Open the file.
  file_id = H5Fopen(fname, H5F_ACC_RDWR, H5P_DEFAULT);

  // Open the relevant group.
  sprintf(target_group, "Snap%03d", ListOutputSnaps[n]);
  group_id = H5Gopen(file_id, target_group, H5P_DEFAULT);

  // Write the galaxies.
  if (TotGalaxies[n] > 0) {
    status = H5TBappend_records(
        group_id, "Galaxies", (hsize_t)(TotGalaxies[n]), HDF5_dst_size,
        HDF5_dst_offsets, HDF5_dst_sizes,
        (struct GALAXY_OUTPUT *)(ptr_galsnapdata[n] + offset_galsnapdata[n]));
  }

  // Close the group.
  status = H5Gclose(group_id);

  // Close the file.
  status = H5Fclose(file_id);
}
#endif //  MINIMIZE_IO

/**
 * @brief   Writes metadata attributes to an HDF5 file
 *
 * @param   n          Snapshot index in ListOutputSnaps
 * @param   filenr     File number to write to
 *
 * This function writes metadata attributes to an HDF5 file after all
 * galaxies have been written. It:
 * 1. Opens the target HDF5 file
 * 2. Navigates to the correct snapshot group
 * 3. Adds attributes such as number of trees and number of galaxies
 * 4. Creates and writes the TreeNgals dataset (galaxies per tree)
 *
 * These attributes are essential for readers to understand the file structure
 * and for tools to navigate and process the galaxy data efficiently.
 */
void write_hdf5_attrs(int n, int filenr) {

  /*
   * Write the HDF5 file attributes.
   */

  herr_t status;
  hid_t file_id, dataset_id, attribute_id, dataspace_id, group_id;
  hsize_t dims;
  char target_group[100];
  char fname[1000];

  // Generate the filename to be opened.
  sprintf(fname, "%s/%s_%03d.hdf5", OutputDir, FileNameGalaxies, filenr);

  // Open the output file and galaxy dataset.
  file_id = H5Fopen(fname, H5F_ACC_RDWR, H5P_DEFAULT);

  // Open the relevant group.
  sprintf(target_group, "Snap%03d", ListOutputSnaps[n]);
  group_id = H5Gopen(file_id, target_group, H5P_DEFAULT);

  dataset_id = H5Dopen(group_id, "Galaxies", H5P_DEFAULT);

  // Create the data space for the attributes.
  dims = 1;
  dataspace_id = H5Screate_simple(1, &dims, NULL);

  // Write the number of trees
  attribute_id = H5Acreate(dataset_id, "Ntrees", H5T_NATIVE_INT, dataspace_id,
                           H5P_DEFAULT, H5P_DEFAULT);
  status = H5Awrite(attribute_id, H5T_NATIVE_INT, &Ntrees);
  status = H5Aclose(attribute_id);

  // Write the total number of galaxies.
  attribute_id = H5Acreate(dataset_id, "TotGalaxies", H5T_NATIVE_INT,
                           dataspace_id, H5P_DEFAULT, H5P_DEFAULT);
  status = H5Awrite(attribute_id, H5T_NATIVE_INT, &TotGalaxies[n]);
  status = H5Aclose(attribute_id);

  // Close the dataspace.
  status = H5Sclose(dataspace_id);

  // Close to the dataset.
  status = H5Dclose(dataset_id);

  // Create an array dataset to hold the number of galaxies per tree and write
  // it.
  dims = Ntrees;
  if (dims <= 0) {
    FATAL_ERROR("Invalid number of trees (Ntrees=%d) in write_hdf5_attrs for "
                "snapshot %d (filenr %d)",
                (int)dims, ListOutputSnaps[n], filenr);
  }
  dataspace_id = H5Screate_simple(1, &dims, NULL);
  dataset_id = H5Dcreate(group_id, "TreeNgals", H5T_NATIVE_INT, dataspace_id,
                         H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  status = H5Dclose(dataset_id);

  // Close the group.
  status = H5Gclose(group_id);

  // Close the file.
  status = H5Fclose(file_id);
}

/**
 * @brief   Stores the simulation parameters as attributes in an HDF5 file
 *
 * @param   master_file_id   HDF5 file ID to write parameters to
 *
 * This function creates a group in the HDF5 file to store all model parameters
 * as attributes. It uses the parameter table to iterate through all parameters
 * and writes their values to the file with appropriate HDF5 data types.
 *
 * The function also adds extra properties such as:
 * - NCores: Number of cores used for the simulation
 * - RunEndTime: Timestamp when the simulation completed
 * - InputSimulation: Name of the input simulation
 *
 * Parameters are retrieved from the SageConfig structure through the parameter
 * table, ensuring that the most current values are stored.
 */
static void store_run_properties(hid_t master_file_id) {
  hid_t props_group_id, dataspace_id, attribute_id, str_type;
  hsize_t dims;
  herr_t status;
  time_t t;
  struct tm *local;
  int i;
  ParameterDefinition *param_table;
  int num_params;

  /* Get the parameter table and its size */
  param_table = get_parameter_table();
  num_params = get_parameter_table_size();

  /* Create the group to hold the run properties */
  props_group_id = H5Gcreate(master_file_id, "RunProperties", H5P_DEFAULT,
                             H5P_DEFAULT, H5P_DEFAULT);

  /* Set up common data structures for attributes */
  dims = 1;
  dataspace_id = H5Screate_simple(1, &dims, NULL);
  str_type = H5Tcopy(H5T_C_S1);
  status = H5Tset_size(str_type, MAX_STRING_LEN);

  /* Store all parameters from the parameter table */
  for (i = 0; i < num_params; i++) {
    /* Skip OutputDir as it might contain sensitive path information */
    if (strcmp(param_table[i].name, "OutputDir") != 0) {
      switch (param_table[i].type) {
      case INT:
        attribute_id =
            H5Acreate(props_group_id, param_table[i].name, H5T_NATIVE_INT,
                      dataspace_id, H5P_DEFAULT, H5P_DEFAULT);
        status = H5Awrite(attribute_id, H5T_NATIVE_INT, param_table[i].address);
        status = H5Aclose(attribute_id);
        break;

      case DOUBLE:
        attribute_id =
            H5Acreate(props_group_id, param_table[i].name, H5T_NATIVE_DOUBLE,
                      dataspace_id, H5P_DEFAULT, H5P_DEFAULT);
        status =
            H5Awrite(attribute_id, H5T_NATIVE_DOUBLE, param_table[i].address);
        status = H5Aclose(attribute_id);
        break;

      case STRING:
        /* Special handling for TreeType which doesn't have a direct address */
        if (strcmp(param_table[i].name, "TreeType") == 0) {
          const char *tree_type_str;
          switch (SageConfig.TreeType) {
          case lhalo_binary:
            tree_type_str = "lhalo_binary";
            break;
          case genesis_lhalo_hdf5:
            tree_type_str = "genesis_lhalo_hdf5";
            break;
          default:
            tree_type_str = "unknown";
          }
          attribute_id =
              H5Acreate(props_group_id, param_table[i].name, str_type,
                        dataspace_id, H5P_DEFAULT, H5P_DEFAULT);
          status = H5Awrite(attribute_id, str_type, tree_type_str);
          status = H5Aclose(attribute_id);
        } else if (param_table[i].address != NULL) {
          attribute_id =
              H5Acreate(props_group_id, param_table[i].name, str_type,
                        dataspace_id, H5P_DEFAULT, H5P_DEFAULT);
          status = H5Awrite(attribute_id, str_type, param_table[i].address);
          status = H5Aclose(attribute_id);
        }
        break;
      }
    }
  }

  /* Add extra properties */
  attribute_id = H5Acreate(props_group_id, "NCores", H5T_NATIVE_INT,
                           dataspace_id, H5P_DEFAULT, H5P_DEFAULT);
  status = H5Awrite(attribute_id, H5T_NATIVE_INT, &NTask);
  status = H5Aclose(attribute_id);

  time(&t);
  local = localtime(&t);
  attribute_id = H5Acreate(props_group_id, "RunEndTime", str_type, dataspace_id,
                           H5P_DEFAULT, H5P_DEFAULT);
  status = H5Awrite(attribute_id, str_type, asctime(local));
  status = H5Aclose(attribute_id);

  /* Add input simulation info if defined */
#ifdef INPUTSIM
  attribute_id = H5Acreate(props_group_id, "InputSimulation", str_type,
                           dataspace_id, H5P_DEFAULT, H5P_DEFAULT);
  status = H5Awrite(attribute_id, str_type, INPUTSIM);
  status = H5Aclose(attribute_id);
#endif

  /* Clean up */
  status = H5Sclose(dataspace_id);
  status = H5Gclose(props_group_id);
}

void write_master_file(void) {

  /*
   * Generate a 'master' file that holds soft links to the data in all of the
   * standard output files.
   */

  int filenr, n, ngal_in_file, ngal_in_core;
  char master_file[1000], target_file[1000];
  char target_group[100], source_ds[100];
  hid_t master_file_id, dataset_id, attribute_id, dataspace_id, group_id,
      target_file_id;
  herr_t status;
  hsize_t dims;
  float redshift;

  // Open the master file.
  sprintf(master_file, "%s/%s.hdf5", OutputDir, FileNameGalaxies);
  DEBUG_LOG("Creating master HDF5 file '%s'", master_file);
  master_file_id =
      H5Fcreate(master_file, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

  printf("\n\nMaking one file to rule them all:\n\t%s\n", master_file);

  // Loop through each snapshot.
  for (n = 0; n < NOUT; n++) {

    // Create a group to hold this snapshot's data
    sprintf(target_group, "Snap%03d", ListOutputSnaps[n]);
    group_id = H5Gcreate(master_file_id, target_group, H5P_DEFAULT, H5P_DEFAULT,
                         H5P_DEFAULT);

    // Save the redshift of this snapshot as an attribute
    dims = 1;
    dataspace_id = H5Screate_simple(1, &dims, NULL);
    attribute_id = H5Acreate(group_id, "Redshift", H5T_NATIVE_FLOAT,
                             dataspace_id, H5P_DEFAULT, H5P_DEFAULT);
    redshift = (float)(ZZ[ListOutputSnaps[n]]);
    status = H5Awrite(attribute_id, H5T_NATIVE_FLOAT, &redshift);
    status = H5Aclose(attribute_id);
    status = H5Sclose(dataspace_id);
    status = H5Gclose(group_id);

    // Loop through each file for this snapshot.
    for (filenr = FirstFile; filenr <= LastFile; filenr++) {
      // Create a group to hold this snapshot's data
      sprintf(target_group, "Snap%03d/File%03d", ListOutputSnaps[n], filenr);
      group_id = H5Gcreate(master_file_id, target_group, H5P_DEFAULT,
                           H5P_DEFAULT, H5P_DEFAULT);
      status = H5Gclose(group_id);

      ngal_in_file = 0;
      // Generate the *relative* path to the actual output file.
      sprintf(target_file, "%s_%03d.hdf5", FileNameGalaxies, filenr);

      // Create a dataset which will act as the soft link to the output
      // galaxies.
      sprintf(target_group, "Snap%03d/File%03d/Galaxies", ListOutputSnaps[n],
              filenr);
      sprintf(source_ds, "Snap%03d/Galaxies", ListOutputSnaps[n]);
      DEBUG_LOG("Creating external DS link - %s", target_group);
      status = H5Lcreate_external(target_file, source_ds, master_file_id,
                                  target_group, H5P_DEFAULT, H5P_DEFAULT);

      // Create a dataset which will act as the soft link to the array storing
      // the number of galaxies per tree for this file.
      sprintf(target_group, "Snap%03d/File%03d/TreeNgals", ListOutputSnaps[n],
              filenr);
      sprintf(source_ds, "Snap%03d/TreeNgals", ListOutputSnaps[n]);
      DEBUG_LOG("Creating external DS link - %s", target_group);
      status = H5Lcreate_external(target_file, source_ds, master_file_id,
                                  target_group, H5P_DEFAULT, H5P_DEFAULT);

      // Increment the total number of galaxies for this file.
      sprintf(target_file, "%s/%s_%03d.hdf5", OutputDir, FileNameGalaxies,
              filenr);
      target_file_id = H5Fopen(target_file, H5F_ACC_RDONLY, H5P_DEFAULT);
      sprintf(source_ds, "Snap%03d/Galaxies", ListOutputSnaps[n]);
      dataset_id = H5Dopen(target_file_id, source_ds, H5P_DEFAULT);
      attribute_id = H5Aopen(dataset_id, "TotGalaxies", H5P_DEFAULT);
      status = H5Aread(attribute_id, H5T_NATIVE_INT, &ngal_in_core);
      status = H5Aclose(attribute_id);
      status = H5Dclose(dataset_id);
      status = H5Fclose(target_file_id);
      ngal_in_file += ngal_in_core;

      // Save the total number of galaxies in this file.
      dims = 1;
      dataspace_id = H5Screate_simple(1, &dims, NULL);
      sprintf(target_group, "Snap%03d/File%03d", ListOutputSnaps[n], filenr);
      group_id = H5Gopen(master_file_id, target_group, H5P_DEFAULT);
      attribute_id = H5Acreate(group_id, "TotGalaxies", H5T_NATIVE_INT,
                               dataspace_id, H5P_DEFAULT, H5P_DEFAULT);
      status = H5Awrite(attribute_id, H5T_NATIVE_INT, &ngal_in_file);
      status = H5Aclose(attribute_id);
      status = H5Gclose(group_id);
      status = H5Sclose(dataspace_id);
    }
  }

#ifdef GITREF_STR
  // Save the git ref if requested
  char tempstr[45];

  dims = 1;
  hid_t str_type = H5Tcopy(H5T_C_S1);
  status = H5Tset_size(str_type, 45);
  dataspace_id = H5Screate_simple(1, &dims, NULL);

  sprintf(tempstr, GITREF_STR);
  attribute_id = H5Acreate(master_file_id, "GitRef", str_type, dataspace_id,
                           H5P_DEFAULT, H5P_DEFAULT);
  status = H5Awrite(attribute_id, str_type, tempstr);

  sprintf(tempstr, MODELNAME);
  attribute_id = H5Acreate(master_file_id, "Model", str_type, dataspace_id,
                           H5P_DEFAULT, H5P_DEFAULT);
  status = H5Awrite(attribute_id, str_type, tempstr);

  status = H5Aclose(attribute_id);
  status = H5Sclose(dataspace_id);
#endif

  // Finally - store the properites of the run...
  store_run_properties(master_file_id);

  // Close the master file.
  H5Fclose(master_file_id);
}

void free_hdf5_ids(void) {

  /*
   * Free any HDF5 objects which are still floating about at the end of the run.
   */
  myfree(HDF5_field_types);
  myfree(HDF5_field_names);
  myfree(HDF5_dst_sizes);
  myfree(HDF5_dst_offsets);
}
