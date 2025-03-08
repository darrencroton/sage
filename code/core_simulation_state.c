#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "types.h"
#include "globals.h"
#include "config.h"
#include "core_proto.h"
#include "error_handling.h"

/*
 * sync_sim_state_to_globals
 * 
 * Updates global variables from SimState structure.
 * Used to maintain backward compatibility with existing code.
 */
void sync_sim_state_to_globals(void)
{
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
  
  /* Snapshot information - handled differently as these can also be in SageConfig */
  MAXSNAPS = SimState.MAXSNAPS;
  Snaplistlen = SimState.Snaplistlen;
  NOUT = SimState.NOUT;
  memcpy(ListOutputSnaps, SimState.ListOutputSnaps, sizeof(int) * ABSOLUTEMAXSNAPS);
  
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

/*
 * sync_globals_to_sim_state
 * 
 * Updates SimState structure from global variables.
 * Used to initialize the structure with current global state.
 */
void sync_globals_to_sim_state(void)
{
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
  memcpy(SimState.ListOutputSnaps, ListOutputSnaps, sizeof(int) * ABSOLUTEMAXSNAPS);
  
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

/*
 * initialize_sim_state
 * 
 * Sets initial values in the SimState structure
 */
void initialize_sim_state(void)
{
  /* Initialize the structure from global variables */
  sync_globals_to_sim_state();
  
  /* Update MAXSNAPS, Snaplistlen, NOUT, and ListOutputSnaps from SageConfig */
  SimState.MAXSNAPS = SageConfig.MAXSNAPS;
  SimState.Snaplistlen = SageConfig.Snaplistlen;
  SimState.NOUT = SageConfig.NOUT;
  memcpy(SimState.ListOutputSnaps, SageConfig.ListOutputSnaps, sizeof(int) * ABSOLUTEMAXSNAPS);
  
  /* Synchronize back to globals for consistency */
  sync_sim_state_to_globals();
  
  DEBUG_LOG("Simulation state initialized. NOUT=%d, MAXSNAPS=%d", 
            SimState.NOUT, SimState.MAXSNAPS);
}
