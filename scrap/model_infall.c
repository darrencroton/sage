/**
 * @file    model_infall.c
 * @brief   Implementation of gas infall and stripping processes
 *
 * This file contains the physical models for baryonic matter accretion
 * onto dark matter halos and environmental stripping of gas from satellites.
 * It implements:
 * - Cosmological gas infall onto halos
 * - Reionization suppression of gas accretion onto low-mass halos
 * - Stripping of hot gas from satellite galaxies
 * - Redistribution of baryons between galaxies
 *
 * The models account for cosmic reionization following Gnedin (2000) with
 * fitting formulas from Kravtsov et al. (2004), and implement
 * environmentally-driven gas stripping from satellites as they orbit within
 * larger halos.
 *
 * Key references:
 * - Gnedin (2000) for reionization model
 * - Kravtsov et al. (2004) for filtering mass formulas
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "core_allvars.h"
#include "core_proto.h"
#include "util_numeric.h"

/* Macro to suppress unused parameter warnings */
#define UNUSED(foo) (void)(foo)

/**
 * @brief   Calculates the amount of gas infalling onto a halo
 *
 * @param   centralgal    Index of the central galaxy
 * @param   ngal          Total number of galaxies in this halo
 * @param   Zcurr         Current redshift
 * @return  Mass of infalling gas
 *
 * This function calculates the amount of gas that should be accreted onto a
 * halo based on the cosmic baryon fraction, dark matter accretion, and
 * reionization effects. It also:
 * 1. Tracks all baryonic mass components in the halo
 * 2. Accounts for reionization suppression in low-mass halos
 * 3. Handles the ejected gas and intracluster stars (ICS) reservoirs
 * 4. Ensures conservation of total baryonic mass
 *
 * The infall is calculated as the difference between the expected baryon
 * content (cosmological baryon fraction × halo mass) and the current baryon
 * content, modified by reionization suppression for low-mass halos.
 */
double infall_recipe(int centralgal, int ngal, double Zcurr) {
  int i;
  double tot_stellarMass, tot_BHMass, tot_coldMass, tot_hotMass, tot_ejected,
      tot_ICS;
  double tot_hotMetals, tot_ejectedMetals, tot_ICSMetals;
  double tot_satBaryons, newSatBaryons;
  double infallingMass, reionization_modifier;

  /* Initialize counters for all baryonic components */
  tot_stellarMass = tot_coldMass = tot_hotMass = tot_hotMetals = tot_ejected =
      tot_BHMass = tot_ejectedMetals = tot_ICS = tot_ICSMetals =
          tot_satBaryons = 0.0;

  /* Loop over all galaxies in the FOF halo to sum baryonic components */
  for (i = 0; i < ngal; i++) {
    /* Sum all baryonic mass components */
    tot_stellarMass += Gal[i].StellarMass;
    tot_BHMass += Gal[i].BlackHoleMass;
    tot_coldMass += Gal[i].ColdGas;
    tot_hotMass += Gal[i].HotGas;
    tot_hotMetals += Gal[i].MetalsHotGas;
    tot_ejected += Gal[i].EjectedMass;
    tot_ejectedMetals += Gal[i].MetalsEjectedMass;
    tot_ICS += Gal[i].ICS;
    tot_ICSMetals += Gal[i].MetalsICS;

    /* Track baryons in satellite galaxies separately */
    if (i != centralgal)
      tot_satBaryons += Gal[i].StellarMass + Gal[i].BlackHoleMass +
                        Gal[i].ColdGas + Gal[i].HotGas;

    /* Move satellite ejected gas to central galaxy's ejected reservoir */
    if (i != centralgal)
      Gal[i].EjectedMass = Gal[i].MetalsEjectedMass = 0.0;

    /* Move satellite intracluster stars to central galaxy */
    if (i != centralgal)
      Gal[i].ICS = Gal[i].MetalsICS = 0.0;
  }

  /* Calculate new baryons that have fallen in with substructure since the last
   * timestep */
  newSatBaryons = tot_satBaryons - Gal[centralgal].TotalSatelliteBaryons;

  /* Calculate reionization suppression factor if enabled */
  if (SageConfig.ReionizationOn)
    reionization_modifier = do_reionization(centralgal, Zcurr);
  else
    reionization_modifier = 1.0;

  /* Calculate infalling gas mass as the difference between expected baryon
   * content (based on halo mass and cosmic baryon fraction, modified by
   * reionization) and current baryon content in all components */
  infallingMass =
      reionization_modifier * SageConfig.BaryonFrac * Gal[centralgal].Mvir -
      (tot_stellarMass + tot_coldMass + tot_hotMass + tot_ejected + tot_BHMass +
       tot_ICS);

  /* Alternative implementation commented out in original code:
   * reionization_modifier * BaryonFrac * Gal[centralgal].deltaMvir -
   * newSatBaryons; */
  UNUSED(newSatBaryons); /* Variable not currently used */

  /* Assign all ejected mass to the central galaxy */
  Gal[centralgal].EjectedMass = tot_ejected;
  Gal[centralgal].MetalsEjectedMass = tot_ejectedMetals;

  /* Enforce physical constraints on ejected mass and metals */
  if (is_greater(Gal[centralgal].MetalsEjectedMass,
                 Gal[centralgal].EjectedMass))
    Gal[centralgal].MetalsEjectedMass =
        Gal[centralgal].EjectedMass; /* Metals can't exceed total mass */
  if (is_less(Gal[centralgal].EjectedMass, 0.0))
    Gal[centralgal].EjectedMass = Gal[centralgal].MetalsEjectedMass =
        0.0; /* No negative masses */
  if (is_less(Gal[centralgal].MetalsEjectedMass, 0.0))
    Gal[centralgal].MetalsEjectedMass = 0.0; /* No negative metal masses */

  /* Assign all intracluster stars to the central galaxy (for numerical
   * convenience) */
  Gal[centralgal].ICS = tot_ICS;
  Gal[centralgal].MetalsICS = tot_ICSMetals;

  /* Enforce physical constraints on ICS mass and metals */
  if (is_greater(Gal[centralgal].MetalsICS, Gal[centralgal].ICS))
    Gal[centralgal].MetalsICS =
        Gal[centralgal].ICS; /* Metals can't exceed total mass */
  if (is_less(Gal[centralgal].ICS, 0.0))
    Gal[centralgal].ICS = Gal[centralgal].MetalsICS =
        0.0; /* No negative masses */
  if (is_less(Gal[centralgal].MetalsICS, 0.0))
    Gal[centralgal].MetalsICS = 0.0; /* No negative metal masses */

  return infallingMass; /* Return calculated infalling gas mass */
}

