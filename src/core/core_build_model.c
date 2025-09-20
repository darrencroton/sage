/**
 * @file    core_build_model.c
 * @brief   Core functions for building and evolving the galaxy formation model
 *
 * This file contains the core algorithms for constructing galaxies from merger
 * trees and evolving them through time. It implements the main semi-analytic
 * modeling framework including the construction of galaxies from their
 * progenitors, the application of physical processes (cooling, star formation,
 * feedback), handling of mergers, and the time integration scheme.
 *
 * Key functions:
 * - construct_galaxies(): Recursive function to build galaxies through merger
 * trees
 * - join_galaxies_of_progenitors(): Integrates galaxies from progenitor halos
 * - evolve_galaxies(): Main time integration function for galaxy evolution
 * - apply_physical_processes(): Applies all physical processes to galaxies
 * - handle_mergers(): Processes galaxy mergers and disruption events
 *
 * References:
 * - Croton et al. (2006) - Main semi-analytic model framework
 * - White & Frenk (1991) - Cooling model
 * - Kauffmann et al. (1999) - Star formation implementation
 * - Somerville et al. (2001) - Merger model
 */

#include <assert.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "core_proto.h"
#include "globals.h"
#include "types.h"
#include "util_numeric.h"

/**
 * @brief   Recursively constructs galaxies by traversing the merger tree
 *
 * @param   halonr    Index of the current halo in the Halo array
 * @param   tree      Index of the current merger tree
 *
 * This function traverses the merger tree in a depth-first manner to ensure
 * that galaxies are constructed from their progenitors before being evolved.
 * It follows these steps:
 *
 * 1. First processes all progenitors of the current halo
 * 2. Then processes all halos in the same FOF group
 * 3. Finally, joins progenitor galaxies and evolves them forward in time
 *
 * The recursive approach ensures that galaxies are built in the correct
 * chronological order, preserving the flow of mass and properties from
 * high redshift to low redshift.
 */
void construct_galaxies(int halonr, int tree) {
  int prog, fofhalo, ngal;

  HaloAux[halonr].DoneFlag = 1;

  prog = Halo[halonr].FirstProgenitor;
  while (prog >= 0) {
    if (HaloAux[prog].DoneFlag == 0)
      construct_galaxies(prog, tree);
    prog = Halo[prog].NextProgenitor;
  }

  fofhalo = Halo[halonr].FirstHaloInFOFgroup;
  if (HaloAux[fofhalo].HaloFlag == 0) {
    HaloAux[fofhalo].HaloFlag = 1;
    while (fofhalo >= 0) {
      prog = Halo[fofhalo].FirstProgenitor;
      while (prog >= 0) {
        if (HaloAux[prog].DoneFlag == 0)
          construct_galaxies(prog, tree);
        prog = Halo[prog].NextProgenitor;
      }

      fofhalo = Halo[fofhalo].NextHaloInFOFgroup;
    }
  }

  // At this point, the galaxies for all progenitors of this halo have been
  // properly constructed. Also, the galaxies of the progenitors of all other
  // halos in the same FOF group have been constructed as well. We can hence go
  // ahead and construct all galaxies for the subhalos in this FOF halo, and
  // evolve them in time.

  fofhalo = Halo[halonr].FirstHaloInFOFgroup;
  if (HaloAux[fofhalo].HaloFlag == 1) {
    ngal = 0;
    HaloAux[fofhalo].HaloFlag = 2;

    while (fofhalo >= 0) {
      ngal = join_galaxies_of_progenitors(fofhalo, ngal);
      fofhalo = Halo[fofhalo].NextHaloInFOFgroup;
    }

    evolve_galaxies(Halo[halonr].FirstHaloInFOFgroup, ngal, tree);
  }
}

