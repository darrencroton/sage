#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* working with c++ compiler */

#include "../core_allvars.h"

    
/* Proto-Types */
extern void load_forest_table_binary(const int32_t filenr, int32_t *ntrees, int32_t **treenhalos);
extern void load_forest_binary(const int32_t nhalos, struct halo_data **halos, int32_t **orig_index);
extern void close_binary_file(void);

#ifdef __cplusplus
}
#endif
