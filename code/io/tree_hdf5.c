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

// External Functions //

void load_tree_table_hdf5(int filenr, hid_t my_hdf5_file)
{

  char buf[MAX_STRING_LEN];
  int32_t totNHalos, i;
  int32_t status;

  struct METADATA_NAMES metadata_names;

  sprintf(buf, "%s/%s.%d%s", SimulationDir, TreeName, filenr, TreeExtension);
  my_hdf5_file = H5Fopen(buf, H5F_ACC_RDONLY, H5P_DEFAULT);
  if (my_hdf5_file < 0)
  {
    printf("can't open file `%s'\n", buf);
    ABORT(0);    
  }

  status = fill_metadata_names(&metadata_names, TreeType);
  if (status != EXIT_SUCCESS)
  {
    ABORT(0);
  }
 
  status = read_attribute_int(my_hdf5_file, "/Header", metadata_names.name_NTrees, &Ntrees);
  if (status != EXIT_SUCCESS)
  {
    fprintf(stderr, "Error while processing file %s\n", buf);
    fprintf(stderr, "Error code is %d\n", status);
    ABORT(0);
  }

  status = read_attribute_int(my_hdf5_file, "/Header", metadata_names.name_totNHalos, &totNHalos);
  if (status != EXIT_SUCCESS)  
  {
    fprintf(stderr, "Error while processing file %s\n", buf);
    fprintf(stderr, "Error code is %d\n", status);
    ABORT(0);
  }

  printf("There are %d trees and %d total halos\n", Ntrees, totNHalos);
  
  TreeNHalos = mymalloc(sizeof(int) * Ntrees); 

  status = read_attribute_int(my_hdf5_file, "/Header", metadata_names.name_TreeNHalos, TreeNHalos);
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

void load_tree_hdf5(int32_t filenr, int32_t treenr, hid_t my_hdf5_file)
{

  Halo = mymalloc(sizeof(struct halo_data) * TreeNHalos[treenr]);
   
  printf("Hello\n");
  exit(0);
}

// Local Functions //

int32_t fill_metadata_names(struct METADATA_NAMES *metadata_names, enum Valid_TreeTypes my_TreeType)
{

  switch (my_TreeType)
  {

    case genesis_lhalo_hdf5: 
  
      snprintf(metadata_names->name_NTrees, 1023, "Ntrees"); // Total number of trees within the file.
      snprintf(metadata_names->name_totNHalos, 1023, "totNHalos"); // Total number of halos within the file.
      snprintf(metadata_names->name_TreeNHalos, 1023, "TreeNHalos"); // Number of halos per tree within the file.

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
