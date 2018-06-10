#pragma once

#ifdef __cplusplus
extern "C" {
#endif

    #include "core_allvars.h"

    /* functions in model_cooling_heating.c */
    extern double cooling_recipe(const int gal, const double dt, struct GALAXY *galaxies);
    extern double do_AGN_heating(double coolingGas, const int centralgal, const double dt, const double x, const double rcool, struct GALAXY *galaxies);
    extern void cool_gas_onto_galaxy(const int centralgal, const double coolingGas, struct GALAXY *galaxies);
    
#ifdef __cplusplus
}
#endif
