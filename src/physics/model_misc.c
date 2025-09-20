/**
 * @file    model_misc.c
 * @brief   Miscellaneous utility functions for galaxy evolution modeling
 *
 * This file contains various utility functions used throughout the SAGE code
 * for galaxy initialization, property calculation, and basic operations.
 * It includes functions for calculating halo properties (mass, velocity,
 * radius), galaxy disk properties, metallicities, and various helper functions.
 *
 * Key functions:
 * - init_galaxy(): Initializes a new galaxy with default properties
 * - get_disk_radius(): Calculates disk scale radius based on halo properties
 * - get_virial_mass/velocity/radius(): Calculate halo virial properties
 * - get_metallicity(): Calculates metallicity from gas mass and metal content
 *
 * References:
 * - Mo, Mao & White (1998) - Disk radius calculation
 * - Bullock et al. (2001) - Spin parameter definition
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../core/core_allvars.h"
#include "../core/core_proto.h"

/**
 * @brief   Initializes a new galaxy with default properties
 *
 * @param   p       Index of the galaxy in the Gal array
 * @param   halonr  Index of the halo in the Halo array
 *
 * This function initializes a new galaxy with default values for all
 * properties. It sets up the galaxy's position, velocity, and physical
 * properties based on its host halo. The galaxy is initialized with zero mass
 * in all components (cold gas, stellar mass, bulge, hot gas, etc.) and will
 * accumulate mass through subsequent evolution.
 *
 * The function also initializes tracking variables for mergers, star formation
 * history, and other galaxy properties. Each new galaxy is assigned a unique
 * galaxy number for identification.
 */
void init_galaxy(int p, int halonr) {
  int j, step;

  assert(halonr == Halo[halonr].FirstHaloInFOFgroup);

  Gal[p].Type = 0;

  Gal[p].GalaxyNr = GalaxyCounter;
  GalaxyCounter++;

  Gal[p].HaloNr = halonr;
  Gal[p].MostBoundID = Halo[halonr].MostBoundID;
  Gal[p].SnapNum = Halo[halonr].SnapNum - 1;

  Gal[p].mergeType = 0;
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

  Gal[p].ColdGas = 0.0;
  Gal[p].StellarMass = 0.0;
  Gal[p].BulgeMass = 0.0;
  Gal[p].HotGas = 0.0;
  Gal[p].EjectedMass = 0.0;
  Gal[p].BlackHoleMass = 0.0;
  Gal[p].ICS = 0.0;

  Gal[p].MetalsColdGas = 0.0;
  Gal[p].MetalsStellarMass = 0.0;
  Gal[p].MetalsBulgeMass = 0.0;
  Gal[p].MetalsHotGas = 0.0;
  Gal[p].MetalsEjectedMass = 0.0;
  Gal[p].MetalsICS = 0.0;

  for (step = 0; step < STEPS; step++) {
    Gal[p].SfrDisk[step] = 0.0;
    Gal[p].SfrBulge[step] = 0.0;
    Gal[p].SfrDiskColdGas[step] = 0.0;
    Gal[p].SfrDiskColdGasMetals[step] = 0.0;
    Gal[p].SfrBulgeColdGas[step] = 0.0;
    Gal[p].SfrBulgeColdGasMetals[step] = 0.0;
  }

  Gal[p].DiskScaleRadius = get_disk_radius(halonr, p);
  Gal[p].MergTime = 999.9;
  Gal[p].Cooling = 0.0;
  Gal[p].Heating = 0.0;
  Gal[p].r_heat = 0.0;
  Gal[p].QuasarModeBHaccretionMass = 0.0;
  Gal[p].TimeOfLastMajorMerger = -1.0;
  Gal[p].TimeOfLastMinorMerger = -1.0;
  Gal[p].OutflowRate = 0.0;
  Gal[p].TotalSatelliteBaryons = 0.0;

  // infall properties
  Gal[p].infallMvir = -1.0;
  Gal[p].infallVvir = -1.0;
  Gal[p].infallVmax = -1.0;
}

/**
 * @brief   Calculates the disk scale radius for a galaxy
 *
 * @param   halonr  Index of the halo in the Halo array
 * @param   p       Index of the galaxy in the Gal array
 * @return  Disk scale radius in Mpc/h
 *
 * This function calculates the disk scale radius based on the Mo, Mao & White
 * (1998) model, which relates disk size to halo properties. It uses the halo's
 * spin parameter (Bullock definition) and virial radius to determine the disk
 * radius.
 *
 * The disk radius is proportional to the spin parameter and the virial radius:
 * Rd = (λ / √2) * Rvir
 *
 * If virial properties are not available, it defaults to 0.1*Rvir.
 */
double get_disk_radius(int halonr, int p) {
  double SpinMagnitude, SpinParameter;

  if (Gal[p].Vvir > 0.0 && Gal[p].Rvir > 0.0) {
    // See Mo, Shude & White (1998) eq12, and using a Bullock style lambda.
    SpinMagnitude = sqrt(Halo[halonr].Spin[0] * Halo[halonr].Spin[0] +
                         Halo[halonr].Spin[1] * Halo[halonr].Spin[1] +
                         Halo[halonr].Spin[2] * Halo[halonr].Spin[2]);

    SpinParameter = SpinMagnitude / (1.414 * Gal[p].Vvir * Gal[p].Rvir);
    return (SpinParameter / 1.414) * Gal[p].Rvir;
  } else
    return 0.1 * Gal[p].Rvir;
}

/**
 * @brief   Calculates the metallicity of a gas or stellar component
 *
 * @param   gas     Total mass of the gas or stellar component
 * @param   metals  Total mass of metals in the component
 * @return  Metallicity as a mass fraction (0.0 to 1.0)
 *
 * This function calculates the metallicity as the ratio of metal mass
 * to total mass. It ensures the result is between 0 and 1, handling
 * edge cases where gas or metal mass might be zero or very small.
 *
 * Metallicity is expressed as a mass fraction rather than relative to
 * solar (Z/Z_sun), so a value of 0.02 would correspond to approximately
 * solar metallicity.
 */
double get_metallicity(double gas, double metals) {
  double metallicity;

  if (gas > 0.0 && metals > 0.0) {
    metallicity = metals / gas;
    if (metallicity < 1.0)
      return metallicity;
    else
      return 1.0;
  } else
    return 0.0;
}

/**
 * @brief   Returns the maximum of two double values
 *
 * @param   x   First value
 * @param   y   Second value
 * @return  The larger of x and y
 *
 * A simple utility function that returns the maximum of two double values.
 * Used in various calculations throughout the code where the maximum of
 * two quantities is needed.
 */
double dmax(double x, double y) {
  if (x > y)
    return x;
  else
    return y;
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
