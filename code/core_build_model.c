/**
 * @file    core_build_model.c
 *
 * This file contains the core algorithms for tracking dark matter halos from
 * merger trees. All baryonic physics has been removed.
 *
 * Key functions:
 * - build_halo_tree(): Recursive function to build halo tracking structures
 * - join_progenitor_halos(): Integrates halos from progenitor structures
 * - process_halo_evolution(): Updates halo properties (no physics applied)
 *
 * All physical processes removed (cooling, star formation, feedback, mergers).
 * This code now only tracks dark matter halo properties.
 *
 * References:
 * - Croton et al. (2006) - Original semi-analytic model framework
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
void build_halo_tree(int halonr, int tree) {
  int prog, fofhalo, ngal;

  HaloAux[halonr].DoneFlag = 1;

  prog = Halo[halonr].FirstProgenitor;
  while (prog >= 0) {
    if (HaloAux[prog].DoneFlag == 0)
      build_halo_tree(prog, tree);
    prog = Halo[prog].NextProgenitor;
  }

  fofhalo = Halo[halonr].FirstHaloInFOFgroup;
  if (HaloAux[fofhalo].HaloFlag == 0) {
    HaloAux[fofhalo].HaloFlag = 1;
    while (fofhalo >= 0) {
      prog = Halo[fofhalo].FirstProgenitor;
      while (prog >= 0) {
        if (HaloAux[prog].DoneFlag == 0)
          build_halo_tree(prog, tree);
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
      ngal = join_progenitor_halos(fofhalo, ngal);
      fofhalo = Halo[fofhalo].NextHaloInFOFgroup;
    }

    process_halo_evolution(Halo[halonr].FirstHaloInFOFgroup, ngal, tree);
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
int copy_progenitor_halos(int halonr, int ngalstart, int first_occupied) {
  int ngal, prog, i, j;
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
        if (Gal[ngal].MergeStatus != 0) {
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

          if (halonr == Halo[halonr].FirstHaloInFOFgroup) {
            // a central galaxy
            Gal[ngal].MergeStatus = 0;
            Gal[ngal].mergeIntoID = -1;
            Gal[ngal].MergTime = 999.9;

            Gal[ngal].Type = 0;
          } else {
            // a satellite with subhalo
            Gal[ngal].MergeStatus = 0;
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
              Gal[ngal].MergTime = 999.9; /* No merging without physics */

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
      init_halo_tracker(ngal, halonr);
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
void set_halo_centrals(int ngalstart, int ngal) {
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
int join_progenitor_halos(int halonr, int ngalstart) {
  int ngal, first_occupied;

  /* Find the most massive progenitor with galaxies */
  first_occupied = find_most_massive_progenitor(halonr);

  /* Copy galaxies from progenitors to the current snapshot */
  ngal = copy_progenitor_halos(halonr, ngalstart, first_occupied);

  /* Set up central galaxy relationships */
  set_halo_centrals(ngalstart, ngal);

  return ngal;
}

/**
 * @brief   Attaches halo tracking structures to halos for output
 *
 * @param   ngal          Total number of halos in this structure
 * @param   centralgal    Index of the central halo
 * @param   deltaT        Time interval for the entire timestep
 *
 * This function attaches halo tracking structures to halos for output.
 * All physics calculations have been removed. Simply copies halo structures
 * to output array (HaloGal).
 */
void update_halo_properties(int ngal, int centralgal, double deltaT) {
  int p, i, currenthalo, offset;

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
      if (Gal[i].MergeStatus > 0)
        if (Gal[p].mergeIntoID > Gal[i].mergeIntoID)
          offset++; /* These galaxies won't be kept, so offset mergeIntoID */
      i--;
    }

    /* Handle merged galaxies - update their merger info in the previous
     * snapshot */
    i = -1;
    if (Gal[p].MergeStatus > 0) {
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
      HaloGal[i].MergeStatus = Gal[p].MergeStatus;
      HaloGal[i].mergeIntoID = Gal[p].mergeIntoID - offset;
      HaloGal[i].mergeIntoSnapNum = Halo[currenthalo].SnapNum;
    }

    /* Copy non-merged galaxies to the permanent array */
    if (Gal[p].MergeStatus == 0) {
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
 * @brief   Updates halo properties for output
 *
 * @param   halonr    Index of the FOF-background subhalo (main halo)
 * @param   ngal      Total number of halos to process
 * @param   tree      Index of the current merger tree
 *
 * This function updates halo properties and prepares them for output.
 * All physics integration has been removed. Simply updates halo properties
 * and attaches to output structures.
 */
void process_halo_evolution(int halonr, int ngal,
                            int tree) /* Note: halonr is here the FOF-background
                                         subhalo (i.e. main halo) */
{
  double deltaT;
  int centralgal;

  /* Identify the central galaxy for this halo */
  centralgal = Gal[0].CentralGal;
  assert(Gal[centralgal].Type == 0 && Gal[centralgal].HaloNr == halonr);

  /* Update final galaxy properties and attach them to halos */
  deltaT = Age[Gal[0].SnapNum] - Age[Halo[halonr].SnapNum];
  update_halo_properties(ngal, centralgal, deltaT);
}
