#pragma once

#ifdef __cplusplus
extern "C" {
#endif

    #include "core_allvars.h"
    
    /* core_io_tree.c */
    extern void load_forest_table(const int ThisTask, const int filenr, const enum Valid_TreeTypes my_TreeType, int *nforests,
                                int **forestnhalos, int **forestngals, int *totgalaxies);
    extern int load_forest(const int forestnr, const int nhalos, enum Valid_TreeTypes my_TreeType, struct halo_data **halos,
                         struct halo_aux_data **haloaux, struct GALAXY **galaxies, struct GALAXY **halogal);
    extern void free_forest_table(const enum Valid_TreeTypes my_TreeType, int **forestngals, int *forestnhalos);
    extern void free_galaxies_and_forest(struct GALAXY *galaxies, struct GALAXY *halogal, struct halo_aux_data *haloaux, struct halo_data *halos);

#ifdef __cplusplus
}
#endif
