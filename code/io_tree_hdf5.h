#ifndef TREE_HDF5_H
#define TREE_HDF5_H

#ifdef HDF5
#include <hdf5.h>

// Proto-Types //

void load_tree_table_hdf5(int filenr);
void load_tree_hdf5(int32_t filenr, int32_t treenr);
void close_hdf5_file(void);

#endif
#endif
