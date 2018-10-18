#pragma once

#ifdef __cplusplus
extern "C" {
#endif

    #include "core_allvars.h"
    
    /* functions in model_infall.c */
    extern double infall_recipe(const int centralgal, const int ngal, const double Zcurr, struct GALAXY *galaxies);
    extern void strip_from_satellite(const int centralgal, const int gal, const double Zcurr, struct GALAXY *galaxies);
    extern double do_reionization(const int gal, const double Zcurr, struct GALAXY *galaxies);
    extern void add_infall_to_hot(const int gal, double infallingGas, struct GALAXY *galaxies);
    

#ifdef __cplusplus
}
#endif
