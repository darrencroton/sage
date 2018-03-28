#ifndef TREE_HDF5_H 
#define TREE_HDF5_H 

#ifdef HDF5
#include <hdf5.h>

// Proto-Types //

void load_tree_table_hdf5(int filenr, hid_t hdf5_file);

#endif
#endif