/**
 * @brief   Finds the most massive progenitor halo that contains a galaxy
 *
 * @param   halonr    Index of the current halo in the Halo array
 * @return  Index of the most massive progenitor with a galaxy
 *
 * This function scans all progenitors of a halo to find the most massive one
 * that actually contains a galaxy. This is important because not all dark
 * matter halos necessarily host galaxies, and we need to identify the main
 * branch for inheriting galaxy properties.
 *
 * Two criteria are tracked:
 * 1. The most massive progenitor overall (by particle count)
 * 2. The most massive progenitor that contains a galaxy
 *
 * The function returns the index of the most massive progenitor containing a
 * galaxy, which is used to determine which galaxy should become the central
 * galaxy of the descendant halo.
 */
int find_most_massive_progenitor(int halonr) {
  int prog, first_occupied, lenmax, lenoccmax;

  lenmax = 0;
  lenoccmax = 0;
  first_occupied = Halo[halonr].FirstProgenitor;
  prog = Halo[halonr].FirstProgenitor;

  if (prog >= 0)
    if (HaloAux[prog].NGalaxies > 0)
      lenoccmax = -1;

  // Find most massive progenitor that contains an actual galaxy
  // Maybe FirstProgenitor never was FirstHaloInFOFGroup and thus has no galaxy
  while (prog >= 0) {
    if (Halo[prog].Len > lenmax) {
      lenmax = Halo[prog].Len;
      /* mother_halo = prog; */
    }
    if (lenoccmax != -1 && Halo[prog].Len > lenoccmax &&
        HaloAux[prog].NGalaxies > 0) {
      lenoccmax = Halo[prog].Len;
      first_occupied = prog;
    }
    prog = Halo[prog].NextProgenitor;
  }

  return first_occupied;
}

/**
 * @brief   Copies and updates galaxies from progenitor halos to the current
 * snapshot
 *
 * @param   halonr          Index of the current halo in the Halo array
 * @param   ngalstart       Starting index for galaxies in the Gal array
 * @param   first_occupied  Index of the most massive progenitor with galaxies
 * @return  Updated number of galaxies after copying
 *
 * This function transfers galaxies from progenitor halos to the current
 * snapshot, updating their properties based on the new halo structure. It
 * handles:
 *
 * 1. Copying galaxies from all progenitors to the temporary Gal array
 * 2. Updating galaxy properties based on their new host halo
 * 3. Handling type transitions (central → satellite → orphan)
 * 4. Setting appropriate merger times for satellites
 * 5. Creating new galaxies when a halo has no progenitor galaxies
 *
 * The function maintains the continuity of galaxy evolution by preserving
 * their properties while updating their status based on the evolving
 * dark matter structures.
 */
