#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>

#include "../core_allvars.h"
#include "../core_proto.h"
#include "tree_hdf5.h"

// Local Variables //
static hid_t hdf5_file;

// Local Structs //

struct METADATA_NAMES
{
  char name_NTrees[MAX_STRING_LEN+1];
  char name_totNHalos[MAX_STRING_LEN+1];
  char name_TreeNHalos[MAX_STRING_LEN+1];
};

// Local Proto-Types //

int32_t fill_metadata_names(struct METADATA_NAMES *metadata_names, enum Valid_TreeTypes my_TreeType);
int32_t read_attribute_int(hid_t my_hdf5_file, char *groupname, char *attr_name, int *attribute);
int32_t read_dataset(hid_t my_hdf5_file, char *dataset_name, int32_t datatype, void *buffer);

// External Functions //

void load_tree_table_hdf5(int filenr)
{

  char buf[MAX_STRING_LEN+1];
  int32_t totNHalos, i;
  int32_t status;

  struct METADATA_NAMES metadata_names;

  snprintf(buf, MAX_STRING_LEN, "%s/%s.%d%s", SimulationDir, TreeName, filenr, TreeExtension);
  hdf5_file = H5Fopen(buf, H5F_ACC_RDONLY, H5P_DEFAULT);

  if (hdf5_file < 0)
  {
    printf("can't open file `%s'\n", buf);
    ABORT(0);
  }

  status = fill_metadata_names(&metadata_names, TreeType);
  if (status != EXIT_SUCCESS)
  {
    ABORT(0);
  }

  status = read_attribute_int(hdf5_file, "/Header", metadata_names.name_NTrees, &Ntrees);
  if (status != EXIT_SUCCESS)
  {
    fprintf(stderr, "Error while processing file %s\n", buf);
    fprintf(stderr, "Error code is %d\n", status);
    ABORT(0);
  }

  status = read_attribute_int(hdf5_file, "/Header", metadata_names.name_totNHalos, &totNHalos);
  if (status != EXIT_SUCCESS)
  {
    fprintf(stderr, "Error while processing file %s\n", buf);
    fprintf(stderr, "Error code is %d\n", status);
    ABORT(0);
  }

  printf("There are %d trees and %d total halos\n", Ntrees, totNHalos);

  TreeNHalos = mymalloc(sizeof(int) * Ntrees);

  status = read_attribute_int(hdf5_file, "/Header", metadata_names.name_TreeNHalos, TreeNHalos);
  if (status != EXIT_SUCCESS)
  {
    fprintf(stderr, "Error while processing file %s\n", buf);
    fprintf(stderr, "Error code is %d\n", status);
    ABORT(0);
  }

  TreeFirstHalo = mymalloc(sizeof(int) * Ntrees);

  for (i = 0; i < 20; ++i)
    printf("Tree %d: NHalos %d\n", i, TreeNHalos[i]);

  if(Ntrees)
    TreeFirstHalo[0] = 0;
  for(i = 1; i < Ntrees; i++)
    TreeFirstHalo[i] = TreeFirstHalo[i - 1] + TreeNHalos[i - 1];

}

