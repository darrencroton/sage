#ifndef TREE_HDF5_H 
#define TREE_HDF5_H 

#ifdef HDF5
#include <hdf5.h>

// Proto-Types //
void load_tree_table_hdf5(int filenr, int *ntrees, int **treenhalos, int **treefirsthalo);
void load_tree_hdf5(int32_t treenr, const int32_t nhalos, struct halo_data **halos);
void close_hdf5_file(void);

#endif
#endif
