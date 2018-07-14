#ifndef TREE_HDF5_H 
#define TREE_HDF5_H 

#ifdef __cplusplus
extern "C" {
#endif /* working with c++ compiler */

#include <hdf5.h>


/* Proto-Types */
extern void load_tree_table_hdf5(const int ThisTask, int filenr, int *ntrees, int **treenhalos);
extern void load_tree_hdf5(int32_t treenr, const int32_t nhalos, struct halo_data **halos);
extern void close_hdf5_file(void);

#ifdef __cplusplus
}
#endif /* working with c++ compiler */
