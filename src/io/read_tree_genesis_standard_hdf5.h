#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* working with c++ compiler */

#include <hdf5.h>

/* for definition of struct halo_data */    
#include "../core_allvars.h"    

/* Proto-Types */
extern void load_forest_table_genesis_hdf5(const int32_t ThisTask, const int32_t filenr, int32_t *nforests, int32_t **forestnhalos);
extern void load_forest_genesis_hdf5(int32_t forestnr, const int32_t nhalos, struct halo_data **halos);
extern void close_genesis_hdf5_file(void);

#ifdef __cplusplus
}
#endif /* working with c++ compiler */