#define READ_TREE_PROPERTY(sage_name, hdf5_name, type_int, data_type) \
{ \
  snprintf(dataset_name, MAX_STRING_LEN, "tree_%03d/%s", treenr, #hdf5_name);\
  status = read_dataset(hdf5_file, dataset_name, type_int, buffer);\
  if (status != EXIT_SUCCESS) \
  {\
    ABORT(0);\
  }\
  for (halo_idx = 0; halo_idx < NHalos_ThisTree; ++halo_idx)\
  {\
    Halo[halo_idx].sage_name = ((data_type*)buffer)[halo_idx];\
  }\
} \

#define READ_TREE_PROPERTY_MULTIPLEDIM(sage_name, hdf5_name, type_int, data_type) \
{ \
  snprintf(dataset_name, MAX_STRING_LEN, "tree_%03d/%s", treenr, #hdf5_name);\
  status = read_dataset(hdf5_file, dataset_name, type_int, buffer_multipledim);\
  if (status != EXIT_SUCCESS) \
  {\
    ABORT(0);\
  }\
  for (halo_idx = 0; halo_idx < NHalos_ThisTree; ++halo_idx)\
  {\
    for (dim = 0; dim < NDIM; ++dim)\
    { \
      Halo[halo_idx].sage_name[dim] = ((data_type*)buffer_multipledim)[halo_idx * NDIM + dim];\
    } \
  }\
} \


void load_tree_hdf5(int32_t filenr, int32_t treenr)
{

  char dataset_name[MAX_STRING_LEN+1];
  int32_t NHalos_ThisTree, status, halo_idx, dim;

  double *buffer; // Buffer to hold the read HDF5 data.
                  // The largest data-type will be double.

  double *buffer_multipledim; // However also need a buffer three times as large to hold data such as position/velocity.

  if (hdf5_file <= 0)
  {
    fprintf(stderr, "The HDF5 file should still be opened when reading the halos in the tree.\n");
    fprintf(stderr, "For tree %d we encountered error %d\n", treenr, hdf5_file);
    ABORT(0);
  }

  NHalos_ThisTree = TreeNHalos[treenr];

  Halo = mymalloc(sizeof(struct halo_data) * NHalos_ThisTree);

  buffer = calloc(NHalos_ThisTree, sizeof(*(buffer)));
  if (buffer == NULL)
  {
    fprintf(stderr, "Could not allocate memory for the HDF5 buffer.\n");
    ABORT(0);
  }

  buffer_multipledim = calloc(NHalos_ThisTree * NDIM, sizeof(*(buffer_multipledim)));
  if (buffer_multipledim == NULL)
  {
    fprintf(stderr, "Could not allocate memory for the HDF5 multiple dimension buffer.\n");
    ABORT(0);
  }

  // We now need to read in all the halo fields for this tree.
  // To do so, we read the field into a buffer and then properly slot the field into the Halo struct.

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
  for (i = 0; i < 20; ++i)
  {
    printf("halo %d: Descendant %d FirstProg %d x %.4f y %.4f z %.4f\n", i, Halo[i].Descendant, Halo[i].FirstProgenitor, Halo[i].Pos[0], Halo[i].Pos[1], Halo[i].Pos[2]);
  }
  ABORT(0);
#endif

}

#undef READ_TREE_PROPERTY
#undef READ_TREE_PROPERTY_MULTIPLEDIM

void close_hdf5_file(void)
{

  H5Fclose(hdf5_file);

}

// Local Functions //

int32_t fill_metadata_names(struct METADATA_NAMES *metadata_names, enum Valid_TreeTypes my_TreeType)
{

  switch (my_TreeType)
  {

    case genesis_lhalo_hdf5:

      snprintf(metadata_names->name_NTrees, MAX_STRING_LEN, "Ntrees"); // Total number of trees within the file.
      snprintf(metadata_names->name_totNHalos, MAX_STRING_LEN, "totNHalos"); // Total number of halos within the file.
      snprintf(metadata_names->name_TreeNHalos, MAX_STRING_LEN, "TreeNHalos"); // Number of halos per tree within the file.
      return EXIT_SUCCESS;

    case lhalo_binary:
      fprintf(stderr, "If the file is binary then this function should never be called.  Something's gone wrong...");
      return EXIT_FAILURE;

    default:
      fprintf(stderr, "Your tree type has not been included in the switch statement for ``fill_metadata_names`` in ``io/tree_hdf5.c``.\n");
      fprintf(stderr, "Please add it there.\n");
      ABORT(EXIT_FAILURE);

  }

  return EXIT_FAILURE;

}

int32_t read_attribute_int(hid_t my_hdf5_file, char *groupname, char *attr_name, int *attribute)
{

  int32_t status;
  hid_t attr_id;

  attr_id = H5Aopen_by_name(my_hdf5_file, groupname, attr_name, H5P_DEFAULT, H5P_DEFAULT);
  if (attr_id < 0)
  {
    fprintf(stderr, "Could not open the attribute %s in group %s\n", attr_name, groupname);
    return attr_id;
  }

  status = H5Aread(attr_id, H5T_NATIVE_INT, attribute);
  if (status < 0)
  {
    fprintf(stderr, "Could not read the attribute %s in group %s\n", attr_name, groupname);
    return status;
  }

  status = H5Aclose(attr_id);
  if (status < 0)
  {
    fprintf(stderr, "Error when closing the file.\n");
    return status;
  }

  return EXIT_SUCCESS;
}

int32_t read_dataset(hid_t my_hdf5_file, char *dataset_name, int32_t datatype, void *buffer)
{
  hid_t dataset_id;

  dataset_id = H5Dopen2(hdf5_file, dataset_name, H5P_DEFAULT);
  if (dataset_id < 0)
  {
    fprintf(stderr, "Error %d when trying to open up dataset %s\n", dataset_id, dataset_name);
    return dataset_id;
  }

  if (datatype == 0)
    H5Dread(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer);
  else if (datatype == 1)
    H5Dread(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer);

  else if (datatype == 2)
    H5Dread(dataset_id, H5T_NATIVE_LLONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer);



  return EXIT_SUCCESS;
}
