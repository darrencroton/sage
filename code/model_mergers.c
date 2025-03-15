/**
 * @file    model_mergers.c
 * @brief   Galaxy merger processes and transformations
 *
 * This file implements the physical processes related to galaxy mergers,
 * including orbital decay of satellites, galaxy-galaxy mergers, starbursts,
 * morphological transformations, and black hole growth during mergers.
 * It handles both major mergers (resulting in spheroid formation) and
 * minor mergers (disk preservation with bulge growth).
 *
 * Key functions:
 * - estimate_merging_time(): Calculates dynamical friction timescale for
 * satellite galaxies
 * - deal_with_galaxy_merger(): Main function processing mergers when they occur
 * - grow_black_hole(): Implements black hole growth during mergers
 * - collisional_starburst_recipe(): Triggers starbursts during galaxy
 * interactions
 * - add_galaxies_together(): Combines properties of merging galaxies
 *
 * References:
 * - Binney & Tremaine (1987) - Dynamical friction formulation
 * - Somerville et al. (2001) - Merger-induced starburst model
 * - Kauffmann & Haehnelt (2000) - Black hole growth during mergers
 * - Cox (PhD thesis) - Updated starburst efficiency parameterization
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "core_allvars.h"
#include "core_proto.h"
#include "util_numeric.h"

/**
 * @brief   Estimates the dynamical friction timescale for satellite galaxies
 *
 * @param   sat_halo     Index of the satellite subhalo
 * @param   mother_halo  Index of the parent/host halo
 * @param   ngal         Index of the satellite galaxy in the Gal array
 * @return  Merging time in Gyr, or -1.0 if calculation fails
 *
 * This function calculates the time required for a satellite galaxy to
 * merge with the central galaxy due to dynamical friction. It uses the
 * approximation from Binney & Tremaine (1987):
 *
 * T_merge = 2.0 * 1.17 * R_sat^2 * V_vir / (ln(M_host/M_sat) * G * M_sat)
 *
 * Where:
 * - R_sat is the virial radius of the host halo
 * - V_vir is the virial velocity of the host halo
 * - M_host/M_sat is the mass ratio (used in Coulomb logarithm)
 * - G is the gravitational constant
 * - M_sat is the total satellite mass (DM + stars + cold gas)
 */
double estimate_merging_time(int sat_halo, int mother_halo, int ngal) {
  double coulomb, mergtime, SatelliteMass, SatelliteRadius;

  if (sat_halo == mother_halo) {
    printf("\t\tSnapNum, Type, IDs, sat radius:\t%i\t%i\t%i\t%i\t--- sat/cent "
           "have the same ID\n",
           Gal[ngal].SnapNum, Gal[ngal].Type, sat_halo, mother_halo);
    return -1.0;
  }

  coulomb = log(Halo[mother_halo].Len / ((double)Halo[sat_halo].Len) + 1);

  SatelliteMass =
      get_virial_mass(sat_halo) + Gal[ngal].StellarMass + Gal[ngal].ColdGas;
  SatelliteRadius = get_virial_radius(mother_halo);

  if (is_greater(SatelliteMass, 0.0) && is_greater(coulomb, 0.0))
    mergtime = 2.0 * 1.17 * SatelliteRadius * SatelliteRadius *
               get_virial_velocity(mother_halo) *
               safe_div(1.0, (coulomb * G * SatelliteMass), EPSILON_SMALL);
  else
    mergtime = -1.0;

  return mergtime;
}

/**
 * @brief   Processes a galaxy merger when it occurs
 *
 * @param   p                  Index of the satellite galaxy
 * @param   merger_centralgal  Index of the central galaxy being merged with
 * @param   centralgal         Index of the central galaxy of the halo
 * @param   time               Current time in the simulation
 * @param   dt                 Time step size
 * @param   halonr             Index of the current halo
 * @param   step               Current substep in the time integration
 *
 * This function handles the merger of two galaxies, including:
 * 1. Calculating the mass ratio for determining merger type
 * 2. Transferring all properties from satellite to central
 * 3. Triggering black hole growth during the merger
 * 4. Triggering a starburst from the merger
 * 5. Performing morphological transformations for major mergers
 * 6. Updating merger type and timing information
 *
 * The mass ratio determines whether it's a major merger (which transforms
 * the remnant into a spheroid) or a minor merger (which preserves the disk).
 */