/**
 * @brief   Strips hot gas from satellite galaxies and adds it to the central
 * galaxy
 *
 * @param   halonr        Index of the current halo
 * @param   centralgal    Index of the central galaxy
 * @param   gal           Index of the satellite galaxy being stripped
 *
 * This function implements environmental stripping of hot gas from satellite
 * galaxies as they move through the hot halo of the central galaxy. The amount
 * of stripped gas is determined by the difference between expected baryon
 * content (considering the satellite's dark matter mass) and the actual baryon
 * content.
 *
 * The stripping occurs gradually over STEPS timesteps, and the stripped gas
 * (including its metals) is added to the central galaxy's hot gas reservoir.
 *
 * The model assumes that satellites can only lose gas, not gain it, so
 * stripping only occurs when the satellite has an excess of baryons relative to
 * its dark matter. The gradual stripping approach (over STEPS timesteps)
 * approximates the continuous nature of environmental effects as a satellite
 * orbits within the host halo.
 */
void strip_from_satellite(int halonr, int centralgal, int gal) {
  double reionization_modifier, strippedGas, strippedGasMetals, metallicity;

  /* Apply reionization modifier if enabled */
  if (SageConfig.ReionizationOn)
    reionization_modifier = do_reionization(gal, ZZ[Halo[halonr].SnapNum]);
  else
    reionization_modifier = 1.0;

  /* Calculate amount of gas to strip
   * This is the difference between expected baryon content (based on dark
   * matter mass) and actual baryon content, divided by STEPS to implement
   * gradual stripping */
  strippedGas =
      -1.0 *
      (reionization_modifier * SageConfig.BaryonFrac * Gal[gal].Mvir -
       (Gal[gal].StellarMass + Gal[gal].ColdGas + Gal[gal].HotGas +
        Gal[gal].EjectedMass + Gal[gal].BlackHoleMass + Gal[gal].ICS)) /
      STEPS;

  /* Alternative implementation commented out in original code:
   * ( reionization_modifier * BaryonFrac * Gal[gal].deltaMvir ) / STEPS; */

  /* Only proceed if there is positive stripping (satellite has excess baryons)
   */
  if (is_greater(strippedGas, 0.0)) {
    /* Calculate metals in the stripped gas */
    metallicity = get_metallicity(Gal[gal].HotGas, Gal[gal].MetalsHotGas);
    strippedGasMetals = strippedGas * metallicity;

    /* Limit stripping to available hot gas and metals */
    if (is_greater(strippedGas, Gal[gal].HotGas))
      strippedGas = Gal[gal].HotGas;
    if (is_greater(strippedGasMetals, Gal[gal].MetalsHotGas))
      strippedGasMetals = Gal[gal].MetalsHotGas;

    /* Remove gas and metals from satellite */
    Gal[gal].HotGas -= strippedGas;
    Gal[gal].MetalsHotGas -= strippedGasMetals;

    /* Add stripped gas and metals to central galaxy */
    Gal[centralgal].HotGas += strippedGas;
    Gal[centralgal].MetalsHotGas += strippedGas * metallicity;
  }
}