int copy_galaxies_from_progenitors(int halonr, int ngalstart,
                                   int first_occupied) {
  int ngal, prog, i, j, step;
  double previousMvir, previousVvir, previousVmax;

  ngal = ngalstart;
  prog = Halo[halonr].FirstProgenitor;

  while (prog >= 0) {
    for (i = 0; i < HaloAux[prog].NGalaxies; i++) {
      if (ngal == (FoF_MaxGals - 1)) {
        /* Calculate new size using growth factor */
        int new_size = (int)(FoF_MaxGals * GALAXY_ARRAY_GROWTH_FACTOR);

        /* Ensure minimum growth to prevent too-frequent reallocations */
        if (new_size - FoF_MaxGals < MIN_GALAXY_ARRAY_GROWTH)
          new_size = FoF_MaxGals + MIN_GALAXY_ARRAY_GROWTH;

        /* Cap maximum size to prevent excessive memory usage */
        if (new_size > MAX_GALAXY_ARRAY_SIZE)
          new_size = MAX_GALAXY_ARRAY_SIZE;

        INFO_LOG("Growing galaxy array from %d to %d elements", FoF_MaxGals,
                 new_size);

        /* Reallocate with new size */
        FoF_MaxGals = new_size;
        Gal = myrealloc(Gal, FoF_MaxGals * sizeof(struct GALAXY));
        SimState.FoF_MaxGals = FoF_MaxGals; /* Update SimState directly */
      }
      assert(ngal < FoF_MaxGals);

      // This is the crucial line in which the properties of the progenitor
      // galaxies are copied over (as a whole) to the (temporary) galaxies
      // Gal[xxx] in the current snapshot After updating their properties and
      // evolving them they are copied to the end of the list of permanent
      // galaxies HaloGal[xxx]
      Gal[ngal] = HaloGal[HaloAux[prog].FirstGalaxy + i];
      Gal[ngal].HaloNr = halonr;
      Gal[ngal].dT = -1.0;

      // this deals with the central galaxies of (sub)halos
      if (Gal[ngal].Type == 0 || Gal[ngal].Type == 1) {
        // this halo shouldn't hold a galaxy that has already merged; remove it
        // from future processing
        if (Gal[ngal].mergeType != 0) {
          Gal[ngal].Type = 3;
          continue;
        }

        // remember properties from the last snapshot
        previousMvir = Gal[ngal].Mvir;
        previousVvir = Gal[ngal].Vvir;
        previousVmax = Gal[ngal].Vmax;

        if (prog == first_occupied) {
          // update properties of this galaxy with physical properties of halo
          Gal[ngal].MostBoundID = Halo[halonr].MostBoundID;

          for (j = 0; j < 3; j++) {
            Gal[ngal].Pos[j] = Halo[halonr].Pos[j];
            Gal[ngal].Vel[j] = Halo[halonr].Vel[j];
          }

          Gal[ngal].Len = Halo[halonr].Len;
          Gal[ngal].Vmax = Halo[halonr].Vmax;

          Gal[ngal].deltaMvir = get_virial_mass(halonr) - Gal[ngal].Mvir;

          if (is_greater(get_virial_mass(halonr), Gal[ngal].Mvir)) {
            Gal[ngal].Rvir =
                get_virial_radius(halonr); // use the maximum Rvir in model
            Gal[ngal].Vvir =
                get_virial_velocity(halonr); // use the maximum Vvir in model
          }
          Gal[ngal].Mvir = get_virial_mass(halonr);

          Gal[ngal].Cooling = 0.0;
          Gal[ngal].Heating = 0.0;
          Gal[ngal].QuasarModeBHaccretionMass = 0.0;
          Gal[ngal].OutflowRate = 0.0;

          for (step = 0; step < STEPS; step++) {
            Gal[ngal].SfrDisk[step] = Gal[ngal].SfrBulge[step] = 0.0;
            Gal[ngal].SfrDiskColdGas[step] =
                Gal[ngal].SfrDiskColdGasMetals[step] = 0.0;
            Gal[ngal].SfrBulgeColdGas[step] =
                Gal[ngal].SfrBulgeColdGasMetals[step] = 0.0;
          }

          if (halonr == Halo[halonr].FirstHaloInFOFgroup) {
            // a central galaxy
            Gal[ngal].mergeType = 0;
            Gal[ngal].mergeIntoID = -1;
            Gal[ngal].MergTime = 999.9;

            Gal[ngal].DiskScaleRadius = get_disk_radius(halonr, ngal);

            Gal[ngal].Type = 0;
          } else {
            // a satellite with subhalo
            Gal[ngal].mergeType = 0;
            Gal[ngal].mergeIntoID = -1;

            if (Gal[ngal].Type ==
                0) // remember the infall properties before becoming a subhalo
            {
              Gal[ngal].infallMvir = previousMvir;
              Gal[ngal].infallVvir = previousVvir;
              Gal[ngal].infallVmax = previousVmax;
            }

            if (Gal[ngal].Type == 0 || is_greater(Gal[ngal].MergTime, 999.0))
              // here the galaxy has gone from type 1 to type 2 or otherwise
              // doesn't have a merging time.
              Gal[ngal].MergTime = estimate_merging_time(
                  halonr, Halo[halonr].FirstHaloInFOFgroup, ngal);

            Gal[ngal].Type = 1;
          }
        } else {
          // an orphan satellite galaxy - these will merge or disrupt within the
          // current timestep
          Gal[ngal].deltaMvir = -1.0 * Gal[ngal].Mvir;
          Gal[ngal].Mvir = 0.0;

          if (is_greater(Gal[ngal].MergTime, 999.0) || Gal[ngal].Type == 0) {
            // here the galaxy has gone from type 0 to type 2 - merge it!
            Gal[ngal].MergTime = 0.0;

            Gal[ngal].infallMvir = previousMvir;
            Gal[ngal].infallVvir = previousVvir;
            Gal[ngal].infallVmax = previousVmax;
          }

          Gal[ngal].Type = 2;
        }
      }

      ngal++;
    }

    prog = Halo[prog].NextProgenitor;
  }

  if (ngal == ngalstart) {
    // We have no progenitors with galaxies. This means we create a new galaxy.
    // init_galaxy requires halonr to be the main subhalo
    if (halonr == Halo[halonr].FirstHaloInFOFgroup) {
      init_galaxy(ngal, halonr);
      ngal++;
    }
    // If not the main subhalo, we don't create a galaxy - this seems to be
    // the behavior of the original code based on the assertion in init_galaxy
  }

  return ngal;
}

