#pragma once

#include <stdint.h>
#include "../core_allvars.h"

#ifdef __cplusplus
extern "C" {
#endif /* working with c++ compiler */

/* Proto-Types */
extern void load_tree_table_binary(const int32_t filenr, int *ntrees, int **treenhalos);
extern void load_tree_binary(const int32_t nhalos, struct halo_data **halos, int32_t **orig_index);
extern void close_binary_file(void);

#ifdef __cplusplus
}
#endif