void deal_with_galaxy_merger(int p, int merger_centralgal, int centralgal,
                             double time, double dt, int halonr, int step) {
  double mi, ma, mass_ratio;

  /* Calculate mass ratio of merging galaxies
   * Always use the smaller/larger ratio regardless of which is satellite */
  if (Gal[p].StellarMass + Gal[p].ColdGas <
      Gal[merger_centralgal].StellarMass + Gal[merger_centralgal].ColdGas) {
    mi = Gal[p].StellarMass + Gal[p].ColdGas;
    ma = Gal[merger_centralgal].StellarMass + Gal[merger_centralgal].ColdGas;
  } else {
    mi = Gal[merger_centralgal].StellarMass + Gal[merger_centralgal].ColdGas;
    ma = Gal[p].StellarMass + Gal[p].ColdGas;
  }

  /* Calculate mass ratio, defaulting to 1.0 if no mass */
  if (is_greater(ma, 0.0))
    mass_ratio = safe_div(mi, ma, 1.0);
  else
    mass_ratio = 1.0;

  /* Add all components of satellite galaxy to central galaxy */
  add_galaxies_together(merger_centralgal, p);

  /* Grow black hole through accretion from cold disk during mergers (Kauffmann
   * & Haehnelt 2000) */
  if (SageConfig.AGNrecipeOn)
    grow_black_hole(merger_centralgal, mass_ratio);

  /* Trigger starburst from the merger (Somerville et al. 2001)
   * Mode 0 indicates a merger-induced starburst */
  collisional_starburst_recipe(mass_ratio, merger_centralgal, centralgal, time,
                               dt, halonr, 0, step);

  /* Update minor merger timing if significant enough (mass ratio > 0.1) */
  if (is_greater(mass_ratio, 0.1))
    Gal[merger_centralgal].TimeOfLastMinorMerger = time;

  /* For major mergers (mass ratio > threshold), transform the remnant into a
   * spheroid */
  if (is_greater(mass_ratio, SageConfig.ThreshMajorMerger)) {
    make_bulge_from_burst(merger_centralgal);
    Gal[merger_centralgal].TimeOfLastMajorMerger = time;
    Gal[p].mergeType = 2; /* Mark as major merger */
  } else {
    Gal[p].mergeType = 1; /* Mark as minor merger */
  }
}

/**
 * @brief   Grows central black hole during galaxy mergers
 *
 * @param   merger_centralgal  Index of the central galaxy with the black hole
 * @param   mass_ratio         Mass ratio of the merging galaxies
 *
 * This function implements black hole growth during galaxy mergers following
 * the model of Kauffmann & Haehnelt (2000). The amount of cold gas accreted
 * onto the black hole depends on:
 *
 * 1. The merger mass ratio (more equal-mass mergers drive more accretion)
 * 2. The virial velocity of the halo (higher velocities allow more accretion)
 * 3. The available cold gas (which provides the fuel for accretion)
 *
 * The function also accounts for metals and triggers quasar-mode feedback
 * when black hole accretion occurs, which can heat or eject gas.
 */
void grow_black_hole(int merger_centralgal, double mass_ratio) {
  double BHaccrete, metallicity;

  /* Only proceed if there is cold gas available for accretion */
  if (is_greater(Gal[merger_centralgal].ColdGas, 0.0)) {
    /* Calculate BH accretion rate using the Kauffmann & Haehnelt (2000) formula
     * Accretion increases with:
     * - Higher mass ratio mergers (more violent events)
     * - Higher virial velocities (deeper potential wells)
     * - More available cold gas (more fuel) */
    BHaccrete =
        SageConfig.BlackHoleGrowthRate * mass_ratio /
        (1.0 + pow(safe_div(280.0, Gal[merger_centralgal].Vvir, EPSILON_SMALL),
                   2.0)) *
        Gal[merger_centralgal].ColdGas;

    /* Limit accretion to available cold gas */
    if (is_greater(BHaccrete, Gal[merger_centralgal].ColdGas))
      BHaccrete = Gal[merger_centralgal].ColdGas;

    /* Calculate metallicity of accreted gas */
    metallicity = get_metallicity(Gal[merger_centralgal].ColdGas,
                                  Gal[merger_centralgal].MetalsColdGas);

    /* Update galaxy properties */
    Gal[merger_centralgal].BlackHoleMass += BHaccrete; /* Grow the black hole */
    Gal[merger_centralgal].ColdGas -=
        BHaccrete; /* Remove gas that was accreted */
    Gal[merger_centralgal].MetalsColdGas -=
        metallicity * BHaccrete; /* Remove corresponding metals */

    /* Track mass accreted in quasar mode for statistics */
    Gal[merger_centralgal].QuasarModeBHaccretionMass += BHaccrete;

    /* Trigger quasar-mode wind feedback from the accretion */
    quasar_mode_wind(merger_centralgal, BHaccrete);
  }
}

