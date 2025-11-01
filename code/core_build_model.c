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
 * @brief   Recursively constructs halos by traversing the merger tree
 *
 * @param   halonr    Index of the current halo in the Halo array
 * @param   tree      Index of the current merger tree
 *
 * This function traverses the merger tree in a depth-first manner to ensure
 * that halos are constructed from their progenitors before being evolved.
 * It follows these steps:
 *
 * 1. First processes all progenitors of the current halo
 * 2. Then processes all halos in the same FOF group
 * 3. Finally, joins progenitor halos and evolves them forward in time
 *
 * The recursive approach ensures that halos are built in the correct
 * chronological order, preserving the flow of mass and properties from
 * high redshift to low redshift.
 */
void build_halo_tree(int halonr, int tree) {
  int prog, fofhalo, ngal;

  HaloAux[halonr].DoneFlag = 1;

  prog = TreeHalos[halonr].FirstProgenitor;
  while (prog >= 0) {
    if (HaloAux[prog].DoneFlag == 0)
      build_halo_tree(prog, tree);
    prog = TreeHalos[prog].NextProgenitor;
  }

  fofhalo = TreeHalos[halonr].FirstHaloInFOFgroup;
  if (HaloAux[fofhalo].HaloFlag == 0) {
    HaloAux[fofhalo].HaloFlag = 1;
    while (fofhalo >= 0) {
      prog = TreeHalos[fofhalo].FirstProgenitor;
      while (prog >= 0) {
        if (HaloAux[prog].DoneFlag == 0)
          build_halo_tree(prog, tree);
        prog = TreeHalos[prog].NextProgenitor;
      }

      fofhalo = TreeHalos[fofhalo].NextHaloInFOFgroup;
    }
  }

  // At this point, the halos for all progenitors of this halo have been
  // properly constructed. Also, the halos of the progenitors of all other
  // halos in the same FOF group have been constructed as well. We can hence go
  // ahead and construct all halos for the subhalos in this FOF halo, and
  // evolve them in time.

  fofhalo = TreeHalos[halonr].FirstHaloInFOFgroup;
  if (HaloAux[fofhalo].HaloFlag == 1) {
    ngal = 0;
    HaloAux[fofhalo].HaloFlag = 2;

    while (fofhalo >= 0) {
      ngal = join_progenitor_halos(fofhalo, ngal);
      fofhalo = TreeHalos[fofhalo].NextHaloInFOFgroup;
    }

    process_halo_evolution(TreeHalos[halonr].FirstHaloInFOFgroup, ngal, tree);
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
 * matter halos necessarily host halos, and we need to identify the main
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
  first_occupied = TreeHalos[halonr].FirstProgenitor;
  prog = TreeHalos[halonr].FirstProgenitor;

  if (prog >= 0)
    if (HaloAux[prog].NHalos > 0)
      lenoccmax = -1;

  // Find most massive progenitor that contains an actual galaxy
  // Maybe FirstProgenitor never was FirstHaloInFOFGroup and thus has no galaxy
  while (prog >= 0) {
    if (TreeHalos[prog].Len > lenmax) {
      lenmax = TreeHalos[prog].Len;
      /* mother_halo = prog; */
    }
    if (lenoccmax != -1 && TreeHalos[prog].Len > lenoccmax &&
        HaloAux[prog].NHalos > 0) {
      lenoccmax = TreeHalos[prog].Len;
      first_occupied = prog;
    }
    prog = TreeHalos[prog].NextProgenitor;
  }

  return first_occupied;
}

/**
 * @brief   Copies and updates halos from progenitor halos to the current
 * snapshot
 *
 * @param   halonr          Index of the current halo in the Halo array
 * @param   ngalstart       Starting index for halos in the Gal array
 * @param   first_occupied  Index of the most massive progenitor with halos
 * @return  Updated number of halos after copying
 *
 * This function transfers halos from progenitor halos to the current
 * snapshot, updating their properties based on the new halo structure. It
 * handles:
 *
 * 1. Copying halos from all progenitors to the temporary Gal array
 * 2. Updating galaxy properties based on their new host halo
 * 3. Handling type transitions (central → satellite → orphan)
 * 4. Setting appropriate merger times for satellites
 * 5. Creating new halos when a halo has no progenitor halos
 *
 * The function maintains the continuity of galaxy evolution by preserving
 * their properties while updating their status based on the evolving
 * dark matter structures.
 */
int copy_progenitor_halos(int halonr, int ngalstart, int first_occupied) {
  int ngal, prog, i, j;
  double previousMvir, previousVvir, previousVmax;

  ngal = ngalstart;
  prog = TreeHalos[halonr].FirstProgenitor;

  while (prog >= 0) {
    for (i = 0; i < HaloAux[prog].NHalos; i++) {
      if (ngal == (MaxWorkingHalos - 1)) {
        /* Calculate new size using growth factor */
        int new_size = (int)(MaxWorkingHalos * HALO_ARRAY_GROWTH_FACTOR);

        /* Ensure minimum growth to prevent too-frequent reallocations */
        if (new_size - MaxWorkingHalos < MIN_HALO_ARRAY_GROWTH)
          new_size = MaxWorkingHalos + MIN_HALO_ARRAY_GROWTH;

        /* Cap maximum size to prevent excessive memory usage */
        if (new_size > MAX_GALAXY_ARRAY_SIZE)
          new_size = MAX_GALAXY_ARRAY_SIZE;

        INFO_LOG("Growing galaxy array from %d to %d elements", MaxWorkingHalos,
                 new_size);

        /* Reallocate with new size */
        MaxWorkingHalos = new_size;
        WorkingHalos = myrealloc(WorkingHalos, MaxWorkingHalos * sizeof(struct Halo));
        SimState.MaxWorkingHalos = MaxWorkingHalos; /* Update SimState directly */
      }
      assert(ngal < MaxWorkingHalos);

      // This is the crucial line in which the properties of the progenitor
      // halos are copied over (as a whole) to the (temporary) halos
      // WorkingHalos[xxx] in the current snapshot After updating their properties and
      // evolving them they are copied to the end of the list of permanent
      // halos CurrentTreeHalos[xxx]
      WorkingHalos[ngal] = CurrentTreeHalos[HaloAux[prog].FirstHalo + i];
      WorkingHalos[ngal].HaloNr = halonr;
      WorkingHalos[ngal].dT = -1.0;

      // this deals with the central halos of (sub)halos
      if (WorkingHalos[ngal].Type == 0 || WorkingHalos[ngal].Type == 1) {
        // this halo shouldn't hold a galaxy that has already merged; remove it
        // from future processing
        if (WorkingHalos[ngal].MergeStatus != 0) {
          WorkingHalos[ngal].Type = 3;
          continue;
        }

        // remember properties from the last snapshot
        previousMvir = WorkingHalos[ngal].Mvir;
        previousVvir = WorkingHalos[ngal].Vvir;
        previousVmax = WorkingHalos[ngal].Vmax;

        if (prog == first_occupied) {
          // update properties of this galaxy with physical properties of halo
          WorkingHalos[ngal].MostBoundID = TreeHalos[halonr].MostBoundID;

          for (j = 0; j < 3; j++) {
            WorkingHalos[ngal].Pos[j] = TreeHalos[halonr].Pos[j];
            WorkingHalos[ngal].Vel[j] = TreeHalos[halonr].Vel[j];
          }

          WorkingHalos[ngal].Len = TreeHalos[halonr].Len;
          WorkingHalos[ngal].Vmax = TreeHalos[halonr].Vmax;

          WorkingHalos[ngal].deltaMvir = get_virial_mass(halonr) - WorkingHalos[ngal].Mvir;

          if (is_greater(get_virial_mass(halonr), WorkingHalos[ngal].Mvir)) {
            WorkingHalos[ngal].Rvir =
                get_virial_radius(halonr); // use the maximum Rvir in model
            WorkingHalos[ngal].Vvir =
                get_virial_velocity(halonr); // use the maximum Vvir in model
          }
          WorkingHalos[ngal].Mvir = get_virial_mass(halonr);

          if (halonr == TreeHalos[halonr].FirstHaloInFOFgroup) {
            // a central galaxy
            WorkingHalos[ngal].MergeStatus = 0;
            WorkingHalos[ngal].mergeIntoID = -1;
            WorkingHalos[ngal].MergTime = 999.9;

            WorkingHalos[ngal].Type = 0;
          } else {
            // a satellite with subhalo
            WorkingHalos[ngal].MergeStatus = 0;
            WorkingHalos[ngal].mergeIntoID = -1;

            if (WorkingHalos[ngal].Type ==
                0) // remember the infall properties before becoming a subhalo
            {
              WorkingHalos[ngal].infallMvir = previousMvir;
              WorkingHalos[ngal].infallVvir = previousVvir;
              WorkingHalos[ngal].infallVmax = previousVmax;
            }

            if (WorkingHalos[ngal].Type == 0 || is_greater(WorkingHalos[ngal].MergTime, 999.0))
              // here the galaxy has gone from type 1 to type 2 or otherwise
              // doesn't have a merging time.
              WorkingHalos[ngal].MergTime = 999.9; /* No merging without physics */

            WorkingHalos[ngal].Type = 1;
          }
        } else {
          // an orphan satellite galaxy - these will merge or disrupt within the
          // current timestep
          WorkingHalos[ngal].deltaMvir = -1.0 * WorkingHalos[ngal].Mvir;
          WorkingHalos[ngal].Mvir = 0.0;

          if (is_greater(WorkingHalos[ngal].MergTime, 999.0) || WorkingHalos[ngal].Type == 0) {
            // here the galaxy has gone from type 0 to type 2 - merge it!
            WorkingHalos[ngal].MergTime = 0.0;

            WorkingHalos[ngal].infallMvir = previousMvir;
            WorkingHalos[ngal].infallVvir = previousVvir;
            WorkingHalos[ngal].infallVmax = previousVmax;
          }

          WorkingHalos[ngal].Type = 2;
        }
      }

      ngal++;
    }

    prog = TreeHalos[prog].NextProgenitor;
  }

  if (ngal == ngalstart) {
    // We have no progenitors with halos. This means we create a new galaxy.
    // init_galaxy requires halonr to be the main subhalo
    if (halonr == TreeHalos[halonr].FirstHaloInFOFgroup) {
      init_halo(ngal, halonr);
      ngal++;
    }
    // If not the main subhalo, we don't create a galaxy - this seems to be
    // the behavior of the original code based on the assertion in init_galaxy
  }

  return ngal;
}

/**
 * @brief   Sets the central galaxy reference for all halos in a halo
 *
 * @param   ngalstart    Starting index of halos for this halo
 * @param   ngal         Ending index (exclusive) of halos for this halo
 *
 * This function identifies the central galaxy (Type 0 or 1) for a halo
 * and sets all halos in the halo to reference this central galaxy.
 * Each halo can have only one Type 0 or Type 1 galaxy, with all others
 * being Type 2 (orphan) halos.
 */
void set_halo_centrals(int ngalstart, int ngal) {
  int i, centralgal;

  /* Per Halo there can be only one Type 0 or 1 galaxy, all others are Type 2
   * (orphan) Find the central galaxy for this halo */
  for (i = ngalstart, centralgal = -1; i < ngal; i++) {
    if (WorkingHalos[i].Type == 0 || WorkingHalos[i].Type == 1) {
      assert(centralgal == -1); /* Ensure only one central galaxy per halo */
      centralgal = i;
    }
  }

  /* Set all halos to point to the central galaxy */
  for (i = ngalstart; i < ngal; i++)
    WorkingHalos[i].CentralHalo = centralgal;
}

/**
 * @brief   Main function to join halos from progenitor halos
 *
 * @param   halonr       Index of the current halo in the Halo array
 * @param   ngalstart    Starting index for halos in the Gal array
 * @return  Updated number of halos after joining
 *
 * This function coordinates the process of integrating halos from
 * progenitor halos into the current halo. It performs three main steps:
 *
 * 1. Identifies the most massive progenitor with halos
 * 2. Copies and updates halos from all progenitors
 * 3. Establishes relationships between halos (central/satellite)
 *
 * The function ensures proper inheritance of galaxy properties while
 * maintaining the hierarchy of central and satellite halos.
 */
int join_progenitor_halos(int halonr, int ngalstart) {
  int ngal, first_occupied;

  /* Find the most massive progenitor with halos */
  first_occupied = find_most_massive_progenitor(halonr);

  /* Copy halos from progenitors to the current snapshot */
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
    if (WorkingHalos[p].HaloNr != currenthalo) {
      currenthalo = WorkingHalos[p].HaloNr;
      HaloAux[currenthalo].FirstHalo =
          NumCurrentTreeHalos; /* Index of first galaxy in this halo */
      HaloAux[currenthalo].NHalos = 0; /* Reset galaxy counter */
    }

    /* Calculate offset for merger target IDs due to halos that won't be
     * output */
    offset = 0;
    i = p - 1;
    while (i >= 0) {
      if (WorkingHalos[i].MergeStatus > 0)
        if (WorkingHalos[p].mergeIntoID > WorkingHalos[i].mergeIntoID)
          offset++; /* These halos won't be kept, so offset mergeIntoID */
      i--;
    }

    /* Handle merged halos - update their merger info in the previous
     * snapshot */
    i = -1;
    if (WorkingHalos[p].MergeStatus > 0) {
      /* Find this galaxy in the previous snapshot's array */
      i = HaloAux[currenthalo].FirstHalo - 1;
      while (i >= 0) {
        if (CurrentTreeHalos[i].UniqueHaloID == WorkingHalos[p].UniqueHaloID)
          break;
        else
          i--;
      }

      assert(i >= 0); /* Galaxy should always be found */

      /* Update merger information in the previous snapshot's entry */
      CurrentTreeHalos[i].MergeStatus = WorkingHalos[p].MergeStatus;
      CurrentTreeHalos[i].mergeIntoID = WorkingHalos[p].mergeIntoID - offset;
      CurrentTreeHalos[i].mergeIntoSnapNum = TreeHalos[currenthalo].SnapNum;
    }

    /* Copy non-merged halos to the permanent array */
    if (WorkingHalos[p].MergeStatus == 0) {
      assert(NumCurrentTreeHalos < MaxCurrentTreeHalos); /* Ensure we don't exceed array bounds */

      WorkingHalos[p].SnapNum = TreeHalos[currenthalo].SnapNum; /* Update snapshot number */
      CurrentTreeHalos[NumCurrentTreeHalos++] =
          WorkingHalos[p]; /* Copy to permanent array and increment counter */
      SimState.NumCurrentTreeHalos = NumCurrentTreeHalos; /* Update SimState after increment */
      HaloAux[currenthalo]
          .NHalos++; /* Increment galaxy count for this halo */
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
  centralgal = WorkingHalos[0].CentralHalo;
  assert(WorkingHalos[centralgal].Type == 0 && WorkingHalos[centralgal].HaloNr == halonr);

  /* Update final galaxy properties and attach them to halos */
  deltaT = Age[WorkingHalos[0].SnapNum] - Age[TreeHalos[halonr].SnapNum];
  update_halo_properties(ngal, centralgal, deltaT);
}
