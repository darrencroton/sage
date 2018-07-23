#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* working with c++ compiler */

#include "../core_allvars.h"

    
/* Proto-Types */
extern void load_forest_table_binary(struct forest_info *forests_info);
extern void load_forest_binary(const int32_t nhalos, struct halo_data **halos, struct forest_info *forests_info);
extern void close_binary_file(struct forest_info *forests_info);

#ifdef __cplusplus
}
#endif
