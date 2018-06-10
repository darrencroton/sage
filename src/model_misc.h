#pragma once

#ifdef __cplusplus
extern "C" {
#endif

    #include "core_allvars.h"

    /* functions in model_misc.c */
    extern void init_galaxy(const int p, const int halonr, int *galaxycounter, struct halo_data *halos, struct GALAXY *galaxies);
    extern double get_metallicity(const double gas, const double metals);
    extern double get_virial_velocity(const int halonr, struct halo_data *halos);
    extern double get_virial_radius(const int halonr, struct halo_data *halos);
    extern double get_virial_mass(const int halonr, struct halo_data *halos);
    extern double get_disk_radius(const int halonr, const int p, struct halo_data *halos, struct GALAXY *galaxies);
    extern double dmax(const double x, const double y);

#ifdef __cplusplus
}
#endif
