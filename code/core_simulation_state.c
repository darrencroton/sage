/**
 * @file    core_simulation_state.c
 * @brief   Functions for managing the simulation state structure
 *
 * This file implements functionality for managing the SAGE simulation state.
 * It provides functions to synchronize between the SimState structure and
 * global variables, maintaining backward compatibility with existing code
 * while supporting a more encapsulated approach using the SimState structure.
 *
 * The SimState structure centralizes all simulation state variables that
 * were previously maintained as global variables, making it easier to
 * save and restore simulation state, track changes, and reduce global
 * variable dependencies.
 *
 * Key functions:
 * - sync_sim_state_to_globals(): Copies SimState values to global variables
 * - sync_globals_to_sim_state(): Copies global variable values to SimState
 * - initialize_sim_state(): Sets up initial SimState values
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "constants.h"
#include "core_proto.h"
#include "globals.h"
#include "types.h"
#include "util_error.h"

/**
 * @brief   Updates global variables from the SimState structure
 *
 * This function copies all values from the SimState structure to their
 * corresponding global variables. It's used to maintain backward compatibility
 * with existing code that relies on global variables, while allowing the
 * simulation state to be encapsulated in the SimState structure.
 *
 * The function handles various types of data:
 * 1. Simple scalar values (counters, IDs)
 * 2. Arrays (via memcpy)
 * 3. Pointers (with special care to maintain pointer relationships)
 *
 * This synchronization is necessary when the SimState structure has been
 * modified and those changes need to be reflected in the global variables
 * used throughout the rest of the code.
 */
void sync_sim_state_to_globals(void) {
  /* Tree and galaxy counts */
  Ntrees = SimState.Ntrees;
  NumGals = SimState.NumGals;
  MaxGals = SimState.MaxGals;
  FoF_MaxGals = SimState.FoF_MaxGals;
  GalaxyCounter = SimState.GalaxyCounter;
  TotHalos = SimState.TotHalos;

  /* Copy array values */
  memcpy(TotGalaxies, SimState.TotGalaxies, sizeof(int) * ABSOLUTEMAXSNAPS);

  /* File and tree identifiers */
  FileNum = SimState.FileNum;
  TreeID = SimState.TreeID;

  /* Snapshot information - handled differently as these can also be in
   * SageConfig */
  MAXSNAPS = SimState.MAXSNAPS;
  Snaplistlen = SimState.Snaplistlen;
  NOUT = SimState.NOUT;
  memcpy(ListOutputSnaps, SimState.ListOutputSnaps,
         sizeof(int) * ABSOLUTEMAXSNAPS);

  /* Pointers - these need special care */
  for (int i = 0; i < ABSOLUTEMAXSNAPS; i++) {
    if (i < NOUT) {
      TreeNgals[i] = SimState.TreeNgals[i];
    }
  }
  FirstHaloInSnap = SimState.FirstHaloInSnap;
  TreeNHalos = SimState.TreeNHalos;
  TreeFirstHalo = SimState.TreeFirstHalo;
}

/**
 * @brief   Updates the SimState structure from global variables
 *
 * This function copies all values from global variables to their
 * corresponding fields in the SimState structure. It's typically used
 * to initialize the SimState structure with the current global state
 * or to ensure the structure is up to date after global variables
 * have been modified.
 *
 * The function handles various types of data:
 * 1. Simple scalar values (counters, IDs)
 * 2. Arrays (via memcpy)
 * 3. Pointers (with special care to maintain pointer relationships)
 *
 * This synchronization is the inverse of sync_sim_state_to_globals()
 * and allows the simulation state to be captured and stored in a
 * single, self-contained structure.
 */
void sync_globals_to_sim_state(void) {
  /* Tree and galaxy counts */
  SimState.Ntrees = Ntrees;
  SimState.NumGals = NumGals;
  SimState.MaxGals = MaxGals;
  SimState.FoF_MaxGals = FoF_MaxGals;
  SimState.GalaxyCounter = GalaxyCounter;
  SimState.TotHalos = TotHalos;

  /* Copy array values */
  memcpy(SimState.TotGalaxies, TotGalaxies, sizeof(int) * ABSOLUTEMAXSNAPS);

  /* File and tree identifiers */
  SimState.FileNum = FileNum;
  SimState.TreeID = TreeID;

  /* Snapshot information */
  SimState.MAXSNAPS = MAXSNAPS;
  SimState.Snaplistlen = Snaplistlen;
  SimState.NOUT = NOUT;
  memcpy(SimState.ListOutputSnaps, ListOutputSnaps,
         sizeof(int) * ABSOLUTEMAXSNAPS);

  /* Pointers - these need special care */
  for (int i = 0; i < ABSOLUTEMAXSNAPS; i++) {
    if (i < NOUT) {
      SimState.TreeNgals[i] = TreeNgals[i];
    } else {
      SimState.TreeNgals[i] = NULL;
    }
  }
  SimState.FirstHaloInSnap = FirstHaloInSnap;
  SimState.TreeNHalos = TreeNHalos;
  SimState.TreeFirstHalo = TreeFirstHalo;
}

/**
 * @brief   Initializes the SimState structure with default values
 *
 * This function sets up the initial values in the SimState structure
 * by first synchronizing from global variables and then updating
 * specific fields from the SageConfig structure. It ensures that the
 * simulation state is properly initialized at the start of a simulation
 * run with consistent values across all related data structures.
 *
 * The function:
 * 1. Copies current global variable values to SimState
 * 2. Updates snapshot-related fields from SageConfig
 * 3. Synchronizes back to globals to ensure consistency
 *
 * After this function completes, both the SimState structure and
 * global variables will contain the same, consistent set of values
 * for the simulation state.
 */
void initialize_sim_state(void) {
  /* Initialize the structure from global variables */
  sync_globals_to_sim_state();

  /* Update MAXSNAPS, Snaplistlen, NOUT, and ListOutputSnaps from SageConfig */
  SimState.MAXSNAPS = SageConfig.MAXSNAPS;
  SimState.Snaplistlen = SageConfig.Snaplistlen;
  SimState.NOUT = SageConfig.NOUT;
  memcpy(SimState.ListOutputSnaps, SageConfig.ListOutputSnaps,
         sizeof(int) * ABSOLUTEMAXSNAPS);

  /* Synchronize back to globals for consistency */
  sync_sim_state_to_globals();
}
