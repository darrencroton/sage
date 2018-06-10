#pragma once

#ifdef __cplusplus
extern "C" {
#endif

    #include "core_allvars.h"

    /* Functions in core_save.c */
    extern void save_galaxies(const int filenr, const int tree, const int ntrees, const int numgals, struct halo_data *halos,
                              struct halo_aux_data *haloaux, struct GALAXY *halogal, int **treengals, int *totgalaxies);
    extern void prepare_galaxy_for_output(int filenr, int tree, struct GALAXY *g, struct GALAXY_OUTPUT *o, struct halo_data *halos,
                                          struct halo_aux_data *haloaux, struct GALAXY *halogal);
    extern void finalize_galaxy_file(const int ntrees, const int *totgalaxies, const int **treengals);
    
#ifdef __cplusplus
}
#endif