/**
 * @brief   Implements quasar-mode AGN feedback winds
 *
 * @param   gal        Index of the galaxy in the Gal array
 * @param   BHaccrete  Amount of mass accreted onto the black hole
 *
 * This function models the powerful winds generated during quasar-mode black
 * hole accretion. It compares the energy released by the quasar wind to the
 * binding energies of the galaxy's gas components, and removes gas that can be
 * ejected by the wind.
 *
 * The model implements two levels of feedback:
 * 1. If quasar energy exceeds cold gas binding energy, all cold gas is ejected
 * 2. If quasar energy exceeds combined cold+hot gas binding energy, all hot gas
 *    is also ejected
 *
 * The quasar energy is calculated as E = η × 0.1 × Maccrete × c², where η is
 * the quasar mode efficiency parameter and 0.1 is the assumed radiative
 * efficiency.
 */
void quasar_mode_wind(int gal, float BHaccrete) {
  float quasar_energy, cold_gas_energy, hot_gas_energy;

  /* Calculate energies involved:
   * 1. Quasar wind energy (E = η × 0.1 × Maccrete × c²)
   * 2. Cold gas binding energy (E = 0.5 × Mcold × Vvir²)
   * 3. Hot gas binding energy (E = 0.5 × Mhot × Vvir²) */
  quasar_energy = SageConfig.QuasarModeEfficiency * 0.1 * BHaccrete *
                  pow(C / UnitVelocity_in_cm_per_s, 2.0);
  cold_gas_energy = 0.5 * Gal[gal].ColdGas * Gal[gal].Vvir * Gal[gal].Vvir;
  hot_gas_energy = 0.5 * Gal[gal].HotGas * Gal[gal].Vvir * Gal[gal].Vvir;

  /* If quasar energy exceeds cold gas binding energy, eject all cold gas */
  if (is_greater(quasar_energy, cold_gas_energy)) {
    /* Add cold gas and its metals to ejected reservoir */
    Gal[gal].EjectedMass += Gal[gal].ColdGas;
    Gal[gal].MetalsEjectedMass += Gal[gal].MetalsColdGas;

    /* Reset cold gas and metals to zero */
    Gal[gal].ColdGas = 0.0;
    Gal[gal].MetalsColdGas = 0.0;
  }

  /* If quasar energy exceeds combined cold+hot gas binding energy, also eject
   * all hot gas */
  if (is_greater(quasar_energy, cold_gas_energy + hot_gas_energy)) {
    /* Add hot gas and its metals to ejected reservoir */
    Gal[gal].EjectedMass += Gal[gal].HotGas;
    Gal[gal].MetalsEjectedMass += Gal[gal].MetalsHotGas;

    /* Reset hot gas and metals to zero */
    Gal[gal].HotGas = 0.0;
    Gal[gal].MetalsHotGas = 0.0;
  }
}

/**
 * @brief   Combines two galaxies during a merger
 *
 * @param   t    Index of the target (central) galaxy
 * @param   p    Index of the satellite galaxy being merged
 *
 * This function transfers all components from the satellite galaxy to the
 * central galaxy during a merger. It handles:
 *
 * 1. Mass components (cold gas, stars, hot gas, ejected mass, ICS, black hole)
 * 2. Metal components for each mass reservoir
 * 3. Star formation history arrays
 *
 * The function follows the physical principle that during mergers, all material
 * from the satellite is added to the central galaxy, with the stellar component
 * specifically added to the central galaxy's bulge.
 */
