#ifndef TREE_HDF5_H 
#define TREE_HDF5_H 

#ifdef HDF5
#include <hdf5.h>

// Proto-Types //

void load_tree_table_hdf5(int filenr, hid_t my_hdf5_file);
void load_tree_hdf5(int32_t filenr, int32_t treenr, hid_t my_hdf5_file);

#endif
#endif
