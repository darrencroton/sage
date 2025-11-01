/**
 * @file    model_misc.c
 * @brief   Miscellaneous utility functions for halo tracking
 *
 * This file contains various utility functions used throughout the SAGE code
 * for halo initialization, property calculation, and basic operations.
 * It includes functions for calculating halo properties (mass, velocity,
 * radius) and initializing halo tracking structures.
 *
 * Key functions:
 * - init_halo_tracker(): Initializes a new halo tracking object
 * - get_virial_mass/velocity/radius(): Calculate halo virial properties
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "core_allvars.h"
#include "core_proto.h"

/**
 * @brief   Initializes a new halo tracking object with default properties
 *
 * @param   p       Index in the Gal array (name retained for compatibility)
 * @param   halonr  Index of the halo in the Halo array
 *
 * This function initializes a new halo tracking object with default values.
 * It sets up the object's position, velocity, and halo properties based on
 * the input merger tree halo data. Each object is assigned a unique
 * number for identification and tracking through cosmic time.
 *
 * Note: The array is still called "Gal" and objects "galaxies" for code
 * compatibility, but these now track dark matter halos only.
 */
void init_halo_tracker(int p, int halonr) {
  int j;

  assert(halonr == Halo[halonr].FirstHaloInFOFgroup);

  Gal[p].Type = 0;

  Gal[p].GalaxyNr = GalaxyCounter;
  GalaxyCounter++;

  Gal[p].HaloNr = halonr;
  Gal[p].MostBoundID = Halo[halonr].MostBoundID;
  Gal[p].SnapNum = Halo[halonr].SnapNum - 1;

  Gal[p].MergeStatus = 0;
  Gal[p].mergeIntoID = -1;
  Gal[p].mergeIntoSnapNum = -1;
  Gal[p].dT = -1.0;

  for (j = 0; j < 3; j++) {
    Gal[p].Pos[j] = Halo[halonr].Pos[j];
    Gal[p].Vel[j] = Halo[halonr].Vel[j];
  }

  Gal[p].Len = Halo[halonr].Len;
  Gal[p].Vmax = Halo[halonr].Vmax;
  Gal[p].Vvir = get_virial_velocity(halonr);
  Gal[p].Mvir = get_virial_mass(halonr);
  Gal[p].Rvir = get_virial_radius(halonr);

  Gal[p].deltaMvir = 0.0;

  Gal[p].MergTime = 999.9;

  // infall properties
  Gal[p].infallMvir = -1.0;
  Gal[p].infallVvir = -1.0;
  Gal[p].infallVmax = -1.0;
}

/**
 * @brief   Returns the virial mass of a halo
 *
 * @param   halonr  Index of the halo in the Halo array
 * @return  Virial mass in 10^10 Msun/h
 *
 * This function returns the virial mass of a halo, using the spherical
 * overdensity mass if available for central halos. For satellite subhalos
 * or when spherical overdensity mass is not available, it returns the mass
 * estimated from particle counts.
 *
 * For central halos (FirstHaloInFOFgroup), it uses the Mvir property if
 * available. Otherwise, it calculates mass as number of particles × particle
 * mass.
 */
double get_virial_mass(int halonr) {
  if (halonr == Halo[halonr].FirstHaloInFOFgroup && Halo[halonr].Mvir >= 0.0)
    return Halo[halonr].Mvir; /* take spherical overdensity mass estimate */
  else
    return Halo[halonr].Len * SageConfig.PartMass;
}

/**
 * @brief   Calculates the virial velocity of a halo
 *
 * @param   halonr  Index of the halo in the Halo array
 * @return  Virial velocity in km/s
 *
 * This function calculates the virial velocity of a halo based on its
 * virial mass and radius using the formula:
 * Vvir = sqrt(G * Mvir / Rvir)
 *
 * The virial velocity represents the circular velocity at the virial radius
 * and is an important parameter for many galaxy formation processes.
 *
 * Returns 0.0 if the virial radius is zero or negative.
 */
double get_virial_velocity(int halonr) {
  double Rvir;

  Rvir = get_virial_radius(halonr);

  if (Rvir > 0.0)
    return sqrt(G * get_virial_mass(halonr) / Rvir);
  else
    return 0.0;
}

/**
 * @brief   Calculates the virial radius of a halo
 *
 * @param   halonr  Index of the halo in the Halo array
 * @return  Virial radius in Mpc/h
 *
 * This function calculates the virial radius of a halo based on its
 * virial mass and the critical density of the universe at the halo's redshift.
 * The virial radius is defined as the radius within which the mean density
 * is 200 times the critical density.
 *
 * The calculation uses the formula:
 * Rvir = [3 * Mvir / (4 * π * 200 * ρcrit)]^(1/3)
 *
 * Where ρcrit is the critical density at the halo's redshift, calculated
 * using the cosmological parameters.
 *
 * Note: For certain simulations like Bolshoi, the Rvir property from the
 * halo catalog could be used directly instead of this calculation.
 */
double get_virial_radius(int halonr) {
  // return Halo[halonr].Rvir;  // Used for Bolshoi

  double zplus1, hubble_of_z_sq, rhocrit, fac;

  zplus1 = 1 + ZZ[Halo[halonr].SnapNum];
  hubble_of_z_sq =
      Hubble * Hubble *
      (SageConfig.Omega * zplus1 * zplus1 * zplus1 +
       (1 - SageConfig.Omega - SageConfig.OmegaLambda) * zplus1 * zplus1 +
       SageConfig.OmegaLambda);

  rhocrit = 3 * hubble_of_z_sq / (8 * M_PI * G);
  fac = 1 / (200 * 4 * M_PI / 3.0 * rhocrit);

  return cbrt(get_virial_mass(halonr) * fac);
}
