#pragma once

#ifdef __cplusplus
extern "C" {
#endif

    #include "core_allvars.h"
    
    /* core_io_tree.c */
    extern void load_forest_table(const int ThisTask, const int filenr, const enum Valid_TreeTypes my_TreeType, int *nforests, int **forestnhalos);
    extern int load_forest(const int forestnr, const int nhalos, enum Valid_TreeTypes my_TreeType, struct halo_data **halos,
                           struct halo_aux_data **haloaux, struct GALAXY **galaxies, struct GALAXY **halogal);
    extern void free_forest_table(const enum Valid_TreeTypes my_TreeType);

#ifdef __cplusplus
}
#endif