/**
 * @brief   Sets the central galaxy reference for all galaxies in a halo
 *
 * @param   ngalstart    Starting index of galaxies for this halo
 * @param   ngal         Ending index (exclusive) of galaxies for this halo
 *
 * This function identifies the central galaxy (Type 0 or 1) for a halo
 * and sets all galaxies in the halo to reference this central galaxy.
 * Each halo can have only one Type 0 or Type 1 galaxy, with all others
 * being Type 2 (orphan) galaxies.
 */
void set_galaxy_centrals(int ngalstart, int ngal) {
  int i, centralgal;

  /* Per Halo there can be only one Type 0 or 1 galaxy, all others are Type 2
   * (orphan) Find the central galaxy for this halo */
  for (i = ngalstart, centralgal = -1; i < ngal; i++) {
    if (Gal[i].Type == 0 || Gal[i].Type == 1) {
      assert(centralgal == -1); /* Ensure only one central galaxy per halo */
      centralgal = i;
    }
  }

  /* Set all galaxies to point to the central galaxy */
  for (i = ngalstart; i < ngal; i++)
    Gal[i].CentralGal = centralgal;
}

/**
 * @brief   Main function to join galaxies from progenitor halos
 *
 * @param   halonr       Index of the current halo in the Halo array
 * @param   ngalstart    Starting index for galaxies in the Gal array
 * @return  Updated number of galaxies after joining
 *
 * This function coordinates the process of integrating galaxies from
 * progenitor halos into the current halo. It performs three main steps:
 *
 * 1. Identifies the most massive progenitor with galaxies
 * 2. Copies and updates galaxies from all progenitors
 * 3. Establishes relationships between galaxies (central/satellite)
 *
 * The function ensures proper inheritance of galaxy properties while
 * maintaining the hierarchy of central and satellite galaxies.
 */
int join_galaxies_of_progenitors(int halonr, int ngalstart) {
  int ngal, first_occupied;

  /* Find the most massive progenitor with galaxies */
  first_occupied = find_most_massive_progenitor(halonr);

  /* Copy galaxies from progenitors to the current snapshot */
  ngal = copy_galaxies_from_progenitors(halonr, ngalstart, first_occupied);

  /* Set up central galaxy relationships */
  set_galaxy_centrals(ngalstart, ngal);

  return ngal;
}

