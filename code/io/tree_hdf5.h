#ifndef TREE_HDF5_H 
#define TREE_HDF5_H 

#ifdef HDF5
#include <hdf5.h>

// Structs //

struct METADATA_NAMES 
{
char name_NTrees[MAX_STRING_LEN];
char name_totNHalos[MAX_STRING_LEN];
char name_TreeNHalos[MAX_STRING_LEN];
}; 

// Proto-Types //

void load_tree_table_hdf5(int filenr, hid_t hdf5_file);

#endif
#endif
