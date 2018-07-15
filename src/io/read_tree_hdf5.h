#pragma once

#ifdef __cplusplus
extern "C" {
#endif /* working with c++ compiler */

#include <hdf5.h>

/* for definition of struct halo_data */    
#include "../core_allvars.h"    

/* Proto-Types */
extern void load_forest_table_hdf5(const int ThisTask, int filenr, int *nforests, int **forestnhalos);
extern void load_forest_hdf5(int32_t forestnr, const int32_t nhalos, struct halo_data **halos);
extern void close_hdf5_file(void);

#ifdef __cplusplus
}
#endif /* working with c++ compiler */