/**
 * @brief   Applies physical processes to all galaxies for a single timestep
 *
 * @param   ngal          Total number of galaxies in this halo
 * @param   centralgal    Index of the central galaxy
 * @param   halonr        Index of the current halo
 * @param   infallingGas  Amount of infalling gas for this timestep
 * @param   step          Current substep in the time integration
 *
 * This function implements the core physical processes that drive galaxy
 * evolution during a single timestep. For each galaxy, it:
 *
 * 1. Calculates the appropriate time step
 * 2. For central galaxies:
 *    - Adds gas from cosmological infall
 *    - Reincorporates previously ejected gas (if enabled)
 * 3. For satellites with subhalos:
 *    - Strips hot gas through environmental effects
 * 4. For all galaxies:
 *    - Calculates gas cooling
 *    - Applies star formation and feedback
 *
 * The implementation follows the standard semi-analytic prescription where
 * physical processes occur in a specific order during each timestep.
 */
void apply_physical_processes(int ngal, int centralgal, int halonr,
                              double infallingGas, int step) {
  int p;
  double coolingGas, deltaT, time;

  /* Loop over all galaxies in the halo */
  for (p = 0; p < ngal; p++) {
    /* Skip galaxies that have already merged */
    if (Gal[p].mergeType > 0)
      continue;

    /* Calculate deltaT (time interval) and time (current cosmic time) for this
     * galaxy */
    deltaT = Age[Gal[p].SnapNum] - Age[Halo[halonr].SnapNum];
    time = Age[Gal[p].SnapNum] - (step + 0.5) * (deltaT / STEPS);

    /* Set time step for galaxies that don't have it yet */
    if (is_less(Gal[p].dT, 0.0))
      Gal[p].dT = deltaT;

    /* Special processes for the central galaxy */
    if (p == centralgal) {
      /* Add cosmological gas infall for this step */
      add_infall_to_hot(centralgal, infallingGas / STEPS);

      /* Reincorporate previously ejected gas if enabled */
      if (SageConfig.ReIncorporationFactor > 0.0)
        reincorporate_gas(centralgal, deltaT / STEPS);
    } else
      /* For satellite galaxies with subhalos and hot gas, apply environmental
       * stripping */
      if (Gal[p].Type == 1 && is_greater(Gal[p].HotGas, 0.0))
        strip_from_satellite(halonr, centralgal, p);

    /* Calculate cooling gas based on halo properties */
    coolingGas = cooling_recipe(p, deltaT / STEPS);
    cool_gas_onto_galaxy(p, coolingGas);

    /* Apply star formation and feedback processes */
    starformation_and_feedback(p, centralgal, time, deltaT / STEPS, halonr,
                               step);
  }
}

/**
 * @brief   Handles merger and disruption events for satellite galaxies
 *
 * @param   ngal          Total number of galaxies in this halo
 * @param   centralgal    Index of the central galaxy
 * @param   halonr        Index of the current halo
 * @param   step          Current substep in the time integration
 *
 * This function processes potential merger and disruption events for satellite
 * galaxies. For each satellite galaxy, it:
 *
 * 1. Updates the remaining time until merging
 * 2. Checks if the satellite meets the criteria for disruption or merging:
 *    - Satellites with no baryonic mass
 *    - Satellites with low dark matter to baryonic mass ratios
 * 3. Handles galaxy disruption (stars added to intracluster stars)
 * 4. Processes galaxy mergers with the appropriate central galaxy
 *
 * The function implements the dynamical friction timescale approach to galaxy
 * mergers and includes environmental effects that can lead to satellite
 * disruption.
 */
