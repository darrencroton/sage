# Recommendation 2 Implementation: SimulationState Structure

## Overview

This document describes the implementation of Recommendation 2 from the SAGE codebase refactoring plan, which involved creating a `SimulationState` structure to encapsulate the runtime state of the simulation.

## Implementation Details

### 1. Structure Definition

The `SimulationState` structure was defined in `types.h` and contains the following fields:

```c
struct SimulationState
{
  /* Tree and galaxy counts */
  int Ntrees;               /* number of trees in current file */
  int NumGals;              /* Total number of galaxies stored for current tree */
  int MaxGals;              /* Maximum number of galaxies allowed for current tree */
  int FoF_MaxGals;          /* Maximum number of galaxies for FoF groups */
  int GalaxyCounter;        /* unique galaxy ID for main progenitor line in tree */
  int TotHalos;             /* Total number of halos */
  int TotGalaxies[ABSOLUTEMAXSNAPS]; /* Galaxy count per snapshot */
  
  /* File and tree identifiers */
  int FileNum;              /* Current file number being processed */
  int TreeID;               /* Current tree ID being processed */
  
  /* Snapshot information */
  int MAXSNAPS;             /* Maximum number of snapshots */
  int Snaplistlen;          /* Length of snapshot list */
  int ListOutputSnaps[ABSOLUTEMAXSNAPS]; /* List of output snapshot numbers */
  
  /* Derived arrays */
  int *TreeNgals[ABSOLUTEMAXSNAPS];
  int *FirstHaloInSnap;
  int *TreeNHalos;
  int *TreeFirstHalo;
};
```

### 2. Synchronization Functions

Two synchronization functions were implemented in `core_simulation_state.c` to maintain backward compatibility with existing code:

- `sync_sim_state_to_globals()`: Updates global variables from the `SimulationState` structure.
- `sync_globals_to_sim_state()`: Updates the `SimulationState` structure from global variables.

An initialization function `initialize_sim_state()` was also implemented to set initial values for the `SimulationState` structure.

### 3. Integration in the Codebase

The `SimulationState` structure was integrated into the codebase through the following steps:

1. Added global declaration in `globals.h`: `extern struct SimulationState SimState;`
2. Modified `main.c` to initialize the structure after calling `init()`: `initialize_sim_state();`
3. Updated key files that use simulation state variables to use the `SimState` structure instead:
   - `core_io_tree.c`: Updated tree table handling and file operations
   - `core_build_model.c`: Updated galaxy construction and evolution
   - `core_save.c`: Updated galaxy saving functionality
   - `model_misc.c`: Updated galaxy initialization

4. Added backward compatibility synchronization throughout the code to maintain existing functionality

### 4. Compilation Support

Updated the `Makefile` to include the new `core_simulation_state.c` file in the build process.

## Benefits

1. **Improved code organization**: Related state variables are now grouped together in a logical structure.
2. **Reduced global variable usage**: Global variables were replaced with structure members.
3. **Better maintainability**: Code is now more modular and easier to understand.
4. **Backward compatibility**: Existing code still works while supporting the new structure-based approach.

## Future Directions

1. Continue refactoring code to directly use the `SimulationState` structure instead of global variables.
2. Remove synchronization code once all references to global variables are replaced with structure references.
3. Consider adding more state variables to the structure as appropriate.