/**
 * @brief   Calculates the reionization suppression factor for gas accretion
 *
 * @param   gal           Index of the galaxy
 * @param   Zcurr         Current redshift
 * @return  Modifier factor (between 0 and 1) for gas accretion
 *
 * This function implements the Gnedin (2000) reionization model with the
 * fitting formulas from Kravtsov et al. (2004) Appendix B. Reionization
 * suppresses gas accretion onto low-mass halos after the universe becomes
 * reionized, due to the increase in gas temperature and Jeans mass.
 *
 * The suppression depends on the ratio between the halo mass and a
 * characteristic mass (the maximum of the filtering mass and the mass
 * corresponding to a virial temperature of 10^4 K).
 *
 * The calculation has three regimes depending on the scale factor:
 * 1. Before UV background turns on (a ≤ a0)
 * 2. During partial reionization (a0 < a < ar)
 * 3. After full reionization (a ≥ ar)
 *
 * For each regime, different formulas are used to calculate the filtering
 * mass, which represents the mass scale below which baryonic accretion is
 * significantly suppressed.
 *
 * The final modifier is calculated as: 1 / [1 + 0.26(Mchar/Mvir)^3]
 * which approaches 0 for Mvir << Mchar and 1 for Mvir >> Mchar.
 *
 * References:
 * - Gnedin (2000) - Original reionization model
 * - Kravtsov et al. (2004) - Fitting formulas for filtering mass
 */
double do_reionization(int gal, double Zcurr) {
  double alpha, a, f_of_a, a_on_a0, a_on_ar, Mfiltering, Mjeans, Mchar,
      mass_to_use, modifier;
  double Tvir, Vchar, omegaZ, xZ, deltacritZ, HubbleZ;

  /* We employ the reionization recipe described in Gnedin (2000), using the
   * fitting formulas given by Kravtsov et al. (2004) Appendix B */

  /* Parameters that Kravtsov et al. keep fixed
   * alpha gives the best fit to the Gnedin data */
  alpha = 6.0;
  Tvir = 1e4; /* Virial temperature in Kelvin */

  /* Calculate scale factor and ratios needed for filtering mass calculation */
  a = 1.0 / (1.0 + Zcurr); /* Scale factor */
  a_on_a0 = a / a0;        /* Ratio to epoch when UV background turns on */
  a_on_ar = a / ar;        /* Ratio to epoch of full reionization */

  /* Calculate f_of_a term from Kravtsov et al. (2004) fitting formula
   * This has three regimes based on the scale factor */
  if (a <= a0)
    /* Before UV background turns on */
    f_of_a =
        3.0 * a / ((2.0 + alpha) * (5.0 + 2.0 * alpha)) * pow(a_on_a0, alpha);
  else if ((a > a0) && (a < ar))
    /* During partial reionization */
    f_of_a = (3.0 / a) * a0 * a0 *
                 (1.0 / (2.0 + alpha) -
                  2.0 * pow(a_on_a0, -0.5) / (5.0 + 2.0 * alpha)) +
             a * a / 10.0 - (a0 * a0 / 10.0) * (5.0 - 4.0 * pow(a_on_a0, -0.5));
  else
    /* After full reionization */
    f_of_a = (3.0 / a) * (a0 * a0 *
                              (1.0 / (2.0 + alpha) -
                               2.0 * pow(a_on_a0, -0.5) / (5.0 + 2.0 * alpha)) +
                          (ar * ar / 10.0) * (5.0 - 4.0 * pow(a_on_ar, -0.5)) -
                          (a0 * a0 / 10.0) * (5.0 - 4.0 * pow(a_on_a0, -0.5)) +
                          a * ar / 3.0 -
                          (ar * ar / 3.0) * (3.0 - 2.0 * pow(a_on_ar, -0.5)));

  /* Calculate the filtering mass (in units of 10^10 Msun/h)
   * Note mu=0.59 and mu^-1.5 = 2.21 for a fully ionized gas */
  Mjeans = 25.0 * pow(SageConfig.Omega, -0.5) * 2.21;
  Mfiltering = Mjeans * pow(f_of_a, 1.5);

  /* Calculate the characteristic mass corresponding to a halo temperature of
   * 10^4K */
  Vchar =
      sqrt(safe_div(Tvir, 36.0, EPSILON_SMALL)); /* Characteristic velocity */

  /* Calculate cosmological parameters at current redshift */
  omegaZ =
      SageConfig.Omega * safe_div(pow(1.0 + Zcurr, 3.0),
                                  (SageConfig.Omega * pow(1.0 + Zcurr, 3.0) +
                                   SageConfig.OmegaLambda),
                                  EPSILON_SMALL);
  xZ = omegaZ - 1.0;
  deltacritZ = 18.0 * M_PI * M_PI + 82.0 * xZ -
               39.0 * xZ * xZ; /* Critical overdensity at z */
  HubbleZ = Hubble * sqrt(SageConfig.Omega * pow(1.0 + Zcurr, 3.0) +
                          SageConfig.OmegaLambda); /* Hubble parameter at z */

  /* Calculate characteristic mass from virial temperature */
  Mchar = Vchar * Vchar * Vchar *
          safe_div(1.0, (G * HubbleZ * sqrt(0.5 * deltacritZ)), EPSILON_SMALL);

  /* Use the maximum of filtering mass and characteristic mass */
  mass_to_use = dmax(Mfiltering, Mchar);

  /* Calculate suppression modifier using fitting formula
   * This approaches 0 for low-mass halos and 1 for high-mass halos */
  modifier = 1.0 / pow(1.0 + 0.26 * safe_div(mass_to_use, Gal[gal].Mvir,
                                             EPSILON_SMALL),
                       3.0);

  return modifier;
}

