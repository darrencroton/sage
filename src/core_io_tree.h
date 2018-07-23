#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

    #include "core_allvars.h"
    
    /* core_io_tree.c */
    extern void load_forest_table(const enum Valid_TreeTypes my_TreeType, struct forest_info *forests_info);
    extern void load_forest(const int forestnr, const int nhalos, enum Valid_TreeTypes my_TreeType, struct halo_data **halos, struct forest_info *forests_info);
    extern void free_forest_table(const enum Valid_TreeTypes my_TreeType, struct forest_info *forests_info);

#ifdef __cplusplus
}
#endif
