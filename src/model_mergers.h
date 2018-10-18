#pragma once

#ifdef __cplusplus
extern "C" {
#endif

    #include "core_allvars.h"

    /* functions in model_mergers.c*/
    extern void disrupt_satellite_to_ICS(const int centralgal, const int gal, struct GALAXY *galaxies);
    extern double estimate_merging_time(const int sat_halo, const int mother_halo, const int ngal, struct halo_data *halos, struct GALAXY *galaxies);
    extern void deal_with_galaxy_merger(const int p, int merger_centralgal, const int centralgal, const double time,
                                        const double dt, const int halonr, const int step, struct GALAXY *galaxies);
    extern void quasar_mode_wind(const int gal, const double BHaccrete, struct GALAXY *galaxies);
    extern void add_galaxies_together(const int t, const int p, struct GALAXY *galaxies);
    extern void make_bulge_from_burst(const int p, struct GALAXY *galaxies);
    extern void grow_black_hole(const int merger_centralgal, const double mass_ratio, struct GALAXY *galaxies);
    extern void collisional_starburst_recipe(const double mass_ratio, const int merger_centralgal, const int centralgal, const double time,
                                             const double dt, const int halonr, const int mode, const int step, struct GALAXY *galaxies);

#ifdef __cplusplus
}
#endif
