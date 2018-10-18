#pragma once

#ifdef __cplusplus
extern "C" {
#endif

    #include "core_allvars.h"
    
    /* functions in model_disk_instability.c */
    extern void check_disk_instability(const int p, const int centralgal, const int halonr, const double time,
                                       const double dt, const int step, struct GALAXY *galaxies);

#ifdef __cplusplus
}
#endif
