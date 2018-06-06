#pragma once

#include <stdint.h>
#include "../core_allvars.h"

#ifdef __cplusplus
extern "C" {
#if 0
/* just to fool the editor that there is no opening brace*/
} /* unreachable */
#endif    
#endif /* working with c++ compiler */

/* Proto-Types */
void load_tree_table_binary(const int32_t filenr, int *ntrees, int **treenhalos, int **treefirsthalo);
void load_tree_binary(const int32_t nhalos, struct halo_data **halos);
void close_binary_file(void);

#ifdef __cplusplus
}
#endif
