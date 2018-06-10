#pragma once

#ifdef __cplusplus
extern "C" {
#endif

    #include "core_allvars.h"
    
    /* functions in model_reincorporation.c */
    extern void reincorporate_gas(const int centralgal, const double dt, struct GALAXY *galaxies);

#ifdef __cplusplus
}
#endif
