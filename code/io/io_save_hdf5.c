#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <hdf5.h>

#include "io_save_hdf5.h"
#include "../core_proto.h"

#define TRUE  1
#define FALSE 0

void calc_hdf5_props(void)
{

  /*
   * Prepare an HDF5 to receive the output galaxy data.
   * Here we store the data in an hdf5 table for easily appending new data.
   */

  struct GALAXY_OUTPUT   galout;

	int                    i;  // dummy

  // If we are calculating any magnitudes then increment the number of
  // output properties appropriately.
  HDF5_n_props = 36;

  // Size of a single galaxy entry.
  HDF5_dst_size = sizeof(struct GALAXY_OUTPUT);

  // Create datatypes for different size arrays
  hid_t array3f_tid = H5Tarray_create(H5T_NATIVE_FLOAT, 1, (hsize_t[]){3});

  // Calculate the offsets of our struct members in memory
  HDF5_dst_offsets     = mymalloc(sizeof(size_t)*HDF5_n_props);
  // Calculate the sizes of our struct members in memory.
  HDF5_dst_sizes       = mymalloc(sizeof(size_t)*HDF5_n_props);
  // Give each galaxy property a field name in the table
  HDF5_field_names     = mymalloc(sizeof(const char*)*HDF5_n_props);
  // Assign a type to each galaxy property field in the table.
  HDF5_field_types     = mymalloc(sizeof(hid_t)*HDF5_n_props);

  i=0;  // Initialise dummy counter

  // Go through each galaxy property and calculate everything we need...
  HDF5_dst_offsets[i]  = HOFFSET(struct GALAXY_OUTPUT, Type);
  HDF5_dst_sizes[i]    = sizeof(galout.Type);
  HDF5_field_names[i]  = "Type";
  HDF5_field_types[i++]  = H5T_NATIVE_INT;

  HDF5_dst_offsets[i]  = HOFFSET(struct GALAXY_OUTPUT, GalaxyIndex);
  HDF5_dst_sizes[i]    = sizeof(galout.GalaxyIndex);
  HDF5_field_names[i]  = "GalaxyIndex";
  HDF5_field_types[i++]  = H5T_NATIVE_LLONG;

  HDF5_dst_offsets[i]  = HOFFSET(struct GALAXY_OUTPUT, HaloIndex);
  HDF5_dst_sizes[i]    = sizeof(galout.HaloIndex);
  HDF5_field_names[i]  = "HaloIndex";
  HDF5_field_types[i++]  = H5T_NATIVE_INT;

  HDF5_dst_offsets[i]  = HOFFSET(struct GALAXY_OUTPUT, FOFHaloIndex);
  HDF5_dst_sizes[i]    = sizeof(galout.FOFHaloIndex);
  HDF5_field_names[i]  = "FOFHaloIndex";
  HDF5_field_types[i++]  = H5T_NATIVE_INT;

  HDF5_dst_offsets[i]  = HOFFSET(struct GALAXY_OUTPUT, TreeIndex);
  HDF5_dst_sizes[i]    = sizeof(galout.TreeIndex);
  HDF5_field_names[i]  = "TreeIndex";
  HDF5_field_types[i++]  = H5T_NATIVE_INT;

  HDF5_dst_offsets[i]  = HOFFSET(struct GALAXY_OUTPUT, SnapNum);
  HDF5_dst_sizes[i]    = sizeof(galout.SnapNum);
  HDF5_field_names[i]  = "SnapNum";
  HDF5_field_types[i++]  = H5T_NATIVE_INT;

  HDF5_dst_offsets[i]  = HOFFSET(struct GALAXY_OUTPUT, CentralGal);
  HDF5_dst_sizes[i]    = sizeof(galout.CentralGal);
  HDF5_field_names[i]  = "CentralGal";
  HDF5_field_types[i++]  = H5T_NATIVE_INT;

  HDF5_dst_offsets[i]  = HOFFSET(struct GALAXY_OUTPUT, CentralMvir);
  HDF5_dst_sizes[i]    = sizeof(galout.CentralMvir);
  HDF5_field_names[i]  = "CentralMvir";
  HDF5_field_types[i++]  = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i]  = HOFFSET(struct GALAXY_OUTPUT, Pos);
  HDF5_dst_sizes[i]    = sizeof(galout.Pos);
  HDF5_field_names[i]  = "Pos";
  HDF5_field_types[i++]  = array3f_tid;

  HDF5_dst_offsets[i]  = HOFFSET(struct GALAXY_OUTPUT, Vel);
  HDF5_dst_sizes[i]    = sizeof(galout.Vel);
  HDF5_field_names[i]  = "Vel";
  HDF5_field_types[i++]  = array3f_tid;

  HDF5_dst_offsets[i]  = HOFFSET(struct GALAXY_OUTPUT, Spin);
  HDF5_dst_sizes[i]    = sizeof(galout.Spin);
  HDF5_field_names[i]  = "Spin";
  HDF5_field_types[i++]  = array3f_tid;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, Len);
  HDF5_dst_sizes[i]   = sizeof(galout.Len);
  HDF5_field_names[i] = "Len";
  HDF5_field_types[i++] = H5T_NATIVE_INT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, Mvir);
  HDF5_dst_sizes[i]   = sizeof(galout.Mvir);
  HDF5_field_names[i] = "Mvir";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, Rvir);
  HDF5_dst_sizes[i]   = sizeof(galout.Rvir);
  HDF5_field_names[i] = "Rvir";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, Vvir);
  HDF5_dst_sizes[i]   = sizeof(galout.Vvir);
  HDF5_field_names[i] = "Vvir";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, Vmax);
  HDF5_dst_sizes[i]   = sizeof(galout.Vmax);
  HDF5_field_names[i] = "Vmax";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, VelDisp);
  HDF5_dst_sizes[i]   = sizeof(galout.VelDisp);
  HDF5_field_names[i] = "VelDisp";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, ColdGas);
  HDF5_dst_sizes[i]   = sizeof(galout.ColdGas);
  HDF5_field_names[i] = "ColdGas";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, StellarMass);
  HDF5_dst_sizes[i]   = sizeof(galout.StellarMass);
  HDF5_field_names[i] = "StellarMass";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, BulgeMass);
  HDF5_dst_sizes[i]   = sizeof(galout.BulgeMass);
  HDF5_field_names[i] = "BulgeMass";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, HotGas);
  HDF5_dst_sizes[i]   = sizeof(galout.HotGas);
  HDF5_field_names[i] = "HotGas";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, EjectedMass);
  HDF5_dst_sizes[i]   = sizeof(galout.EjectedMass);
  HDF5_field_names[i] = "EjectedMass";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, BlackHoleMass);
  HDF5_dst_sizes[i]   = sizeof(galout.BlackHoleMass);
  HDF5_field_names[i] = "BlackHoleMass";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, ICS);
  HDF5_dst_sizes[i]   = sizeof(galout.ICS);
  HDF5_field_names[i] = "ICS";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, MetalsColdGas);
  HDF5_dst_sizes[i]   = sizeof(galout.MetalsColdGas);
  HDF5_field_names[i] = "MetalsColdGas";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, MetalsStellarMass);
  HDF5_dst_sizes[i]   = sizeof(galout.MetalsStellarMass);
  HDF5_field_names[i] = "MetalsStellarMass";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, MetalsBulgeMass);
  HDF5_dst_sizes[i]   = sizeof(galout.MetalsBulgeMass);
  HDF5_field_names[i] = "MetalsBulgeMass";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, MetalsHotGas);
  HDF5_dst_sizes[i]   = sizeof(galout.MetalsHotGas);
  HDF5_field_names[i] = "MetalsHotGas";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, MetalsEjectedMass);
  HDF5_dst_sizes[i]   = sizeof(galout.MetalsEjectedMass);
  HDF5_field_names[i] = "MetalsEjectedMass";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, MetalsICS);
  HDF5_dst_sizes[i]   = sizeof(galout.MetalsICS);
  HDF5_field_names[i] = "MetalsICS";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, Sfr);
  HDF5_dst_sizes[i]   = sizeof(galout.Sfr);
  HDF5_field_names[i] = "Sfr";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, SfrBulge);
  HDF5_dst_sizes[i]   = sizeof(galout.SfrBulge);
  HDF5_field_names[i] = "SfrBulge";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, SfrICS);
  HDF5_dst_sizes[i]   = sizeof(galout.SfrICS);
  HDF5_field_names[i] = "SfrICS";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, DiskScaleRadius);
  HDF5_dst_sizes[i]   = sizeof(galout.DiskScaleRadius);
  HDF5_field_names[i] = "DiskScaleRadius";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, Cooling);
  HDF5_dst_sizes[i]   = sizeof(galout.Cooling);
  HDF5_field_names[i] = "Cooling";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  HDF5_dst_offsets[i] = HOFFSET(struct GALAXY_OUTPUT, Heating);
  HDF5_dst_sizes[i]   = sizeof(galout.Heating);
  HDF5_field_names[i] = "Heating";
  HDF5_field_types[i++] = H5T_NATIVE_FLOAT;

  // DEBUG
  if(i != HDF5_n_props){
    fprintf(stderr, "Incorrect number of galaxy properties in HDF5 file!\n");
    exit(EXIT_FAILURE);
  }

}