/**
 * @brief   Adds infalling gas to the hot gas component of a galaxy
 *
 * @param   gal           Index of the galaxy
 * @param   infallingGas  Amount of gas to be added (can be negative)
 *
 * This function handles the addition of infalling gas to a galaxy's hot gas
 * reservoir. It also handles the case of negative infall (mass loss) by
 * removing gas in a physically consistent way:
 *
 * 1. For mass loss (negative infallingGas):
 *    a. First remove from the ejected gas reservoir
 *    b. Then remove from the hot gas reservoir
 * 2. For mass gain (positive infallingGas):
 *    a. Add directly to the hot gas reservoir
 *
 * Metals are handled consistently with the gas flows, preserving metallicities
 * during transfers between reservoirs.
 */
void add_infall_to_hot(int gal, double infallingGas) {
  float metallicity;

  /* Handle mass loss case (negative infall) */
  if (is_less(infallingGas, 0.0) && is_greater(Gal[gal].EjectedMass, 0.0)) {
    /* First remove from ejected gas reservoir */
    metallicity =
        get_metallicity(Gal[gal].EjectedMass, Gal[gal].MetalsEjectedMass);

    /* Update ejected metals, preserving metallicity */
    Gal[gal].MetalsEjectedMass += infallingGas * metallicity;
    if (is_less(Gal[gal].MetalsEjectedMass, 0.0))
      Gal[gal].MetalsEjectedMass = 0.0; /* Prevent negative metals */

    /* Update ejected gas mass */
    Gal[gal].EjectedMass += infallingGas;

    /* If ejected reservoir is depleted, continue removing from hot gas */
    if (is_less(Gal[gal].EjectedMass, 0.0)) {
      infallingGas = Gal[gal].EjectedMass; /* Remaining gas to remove */
      Gal[gal].EjectedMass = Gal[gal].MetalsEjectedMass =
          0.0; /* Reset ejected reservoir */
    } else
      infallingGas = 0.0; /* All gas removal handled by ejected reservoir */
  }

  /* If we still have mass loss after depleting ejected gas, remove from hot gas
   * metals */
  if (is_less(infallingGas, 0.0) && is_greater(Gal[gal].MetalsHotGas, 0.0)) {
    metallicity = get_metallicity(Gal[gal].HotGas, Gal[gal].MetalsHotGas);

    /* Update hot gas metals, preserving metallicity */
    Gal[gal].MetalsHotGas += infallingGas * metallicity;
    if (is_less(Gal[gal].MetalsHotGas, 0.0))
      Gal[gal].MetalsHotGas = 0.0; /* Prevent negative metals */
  }

  /* Finally update the hot gas component */
  Gal[gal].HotGas += infallingGas;

  /* Ensure non-negative values */
  if (is_less(Gal[gal].HotGas, 0.0))
    Gal[gal].HotGas = Gal[gal].MetalsHotGas = 0.0;
}
