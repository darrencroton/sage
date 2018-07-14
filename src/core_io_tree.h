#pragma once

#ifdef __cplusplus
extern "C" {
#endif

    #include "core_allvars.h"
    
    /* core_io_tree.c */
    extern void load_tree_table(const int ThisTask, const int filenr, const enum Valid_TreeTypes my_TreeType, int *ntrees,
                                int **treenhalos, int **treengals, int *totgalaxies);
    extern int load_tree(const int treenr, const int nhalos, enum Valid_TreeTypes my_TreeType, struct halo_data **halos,
                         struct halo_aux_data **haloaux, struct GALAXY **galaxies, struct GALAXY **halogal);
    extern void free_tree_table(const enum Valid_TreeTypes my_TreeType, int **treengals, int *treenhalos);
    extern void free_galaxies_and_tree(struct GALAXY *galaxies, struct GALAXY *halogal, struct halo_aux_data *haloaux, struct halo_data *halos);

#ifdef __cplusplus
}
#endif