void add_galaxies_together(int t, int p) {
  int step;

  /* Add gas components and their metals */
  Gal[t].ColdGas += Gal[p].ColdGas;
  Gal[t].MetalsColdGas += Gal[p].MetalsColdGas;

  Gal[t].HotGas += Gal[p].HotGas;
  Gal[t].MetalsHotGas += Gal[p].MetalsHotGas;

  Gal[t].EjectedMass += Gal[p].EjectedMass;
  Gal[t].MetalsEjectedMass += Gal[p].MetalsEjectedMass;

  /* Add stellar components */
  Gal[t].StellarMass += Gal[p].StellarMass;
  Gal[t].MetalsStellarMass += Gal[p].MetalsStellarMass;

  /* Add intracluster stars */
  Gal[t].ICS += Gal[p].ICS;
  Gal[t].MetalsICS += Gal[p].MetalsICS;

  /* Add black hole mass */
  Gal[t].BlackHoleMass += Gal[p].BlackHoleMass;

  /* Add satellite's stellar mass to the bulge component
   * This models the morphological transformation during mergers */
  Gal[t].BulgeMass += Gal[p].StellarMass;
  Gal[t].MetalsBulgeMass += Gal[p].MetalsStellarMass;

  /* Add star formation histories for all time steps
   * Both disk and bulge components from the satellite are added to the bulge of
   * the central */
  for (step = 0; step < STEPS; step++) {
    Gal[t].SfrBulge[step] += Gal[p].SfrDisk[step] + Gal[p].SfrBulge[step];
    Gal[t].SfrBulgeColdGas[step] +=
        Gal[p].SfrDiskColdGas[step] + Gal[p].SfrBulgeColdGas[step];
    Gal[t].SfrBulgeColdGasMetals[step] +=
        Gal[p].SfrDiskColdGasMetals[step] + Gal[p].SfrBulgeColdGasMetals[step];
  }
}

/**
 * @brief   Transforms a disk-dominated galaxy into a bulge-dominated one
 *
 * @param   p    Index of the galaxy in the Gal array
 *
 * This function implements the morphological transformation from a
 * disk-dominated galaxy to a bulge-dominated one, typically triggered by a
 * major merger. It transfers all stellar content from the disk to the bulge
 * component and updates the star formation history arrays accordingly.
 *
 * After this transformation, the galaxy will have no remaining disk component,
 * representing the violent relaxation that occurs during major mergers.
 */
void make_bulge_from_burst(int p) {
  int step;

  /* Transfer all stellar mass to the bulge component */
  Gal[p].BulgeMass =
      Gal[p].StellarMass; /* Set bulge mass to total stellar mass */
  Gal[p].MetalsBulgeMass = Gal[p].MetalsStellarMass; /* Transfer all metals */

  /* Update star formation history */
  for (step = 0; step < STEPS; step++) {
    /* Add disk SFR components to bulge components */
    Gal[p].SfrBulge[step] += Gal[p].SfrDisk[step];
    Gal[p].SfrBulgeColdGas[step] += Gal[p].SfrDiskColdGas[step];
    Gal[p].SfrBulgeColdGasMetals[step] += Gal[p].SfrDiskColdGasMetals[step];

    /* Reset disk SFR components to zero */
    Gal[p].SfrDisk[step] = 0.0;
    Gal[p].SfrDiskColdGas[step] = 0.0;
    Gal[p].SfrDiskColdGasMetals[step] = 0.0;
  }
}

/**
 * @brief   Triggers a starburst during galaxy interactions or mergers
 *
 * @param   mass_ratio         Mass ratio of the merging galaxies
 * @param   merger_centralgal  Index of the galaxy experiencing the starburst
 * @param   centralgal         Index of the central galaxy of the halo
 * @param   time               Current simulation time
 * @param   dt                 Time step size
 * @param   halonr             Index of the current halo
 * @param   mode               Burst mode: 0 for mergers, 1 for disk
 * instabilities
 * @param   step               Current time integration step
 *
 * This function implements the collisional starburst model from Somerville et
 * al. (2001) with updated coefficients from TJ Cox's PhD thesis. It calculates
 * the fraction of cold gas that is converted to stars during a merger or disk
 * instability event.
 *
 * The starburst efficiency depends on:
 * - The mass ratio of the interacting galaxies
 * - The mode of interaction (merger vs. disk instability)
 *
 * The function also handles:
 * - Supernova feedback from the starburst
 * - Gas reheating and ejection
 * - Metal production and distribution
 * - Morphological changes (stars formed go to the bulge)
 * - Potential triggering of disk instability in minor mergers
 *
 * References:
 * - Somerville et al. (2001) for the starburst recipe
 * - Cox (PhD thesis) for updated efficiency coefficients
 * - Krumholz & Dekel (2011) for metal distribution formulas
 */
