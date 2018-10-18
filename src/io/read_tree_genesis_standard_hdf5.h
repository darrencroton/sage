#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* working with c++ compiler */

#include <hdf5.h>

/* for definition of struct halo_data */    
#include "../core_allvars.h"    

/* Proto-Types */
extern void load_forest_table_genesis_hdf5(struct forest_info *forests_info);
extern void load_forest_genesis_hdf5(int32_t forestnr, const int32_t nhalos, struct halo_data **halos, struct forest_info *forests_info);
extern void close_genesis_hdf5_file(struct forest_info *forests_info);

#ifdef __cplusplus
}
#endif /* working with c++ compiler */