void handle_mergers(int ngal, int centralgal, int halonr, int step) {
  int p, merger_centralgal;
  double time, galaxyBaryons, currentMvir, deltaT;

  /* Check for satellite disruption and merger events */
  for (p = 0; p < ngal; p++) {
    /* Only process satellite galaxies that haven't already been marked for
     * merging */
    if ((Gal[p].Type == 1 || Gal[p].Type == 2) && Gal[p].mergeType == 0) {
      /* All satellites should have a valid merger time */
      assert(is_less(Gal[p].MergTime, 999.0));

      /* Update remaining time until merging */
      deltaT = Age[Gal[p].SnapNum] - Age[Halo[halonr].SnapNum];
      Gal[p].MergTime -= deltaT / STEPS;

      /* Calculate current halo mass accounting for linear mass loss during the
       * step */
      currentMvir =
          Gal[p].Mvir -
          Gal[p].deltaMvir * (1.0 - safe_div((double)step + 1.0, (double)STEPS,
                                             EPSILON_SMALL));
      galaxyBaryons = Gal[p].StellarMass + Gal[p].ColdGas;

      /* Check if satellite meets disruption/merger criteria:
       * 1. Has no baryonic mass (will never grow)
       * 2. Has a dark matter to baryonic mass ratio below threshold */
      if (is_zero(galaxyBaryons) ||
          (is_greater(galaxyBaryons, 0.0) &&
           is_less_or_equal(safe_div(currentMvir, galaxyBaryons, EPSILON_SMALL),
                            SageConfig.ThresholdSatDisruption))) {
        /* Determine which galaxy this satellite will merge into */
        if (Gal[p].Type == 1)
          merger_centralgal =
              centralgal; /* Type 1 satellites merge with halo central */
        else
          merger_centralgal = Gal[p].CentralGal; /* Type 2 orphans merge with
                                                    their designated central */

        /* If target has also merged, redirect to its merger target */
        if (Gal[merger_centralgal].mergeType > 0)
          merger_centralgal = Gal[merger_centralgal].CentralGal;

        /* Set the ID that this galaxy will merge into (in the output array) */
        Gal[p].mergeIntoID = NumGals + merger_centralgal;

        /* Handle satellite disruption if merger time hasn't expired */
        if (is_greater(Gal[p].MergTime, 0.0)) {
          disrupt_satellite_to_ICS(merger_centralgal, p);
        } else if (is_less_or_equal(Gal[p].MergTime,
                                    0.0)) /* Handle galaxy merger */
        {
          /* Calculate the cosmic time for this merger event */
          time = Age[Gal[p].SnapNum] - (step + 0.5) * (deltaT / STEPS);
          deal_with_galaxy_merger(p, merger_centralgal, centralgal, time,
                                  deltaT / STEPS, halonr, step);
        }
      }
    }
  }
}

/**
 * @brief   Updates final galaxy properties and attaches galaxies to halos
 *
 * @param   ngal          Total number of galaxies in this halo
 * @param   centralgal    Index of the central galaxy
 * @param   deltaT        Time interval for the entire timestep
 *
 * This function finalizes galaxy properties after all physical processes and
 * merger events have been applied. It:
 *
 * 1. Converts accumulated values to rates (e.g., cooling, heating, outflow)
 * 2. Calculates the total baryon mass in satellite galaxies
 * 3. Updates merger information for galaxies that have merged
 * 4. Copies surviving galaxies to the permanent galaxy array (HaloGal)
 *
 * This function is called at the end of the time integration to prepare
 * galaxies for output and further processing in subsequent snapshots.
 */