void prep_hdf5_file(char *fname)
{

  hsize_t  chunk_size = 1000;  // This value can have a significant impact on read performance!
  int     *fill_data  = NULL;
  hid_t    file_id, snap_group_id;
  char     target_group[100];
  hid_t    status;
  int      i_snap;

  // Create a new file
  file_id = H5Fcreate( fname, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT );

  // Create a group for each output snapshot
  for (i_snap=0; i_snap<NOUT; i_snap++) {
    sprintf(target_group, "Snap%03d", ListOutputSnaps[i_snap]);
    snap_group_id = H5Gcreate(file_id, target_group, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    // Make the table
    status=H5TBmake_table( "Galaxy Table", snap_group_id, "Galaxies",HDF5_n_props,0,
        HDF5_dst_size,HDF5_field_names, HDF5_dst_offsets, HDF5_field_types,
        chunk_size, fill_data, 0, NULL );

    H5Gclose(snap_group_id);
  }

  // Close the HDF5 file.
  status = H5Fclose(file_id);

}


void write_hdf5_galaxy(struct GALAXY_OUTPUT *galaxy_output, int n, int filenr)
{

  /*
   * Write a single galaxy to the hdf5 file table.
   */

  herr_t status;
  hid_t  file_id, group_id;
  char   target_group[100];
  char   fname[1000];

  // Generate the filename to be opened.
  sprintf(fname, "%s/%s_%03d.hdf5", OutputDir, FileNameGalaxies, filenr);

  // Open the file.
  file_id = H5Fopen(fname, H5F_ACC_RDWR, H5P_DEFAULT);

  // Open the relevant group.
  sprintf(target_group, "Snap%03d", ListOutputSnaps[n]);
  group_id = H5Gopen(file_id, target_group, H5P_DEFAULT);

  // Write the galaxy.
  status = H5TBappend_records(group_id, "Galaxies", 1, HDF5_dst_size, HDF5_dst_offsets, HDF5_dst_sizes,
  galaxy_output);

  // Close the group
  status = H5Gclose(group_id);

  // Close the file.
  status = H5Fclose(file_id);

}


#ifdef MINIMIZE_IO
void write_hdf5_galsnap_data(int n, int filenr)
{

  /*
   * Write a batch of galaxies to the output HDF5 table.
   */

  herr_t status;
  hid_t  file_id, group_id;
  char   target_group[100];
  char   fname[1000];

  // Generate the filename to be opened.
  sprintf(fname, "%s/%s_%03d.hdf5", OutputDir, FileNameGalaxies, filenr);

  // Open the file.
  file_id = H5Fopen(fname, H5F_ACC_RDWR, H5P_DEFAULT);

  // Open the relevant group.
  sprintf(target_group, "Snap%03d", ListOutputSnaps[n]);
  group_id = H5Gopen(file_id, target_group, H5P_DEFAULT);

  // Write the galaxies.
  if (TotGalaxies[n]>0){
    status=H5TBappend_records(group_id, "Galaxies", (hsize_t)(TotGalaxies[n]),
        HDF5_dst_size, HDF5_dst_offsets, HDF5_dst_sizes,
        (struct GALAXY_OUTPUT *)(ptr_galsnapdata[n]+offset_galsnapdata[n]));
  } 

  // Close the group.
  status = H5Gclose(group_id);

  // Close the file.
  status = H5Fclose(file_id);

}
#endif  //  MINIMIZE_IO


void write_hdf5_attrs(int n, int filenr)
{

  /*
   * Write the HDF5 file attributes.
   */

  herr_t  status;
  hid_t   file_id, dataset_id, attribute_id, dataspace_id, group_id;
  hsize_t dims;
  char    target_group[100];
  char    fname[1000];

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
  attribute_id = H5Acreate(dataset_id, "Ntrees", H5T_NATIVE_INT, dataspace_id, H5P_DEFAULT, H5P_DEFAULT);
  status = H5Awrite(attribute_id, H5T_NATIVE_INT, &Ntrees);
  status = H5Aclose(attribute_id);

  // Write the total number of galaxies.
  attribute_id = H5Acreate(dataset_id, "TotGalaxies", H5T_NATIVE_INT, dataspace_id, H5P_DEFAULT, H5P_DEFAULT);
  status = H5Awrite(attribute_id, H5T_NATIVE_INT, &TotGalaxies[n]);
  status = H5Aclose(attribute_id);

  // Close the dataspace.
  status = H5Sclose(dataspace_id);

  // Close to the dataset.
  status = H5Dclose(dataset_id);

  // Create an array dataset to hold the number of galaxies per tree and write it.
  dims = Ntrees;
  if (dims<=0){
    fprintf(stderr, "WTF? Ntrees=%d in write_hdf5_attrs !?!\n", (int)dims);
    ABORT(EXIT_FAILURE);
  }
  dataspace_id = H5Screate_simple(1, &dims, NULL);
  dataset_id = H5Dcreate(group_id, "TreeNgals", H5T_NATIVE_INT, dataspace_id, H5P_DEFAULT,
                          H5P_DEFAULT, H5P_DEFAULT);
  status = H5Dclose(dataset_id);

  // Close the group.
  status = H5Gclose(group_id);

  // Close the file.
  status = H5Fclose(file_id);

}

static void store_run_properties(hid_t master_file_id)
{

  /*
   * Store the properties of this run in the master HDF5 file.
   */

  hid_t props_group_id, dataspace_id, attribute_id, str_type;
  hsize_t dims;
  herr_t status;
  time_t t;
  struct tm *local;
  int i;

  // Create the group to hold the run properties.
  props_group_id = H5Gcreate(master_file_id, "RunProperties", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  // Store all of the properties we want as attributes
  dims = 1;
  dataspace_id = H5Screate_simple(1, &dims, NULL);
  str_type = H5Tcopy(H5T_C_S1);
  status = H5Tset_size(str_type, 100);

  for (i = 0; i < NParam; i++) {
    // Ignore the OutputDir, DefaultsFile and RequestedMagBands tags.
    if (strcmp(ParamTag[i], "OutputDir")!=0)
    {
      switch (ParamID[i])
      {
        case INT:
          attribute_id = H5Acreate(props_group_id, ParamTag[i], H5T_NATIVE_INT, dataspace_id, H5P_DEFAULT, H5P_DEFAULT);
          status = H5Awrite(attribute_id, H5T_NATIVE_INT, ParamAddr[i]);
          status = H5Aclose(attribute_id);
          break;

        case DOUBLE:
          attribute_id = H5Acreate(props_group_id, ParamTag[i], H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT);
          status = H5Awrite(attribute_id, H5T_NATIVE_DOUBLE, ParamAddr[i]);
          status = H5Aclose(attribute_id);
          break;

        case STRING:
          attribute_id = H5Acreate(props_group_id, ParamTag[i], str_type, dataspace_id, H5P_DEFAULT, H5P_DEFAULT);
          status = H5Awrite(attribute_id, str_type, ParamAddr[i]);
          status = H5Aclose(attribute_id);
          break;
      }
    }
  }
  
  // Add some extra properties
  attribute_id = H5Acreate(props_group_id, "NCores", H5T_NATIVE_INT, dataspace_id,
                            H5P_DEFAULT, H5P_DEFAULT);
  status = H5Awrite(attribute_id, H5T_NATIVE_INT, &NTask);
  status = H5Aclose(attribute_id);

  local = localtime(&t);
  attribute_id = H5Acreate(props_group_id, "RunEndTime", str_type, dataspace_id,
                            H5P_DEFAULT, H5P_DEFAULT);
  status = H5Awrite(attribute_id, str_type, asctime(local));
  status = H5Aclose(attribute_id);

  attribute_id = H5Acreate(props_group_id, "InputSimulation", str_type, dataspace_id,
                            H5P_DEFAULT, H5P_DEFAULT);
  status = H5Awrite(attribute_id, str_type, INPUTSIM);
  status = H5Aclose(attribute_id);

  status = H5Sclose(dataspace_id);
  status = H5Gclose(props_group_id);
}

void write_master_file(void)
{

  /*
   * Generate a 'master' file that holds soft links to the data in all of the standard
   * output files.
   */

  int     filenr              , n, ngal_in_file, ngal_in_core;
  char    master_file[1000]   , target_file[1000];
  char    target_group[100]   , source_ds[100];
  hid_t   master_file_id      , dataset_id, attribute_id, dataspace_id, group_id, target_file_id;
  herr_t  status;
  hsize_t dims;
  float   redshift;

  // Open the master file.
  sprintf(master_file, "%s/%s.hdf5", OutputDir, FileNameGalaxies);
  master_file_id = H5Fcreate(master_file, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

  printf("\n\nMaking one file to rule them all:\n\t%s\n", master_file);

  // Loop through each snapshot.
  for(n = 0; n < NOUT; n++) {

    // Create a group to hold this snapshot's data
    sprintf(target_group, "Snap%03d", ListOutputSnaps[n]);
    group_id = H5Gcreate(master_file_id, target_group, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    // Save the redshift of this snapshot as an attribute
    dims = 1;
    dataspace_id = H5Screate_simple(1, &dims, NULL);
    attribute_id = H5Acreate(group_id, "Redshift", H5T_NATIVE_FLOAT, dataspace_id, H5P_DEFAULT, H5P_DEFAULT);
    redshift = (float)(ZZ[ListOutputSnaps[n]]);
    status = H5Awrite(attribute_id, H5T_NATIVE_FLOAT, &redshift);
    status = H5Aclose(attribute_id);
    status = H5Sclose(dataspace_id);
    status = H5Gclose(group_id);

    // Loop through each file for this snapshot.
    for(filenr = FirstFile; filenr <= LastFile; filenr++)
    {
      // Create a group to hold this snapshot's data
      sprintf(target_group, "Snap%03d/File%03d", ListOutputSnaps[n], filenr);
      group_id = H5Gcreate(master_file_id, target_group, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
      status = H5Gclose(group_id);

      ngal_in_file = 0;
      // Generate the *relative* path to the actual output file.
      sprintf(target_file, "%s_%03d.hdf5", FileNameGalaxies, filenr);

      // Create a dataset which will act as the soft link to the output galaxies.
      sprintf(target_group, "Snap%03d/File%03d/Galaxies", ListOutputSnaps[n], filenr);
      sprintf(source_ds, "Snap%03d/Galaxies", ListOutputSnaps[n]);
      // printf("    Creating external DS link -  %s\n", target_group);
      status = H5Lcreate_external(target_file, source_ds, master_file_id, target_group, H5P_DEFAULT, H5P_DEFAULT);

      // Create a dataset which will act as the soft link to the array storing the number of galaxies per tree for this file.
      sprintf(target_group, "Snap%03d/File%03d/TreeNgals", ListOutputSnaps[n], filenr);
      sprintf(source_ds, "Snap%03d/TreeNgals", ListOutputSnaps[n]);
      // printf("    Creating external DS link -  %s\n", target_group);
      status = H5Lcreate_external(target_file, source_ds, master_file_id, target_group, H5P_DEFAULT, H5P_DEFAULT);

      // Increment the total number of galaxies for this file.
      sprintf(target_file, "%s/%s_%03d.hdf5", OutputDir,
          FileNameGalaxies, filenr);
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
      attribute_id = H5Acreate(group_id, "TotGalaxies", H5T_NATIVE_INT, dataspace_id, H5P_DEFAULT, H5P_DEFAULT);
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
  attribute_id = H5Acreate(master_file_id, "GitRef", str_type, dataspace_id, H5P_DEFAULT, H5P_DEFAULT);
  status = H5Awrite(attribute_id, str_type, tempstr);

  sprintf(tempstr, MODELNAME);
  attribute_id = H5Acreate(master_file_id, "Model", str_type, dataspace_id, H5P_DEFAULT, H5P_DEFAULT);
  status = H5Awrite(attribute_id, str_type, tempstr);

  status = H5Aclose(attribute_id);
  status = H5Sclose(dataspace_id);
#endif

  // Finally - store the properites of the run...
  store_run_properties(master_file_id);

  // Close the master file.
  H5Fclose(master_file_id);

}

void free_hdf5_ids(void)
{

  /*
   * Free any HDF5 objects which are still floating about at the end of the run.
   */
  myfree(HDF5_field_types);
  myfree(HDF5_field_names);
  myfree(HDF5_dst_sizes);
  myfree(HDF5_dst_offsets);

}

