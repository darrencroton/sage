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
  char name_NTrees[MAX_STRING_LEN];
  char name_totNHalos[MAX_STRING_LEN];
  char name_TreeNHalos[MAX_STRING_LEN];
}; 

// Local Proto-Types //

int32_t fill_metadata_names(struct METADATA_NAMES *metadata_names, enum Valid_TreeTypes my_TreeType);
int32_t read_attribute_int(hid_t my_hdf5_file, char *groupname, char *attr_name, int *attribute);
int32_t read_dataset(hid_t my_hdf5_file, char *dataset_name, int32_t datatype, void *buffer);

// External Functions //

void load_tree_table_hdf5(int filenr)
{

  char buf[MAX_STRING_LEN];
  int32_t totNHalos, i;
  int32_t status;

  struct METADATA_NAMES metadata_names;

  snprintf(buf, "%s/%s.%d%s", MAX_STRING_LEN - 1, SimulationDir, TreeName, filenr, TreeExtension);
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

 // H5Fclose(hdf5_file);

}

void load_tree_hdf5(int32_t filenr, int32_t treenr)
{

  char dataset_name[MAX_STRING_LEN];
  int32_t NHalos_ThisTree, status, halo_idx, dim;

  int32_t *buffer_int;
  float *buffer_float;
  long long *buffer_longlong;

  if (hdf5_file <= 0)
  {
    fprintf(stderr, "The HDF5 file should still be opened when reading the halos in the tree.\n");
    fprintf(stderr, "For tree %d we encountered error %d\n", treenr, hdf5_file);
    ABORT(0);
  }

  NHalos_ThisTree = TreeNHalos[treenr];

  Halo = mymalloc(sizeof(struct halo_data) * NHalos_ThisTree); 

  // We now need to read in all the halo fields for this tree.
  // To do so, we read the field into a buffer and then properly slot the field into the Halo struct.


  // TODO: This is currently a very (VERY) messy way of doing things.
  // Ideally we would just like to specify the field name of the HDF5 file and the corresponding field in the Halo struct.
  // However it's a bit tricky because we would need to account for the case of Position where the HDF5 dataset is Nx3.
  // Furthermore, also need to think about how to do the slotting into the Halo struct.  I don't think this is possibly to do all at once.

  /* Merger Tree Pointers */ 

  // Descendant //
  snprintf(dataset_name, MAX_STRING_LEN - 1, "tree_%03d/Descendant", treenr);
  buffer_int = malloc(sizeof(int32_t) * NHalos_ThisTree);
  status = read_dataset(hdf5_file, dataset_name, 0, buffer_int);
  if (status != EXIT_SUCCESS)
  {
    ABORT(0);
  } 
  for (halo_idx = 0; halo_idx < NHalos_ThisTree; ++halo_idx)
  {
    Halo[halo_idx].Descendant = buffer_int[halo_idx];
  }
  free(buffer_int);

  // First Progenitor //
  snprintf(dataset_name, MAX_STRING_LEN - 1, "tree_%03d/FirstProgenitor", treenr);
  buffer_int = malloc(sizeof(int32_t) * NHalos_ThisTree);
  status = read_dataset(hdf5_file, dataset_name, 0, buffer_int);
  if (status != EXIT_SUCCESS)
  {
    ABORT(0);
  } 
  for (halo_idx = 0; halo_idx < NHalos_ThisTree; ++halo_idx)
  {
    Halo[halo_idx].FirstProgenitor = buffer_int[halo_idx];
  }
  free(buffer_int);

  // Next Progenitor //
  snprintf(dataset_name, MAX_STRING_LEN - 1, "tree_%03d/NextProgenitor", treenr);
  buffer_int = malloc(sizeof(int32_t) * NHalos_ThisTree);
  status = read_dataset(hdf5_file, dataset_name, 0, buffer_int);
  if (status != EXIT_SUCCESS)
  {
    ABORT(0);
  } 
  for (halo_idx = 0; halo_idx < NHalos_ThisTree; ++halo_idx)
  {
    Halo[halo_idx].NextProgenitor = buffer_int[halo_idx];
  }
  free(buffer_int);

  // FirstHaloInFOFgroup//
  snprintf(dataset_name, MAX_STRING_LEN - 1, "tree_%03d/FirstHaloInFOFgroup", treenr);
  buffer_int = malloc(sizeof(int32_t) * NHalos_ThisTree);
  status = read_dataset(hdf5_file, dataset_name, 0, buffer_int);
  if (status != EXIT_SUCCESS)
  {
    ABORT(0);
  } 
  for (halo_idx = 0; halo_idx < NHalos_ThisTree; ++halo_idx)
  {
    Halo[halo_idx].FirstHaloInFOFgroup = buffer_int[halo_idx];
  }
  free(buffer_int);

  // NextHaloInFOFgroup//
  snprintf(dataset_name, MAX_STRING_LEN - 1, "tree_%03d/NextHaloInFOFgroup", treenr);
  buffer_int = malloc(sizeof(int32_t) * NHalos_ThisTree);
  status = read_dataset(hdf5_file, dataset_name, 0, buffer_int);
  if (status != EXIT_SUCCESS)
  {
    ABORT(0);
  } 
  for (halo_idx = 0; halo_idx < NHalos_ThisTree; ++halo_idx)
  {
    Halo[halo_idx].NextHaloInFOFgroup= buffer_int[halo_idx];
  }
  free(buffer_int);

  /* Halo Properties */ 

  // Len //
  snprintf(dataset_name, MAX_STRING_LEN - 1, "tree_%03d/Len", treenr);
  buffer_int = malloc(sizeof(int32_t) * NHalos_ThisTree);
  status = read_dataset(hdf5_file, dataset_name, 0, buffer_int);
  if (status != EXIT_SUCCESS)
  {
    ABORT(0);
  } 
  for (halo_idx = 0; halo_idx < NHalos_ThisTree; ++halo_idx)
  {
    Halo[halo_idx].Len= buffer_int[halo_idx];
  }
  free(buffer_int);

  // M_mean200 //
  snprintf(dataset_name, MAX_STRING_LEN - 1, "tree_%03d/M_mean200", treenr);
  buffer_float = malloc(sizeof(float) * NHalos_ThisTree);
  status = read_dataset(hdf5_file, dataset_name, 1, buffer_float);
  if (status != EXIT_SUCCESS)
  {
    ABORT(0);
  } 
  for (halo_idx = 0; halo_idx < NHalos_ThisTree; ++halo_idx)
  {
    Halo[halo_idx].M_Mean200 = buffer_float[halo_idx];
  }
  free(buffer_float);

  // Mvir //
  snprintf(dataset_name, MAX_STRING_LEN - 1, "tree_%03d/Mvir", treenr);
  buffer_float = malloc(sizeof(float) * NHalos_ThisTree);
  status = read_dataset(hdf5_file, dataset_name, 1, buffer_float);
  if (status != EXIT_SUCCESS)
  {
    ABORT(0);
  } 
  for (halo_idx = 0; halo_idx < NHalos_ThisTree; ++halo_idx)
  {
    Halo[halo_idx].Mvir = buffer_float[halo_idx];
  }
  free(buffer_float);

  // M_TopHat //
  snprintf(dataset_name, MAX_STRING_LEN - 1, "tree_%03d/M_TopHat", treenr);
  buffer_float = malloc(sizeof(float) * NHalos_ThisTree);
  status = read_dataset(hdf5_file, dataset_name, 1, buffer_float);
  if (status != EXIT_SUCCESS)
  {
    ABORT(0);
  } 
  for (halo_idx = 0; halo_idx < NHalos_ThisTree; ++halo_idx)
  {
    Halo[halo_idx].M_TopHat = buffer_float[halo_idx];
  }
  free(buffer_float);

  // Pos //
  snprintf(dataset_name, MAX_STRING_LEN - 1, "tree_%03d/Pos", treenr);
  buffer_float = malloc(sizeof(float) * NHalos_ThisTree * 3);
  status = read_dataset(hdf5_file, dataset_name, 1, buffer_float);
  if (status != EXIT_SUCCESS)
  {
    ABORT(0);
  } 
  for (halo_idx = 0; halo_idx < NHalos_ThisTree; ++halo_idx)
  {
    for (dim = 0; dim < NDIM; ++dim)
    {    
      Halo[halo_idx].Pos[dim]= buffer_float[halo_idx * NDIM + dim];
    }
  }
  free(buffer_float);

  // Vel //
  snprintf(dataset_name, MAX_STRING_LEN - 1, "tree_%03d/Vel", treenr);
  buffer_float = malloc(sizeof(float) * NHalos_ThisTree * 3);
  status = read_dataset(hdf5_file, dataset_name, 1, buffer_float);
  if (status != EXIT_SUCCESS)
  {
    ABORT(0);
  } 
  for (halo_idx = 0; halo_idx < NHalos_ThisTree; ++halo_idx)
  {
    for (dim = 0; dim < NDIM; ++dim)
    {    
      Halo[halo_idx].Vel[dim]= buffer_float[halo_idx * NDIM + dim];
    }
  }
  free(buffer_float);

  // VelDisp //
  snprintf(dataset_name, MAX_STRING_LEN - 1, "tree_%03d/VelDisp", treenr);
  buffer_float = malloc(sizeof(float) * NHalos_ThisTree);
  status = read_dataset(hdf5_file, dataset_name, 1, buffer_float);
  if (status != EXIT_SUCCESS)
  {
    ABORT(0);
  } 
  for (halo_idx = 0; halo_idx < NHalos_ThisTree; ++halo_idx)
  {
    Halo[halo_idx].VelDisp = buffer_float[halo_idx];
  }
  free(buffer_float);

  // Vmax //
  snprintf(dataset_name, MAX_STRING_LEN - 1, "tree_%03d/Vmax", treenr);
  buffer_float = malloc(sizeof(float) * NHalos_ThisTree);
  status = read_dataset(hdf5_file, dataset_name, 1, buffer_float);
  if (status != EXIT_SUCCESS)
  {
    ABORT(0);
  } 
  for (halo_idx = 0; halo_idx < NHalos_ThisTree; ++halo_idx)
  {
    Halo[halo_idx].Vmax = buffer_float[halo_idx];
  }
  free(buffer_float);

  // Spin //
  snprintf(dataset_name, MAX_STRING_LEN - 1, "tree_%03d/Spin", treenr);
  buffer_float = malloc(sizeof(float) * NHalos_ThisTree * 3);
  status = read_dataset(hdf5_file, dataset_name, 1, buffer_float);
  if (status != EXIT_SUCCESS)
  {
    ABORT(0);
  } 
  for (halo_idx = 0; halo_idx < NHalos_ThisTree; ++halo_idx)
  {
    for (dim = 0; dim < NDIM; ++dim)
    {    
      Halo[halo_idx].Spin[dim]= buffer_float[halo_idx * NDIM + dim];
    }
  }
  free(buffer_float);

  // MostBoundID //
  snprintf(dataset_name, MAX_STRING_LEN - 1, "tree_%03d/MostBoundID", treenr);
  buffer_longlong = malloc(sizeof(long long) * NHalos_ThisTree);
  status = read_dataset(hdf5_file, dataset_name, 2, buffer_longlong);
  if (status != EXIT_SUCCESS)
  {
    ABORT(0);
  } 
  for (halo_idx = 0; halo_idx < NHalos_ThisTree; ++halo_idx)
  {
    Halo[halo_idx].MostBoundID = buffer_longlong[halo_idx];
  }
  free(buffer_longlong);

  /* Position in Original Files */

  // SnapNum //
  snprintf(dataset_name, MAX_STRING_LEN - 1, "tree_%03d/SnapNum", treenr);
  buffer_int = malloc(sizeof(int32_t) * NHalos_ThisTree);
  status = read_dataset(hdf5_file, dataset_name, 0, buffer_int);
  if (status != EXIT_SUCCESS)
  {
    ABORT(0);
  } 
  for (halo_idx = 0; halo_idx < NHalos_ThisTree; ++halo_idx)
  {
    Halo[halo_idx].SnapNum = buffer_int[halo_idx];
  }
  free(buffer_int);

  // FileNr //
  snprintf(dataset_name, MAX_STRING_LEN - 1, "tree_%03d/Filenr", treenr);
  buffer_int = malloc(sizeof(int32_t) * NHalos_ThisTree);
  status = read_dataset(hdf5_file, dataset_name, 0, buffer_int);
  if (status != EXIT_SUCCESS)
  {
    ABORT(0);
  } 
  for (halo_idx = 0; halo_idx < NHalos_ThisTree; ++halo_idx)
  {
    Halo[halo_idx].FileNr = buffer_int[halo_idx];
  }
  free(buffer_int);

  // SubhaloIndex //
  snprintf(dataset_name, MAX_STRING_LEN - 1, "tree_%03d/SubHaloIndex", treenr);
  buffer_int = malloc(sizeof(int32_t) * NHalos_ThisTree);
  status = read_dataset(hdf5_file, dataset_name, 0, buffer_int);
  if (status != EXIT_SUCCESS)
  {
    ABORT(0);
  } 
  for (halo_idx = 0; halo_idx < NHalos_ThisTree; ++halo_idx)
  {
    Halo[halo_idx].SubhaloIndex = buffer_int[halo_idx];
  }
  free(buffer_int);

  // SubHalfMass //
  snprintf(dataset_name, MAX_STRING_LEN - 1, "tree_%03d/SubHalfMass", treenr);
  buffer_float = malloc(sizeof(float) * NHalos_ThisTree);
  status = read_dataset(hdf5_file, dataset_name, 1, buffer_float);
  if (status != EXIT_SUCCESS)
  {
    ABORT(0);
  } 
  for (halo_idx = 0; halo_idx < NHalos_ThisTree; ++halo_idx)
  {
    Halo[halo_idx].SubHalfMass= buffer_float[halo_idx];
  }
  free(buffer_float);

#ifdef DEBUG_HDF5_READER 
  int32_t i;
  for (i = 0; i < 20; ++i)
  { 
    printf("halo %d: Descendant %d FirstProg %d x %.4f y %.4f z %.4f\n", i, Halo[i].Descendant, Halo[i].FirstProgenitor, Halo[i].Pos[0], Halo[i].Pos[1], Halo[i].Pos[2]); 
  }
#endif 
 
}

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
  
      snprintf(metadata_names->name_NTrees, MAX_STRING_LEN - 1, "Ntrees"); // Total number of trees within the file.
      snprintf(metadata_names->name_totNHalos, MAX_STRING_LEN - 1, "totNHalos"); // Total number of halos within the file.
      snprintf(metadata_names->name_TreeNHalos, MAX_STRING_LEN - 1, "TreeNHalos"); // Number of halos per tree within the file.

      break;

    case lhalo_binary: 
      fprintf(stderr, "If the file is binary then this function should never be called.  Something's gone wrong...");
      return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
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