void update_galaxy_properties(int ngal, int centralgal, double deltaT) {
  int p, i, currenthalo, offset;

  /* Reset total satellite baryons counter for the central galaxy */
  Gal[centralgal].TotalSatelliteBaryons = 0.0;

  /* Calculate rates and update satellite baryons */
  for (p = 0; p < ngal; p++) {
    /* Skip galaxies that have already merged */
    if (Gal[p].mergeType > 0)
      continue;

    /* Convert accumulated values to rates by dividing by the time interval */
    Gal[p].Cooling /= deltaT;
    Gal[p].Heating /= deltaT;
    Gal[p].OutflowRate /= deltaT;

    /* For satellite galaxies, add their baryon mass to the central's counter */
    if (p != centralgal)
      Gal[centralgal].TotalSatelliteBaryons +=
          (Gal[p].StellarMass + Gal[p].BlackHoleMass + Gal[p].ColdGas +
           Gal[p].HotGas);
  }

  /* Attach final galaxy list to halos */
  offset = 0;
  for (p = 0, currenthalo = -1; p < ngal; p++) {
    /* When processing a new halo, update its galaxy pointers */
    if (Gal[p].HaloNr != currenthalo) {
      currenthalo = Gal[p].HaloNr;
      HaloAux[currenthalo].FirstGalaxy =
          NumGals; /* Index of first galaxy in this halo */
      HaloAux[currenthalo].NGalaxies = 0; /* Reset galaxy counter */
    }

    /* Calculate offset for merger target IDs due to galaxies that won't be
     * output */
    offset = 0;
    i = p - 1;
    while (i >= 0) {
      if (Gal[i].mergeType > 0)
        if (Gal[p].mergeIntoID > Gal[i].mergeIntoID)
          offset++; /* These galaxies won't be kept, so offset mergeIntoID */
      i--;
    }

    /* Handle merged galaxies - update their merger info in the previous
     * snapshot */
    i = -1;
    if (Gal[p].mergeType > 0) {
      /* Find this galaxy in the previous snapshot's array */
      i = HaloAux[currenthalo].FirstGalaxy - 1;
      while (i >= 0) {
        if (HaloGal[i].GalaxyNr == Gal[p].GalaxyNr)
          break;
        else
          i--;
      }

      assert(i >= 0); /* Galaxy should always be found */

      /* Update merger information in the previous snapshot's entry */
      HaloGal[i].mergeType = Gal[p].mergeType;
      HaloGal[i].mergeIntoID = Gal[p].mergeIntoID - offset;
      HaloGal[i].mergeIntoSnapNum = Halo[currenthalo].SnapNum;
    }

    /* Copy non-merged galaxies to the permanent array */
    if (Gal[p].mergeType == 0) {
      assert(NumGals < MaxGals); /* Ensure we don't exceed array bounds */

      Gal[p].SnapNum = Halo[currenthalo].SnapNum; /* Update snapshot number */
      HaloGal[NumGals++] =
          Gal[p]; /* Copy to permanent array and increment counter */
      SimState.NumGals = NumGals; /* Update SimState after increment */
      HaloAux[currenthalo]
          .NGalaxies++; /* Increment galaxy count for this halo */
    }
  }
}

/**
 * @brief   Main function to evolve galaxies through time
 *
 * @param   halonr    Index of the FOF-background subhalo (main halo)
 * @param   ngal      Total number of galaxies to evolve
 * @param   tree      Index of the current merger tree
 *
 * This function implements the time integration of galaxy properties between
 * two consecutive snapshots. It:
 *
 * 1. Calculates the total infalling gas for the central galaxy
 * 2. Integrates forward in time using a series of substeps (STEPS)
 * 3. For each substep:
 *    a. Applies physical processes (infall, cooling, star formation)
 *    b. Handles merger and disruption events
 * 4. Updates final galaxy properties and attaches them to halos
 *
 * This is the core evolution function that coordinates all the physical
 * processes in the semi-analytic model.
 */
void evolve_galaxies(int halonr, int ngal,
                     int tree) /* Note: halonr is here the FOF-background
                                  subhalo (i.e. main halo) */
{
  int step;
  double deltaT;
  int centralgal;
  double infallingGas;

  /* Identify the central galaxy for this halo */
  centralgal = Gal[0].CentralGal;
  assert(Gal[centralgal].Type == 0 && Gal[centralgal].HaloNr == halonr);

  /* Calculate infalling gas once, outside the time step loop */
  infallingGas = infall_recipe(centralgal, ngal, ZZ[Halo[halonr].SnapNum]);

  /* Integrate forward in time using STEPS intervals */
  for (step = 0; step < STEPS; step++) {
    /* Apply physical processes (infall, cooling, star formation) */
    apply_physical_processes(ngal, centralgal, halonr, infallingGas, step);

    /* Handle mergers and disruption events */
    handle_mergers(ngal, centralgal, halonr, step);
  }

  /* Update final galaxy properties and attach them to halos */
  deltaT = Age[Gal[0].SnapNum] - Age[Halo[halonr].SnapNum];
  update_galaxy_properties(ngal, centralgal, deltaT);
}