void collisional_starburst_recipe(double mass_ratio, int merger_centralgal,
                                  int centralgal, double time, double dt,
                                  int halonr, int mode, int step) {
  double stars, reheated_mass, ejected_mass, fac, metallicity, eburst;
  double FracZleaveDiskVal;

  /* This is the major and minor merger starburst recipe of Somerville et al.
   * 2001. The coefficients in eburst are taken from TJ Cox's PhD thesis and
   * should be more accurate than previous. */

  /* Calculate the bursting fraction based on mode:
   * - mode=1 (disk instability): burst fraction equals mass ratio
   * - mode=0 (merger): burst fraction follows power law from Cox's thesis */
  if (mode == 1)
    eburst = mass_ratio; /* Disk instability mode */
  else
    eburst = 0.56 * pow(mass_ratio, 0.7); /* Merger-induced starburst */

  /* Calculate new stars formed from cold gas during the burst */
  stars = eburst * Gal[merger_centralgal].ColdGas;
  if (stars < 0.0)
    stars = 0.0;

  /* Calculate supernova feedback from the burst
   * This determines how much cold gas is reheated to hot phase */
  if (SageConfig.SupernovaRecipeOn == 1)
    reheated_mass = SageConfig.FeedbackReheatingEpsilon *
                    stars; /* Proportional to star formation */
  else
    reheated_mass = 0.0; /* No feedback if supernova recipe is off */

  assert(reheated_mass >= 0.0);

  /* Ensure mass conservation: can't use more cold gas than is available
   * If combined star formation + feedback exceeds available cold gas,
   * scale both down proportionally to fit within available gas */
  if ((stars + reheated_mass) > Gal[merger_centralgal].ColdGas) {
    fac = Gal[merger_centralgal].ColdGas / (stars + reheated_mass);
    stars *= fac;
    reheated_mass *= fac;
  }

  /* Calculate ejection of gas from the halo due to supernova energy
   * This uses an energy balance approach where SN energy is compared to
   * the binding energy of the halo (∝ Vvir²) */
  if (SageConfig.SupernovaRecipeOn == 1) {
    if (Gal[centralgal].Vvir > 0.0)
      ejected_mass =
          (SageConfig.FeedbackEjectionEfficiency * (EtaSNcode * EnergySNcode) /
               (Gal[centralgal].Vvir * Gal[centralgal].Vvir) -
           SageConfig.FeedbackReheatingEpsilon) *
          stars;
    else
      ejected_mass = 0.0;

    if (ejected_mass < 0.0)
      ejected_mass = 0.0; /* Prevent negative ejection */
  } else
    ejected_mass = 0.0; /* No ejection if supernova recipe is off */

  /* Record star formation in the bulge component
   * All stars formed during bursts are added to the bulge */
  Gal[merger_centralgal].SfrBulge[step] += stars / dt; /* Star formation rate */
  Gal[merger_centralgal].SfrBulgeColdGas[step] +=
      Gal[merger_centralgal].ColdGas; /* Record available gas */
  Gal[merger_centralgal].SfrBulgeColdGasMetals[step] +=
      Gal[merger_centralgal].MetalsColdGas; /* Record available metals */

  /* Calculate current cold gas metallicity for star formation */
  metallicity = get_metallicity(Gal[merger_centralgal].ColdGas,
                                Gal[merger_centralgal].MetalsColdGas);

  /* Update galaxy properties from star formation */
  update_from_star_formation(merger_centralgal, stars, metallicity);

  /* Add newly formed stars to the bulge (accounting for recycling) */
  Gal[merger_centralgal].BulgeMass += (1 - SageConfig.RecycleFraction) * stars;
  Gal[merger_centralgal].MetalsBulgeMass +=
      metallicity * (1 - SageConfig.RecycleFraction) * stars;

  /* Recalculate cold gas metallicity after star formation */
  metallicity = get_metallicity(Gal[merger_centralgal].ColdGas,
                                Gal[merger_centralgal].MetalsColdGas);

  /* Apply supernova feedback effects (gas reheating and ejection) */
  update_from_feedback(merger_centralgal, centralgal, reheated_mass,
                       ejected_mass, metallicity);

  /* For minor mergers, check if the remnant becomes unstable
   * Note: Disk instability is only checked for minor mergers (mass_ratio <
   * threshold) since major mergers already transform the remnant to a spheroid
   */
  if (SageConfig.DiskInstabilityOn && mode == 0)
    if (mass_ratio < SageConfig.ThreshMajorMerger)
      check_disk_instability(merger_centralgal, centralgal, halonr, time, dt,
                             step);

  /* Create new metals from the starburst (instantaneous recycling
   * approximation) Only considering Type II supernovae enrichment */
  if (Gal[merger_centralgal].ColdGas > 1e-8 &&
      mass_ratio < SageConfig.ThreshMajorMerger) {
    /* Calculate fraction of metals that leave the disk (Krumholz & Dekel 2011,
     * Eq. 22) In massive halos (Mvir >> 30), more metals are retained in the
     * cold gas */
    FracZleaveDiskVal =
        SageConfig.FracZleaveDisk * exp(-1.0 * Gal[centralgal].Mvir / 30.0);

    /* Distribute newly produced metals between cold and hot phases */
    Gal[merger_centralgal].MetalsColdGas +=
        SageConfig.Yield * (1.0 - FracZleaveDiskVal) *
        stars; /* Metals retained in cold gas */
    Gal[centralgal].MetalsHotGas += SageConfig.Yield * FracZleaveDiskVal *
                                    stars; /* Metals ejected to hot gas */
    /* Alternative approach (commented out in original code):
     * Gal[centralgal].MetalsEjectedMass += SageConfig.Yield * FracZleaveDiskVal
     * * stars; */
  } else
    /* For major mergers or negligible cold gas, all metals go to hot phase */
    Gal[centralgal].MetalsHotGas += SageConfig.Yield * stars;
  /* Alternative approach (commented out in original code):
   * Gal[centralgal].MetalsEjectedMass += Yield * stars; */
}

