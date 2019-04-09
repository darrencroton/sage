#pragma once

#ifdef __cplusplus
extern "C" {
#endif

    #include "core_allvars.h"

    /* functions in model_dust.c */
    extern void produce_metals_dust(const double metallicity, const double dt, const int p, const int centralgal, const int step, struct GALAXY *galaxies);
    extern void accrete_dust(const double metallicity, const double dt, const int p, const int step, struct GALAXY *galaxies);
    extern void destruct_dust(const double metallicity, const double stars, const double dt, const int p, const int step, struct GALAXY *galaxies);
    extern void dust_thermal_sputtering(const int gal, const double dt, struct GALAXY *galaxies);

#ifdef __cplusplus
}
#endif
