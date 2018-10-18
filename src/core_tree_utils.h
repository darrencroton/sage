#pragma once

#ifdef __cplusplus
extern "C" {
#endif
    
#include <stdint.h>
#include <inttypes.h>
    
#include "core_allvars.h"
    
    /* functions in core_tree_utils.c */
    extern int fix_mergertree_index(struct halo_data *tree, const int64_t nhalos, const int32_t *index);
    extern int reorder_lhalo_to_lhvt(const int32_t nhalos, struct halo_data *tree, const int32_t test, int32_t **orig_index);
    extern void get_nfofs_all_snaps(const struct halo_data *forest, const int nhalos, int *nfofs_all_snaps, const int nsnaps);
    
#ifdef __cplusplus
}
#endif