/**
 * @brief   Disrupts a satellite galaxy, transferring its stars to intracluster
 * stars
 *
 * @param   centralgal   Index of the central galaxy of the halo
 * @param   gal          Index of the satellite galaxy being disrupted
 *
 * This function handles the complete disruption of a satellite galaxy, where
 * instead of merging with the central galaxy, its stars are dispersed into
 * the intracluster stellar component (ICS). This typically occurs when a
 * satellite's dark matter subhalo has been significantly stripped.
 *
 * The gas components (cold, hot, ejected) are added to the central galaxy's
 * respective components, while the stellar mass is added to the ICS component.
 *
 * Note: The function leaves the black hole handling as an open question in
 * the original code.
 */
void disrupt_satellite_to_ICS(int centralgal, int gal) {
  /* Transfer gas components to the central galaxy */
  Gal[centralgal].HotGas +=
      Gal[gal].ColdGas + Gal[gal].HotGas; /* All gas becomes hot */
  Gal[centralgal].MetalsHotGas +=
      Gal[gal].MetalsColdGas + Gal[gal].MetalsHotGas;

  /* Transfer ejected mass */
  Gal[centralgal].EjectedMass += Gal[gal].EjectedMass;
  Gal[centralgal].MetalsEjectedMass += Gal[gal].MetalsEjectedMass;

  /* Transfer existing ICS */
  Gal[centralgal].ICS += Gal[gal].ICS;
  Gal[centralgal].MetalsICS += Gal[gal].MetalsICS;

  /* Add all stellar mass to intracluster stars */
  Gal[centralgal].ICS += Gal[gal].StellarMass;
  Gal[centralgal].MetalsICS += Gal[gal].MetalsStellarMass;

  /* Note regarding black holes (from original comment):
   * "what should we do with the disrupted satellite BH?" */

  /* Mark this satellite as disrupted */
  Gal[gal].mergeType = 4; /* Type 4 = disruption to ICS */
}
