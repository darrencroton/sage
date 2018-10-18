#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* working with c++ compiler */

#include "../core_allvars.h"

    
    /* Proto-Types */
    extern void load_forest_table_ctrees(struct forest_info *forests_info);
    extern void load_forest_ctrees(const int32_t forestnr, const int32_t nhalos, struct halo_data **halos, struct forest_info *forests_info);
    extern void close_ctrees_file(struct forest_info *forests_info);

#ifdef __cplusplus
}
#endif
