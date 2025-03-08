# SimulationState Structure Documentation

## Overview

The `SimulationState` structure is part of SAGE's refactoring effort to reduce global variable usage and improve code organization. This structure encapsulates the runtime state of the simulation, making it easier to track and manage as the simulation progresses.

## Structure Definition

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
};
```

## Field Descriptions

### Tree and Galaxy Counts

- `Ntrees`: Number of merger trees in the current file being processed.
- `NumGals`: Total number of galaxies stored for the current tree.
- `MaxGals`: Maximum number of galaxies allocated for the current tree.
- `FoF_MaxGals`: Maximum number of galaxies for Friends-of-Friends groups.
- `GalaxyCounter`: A unique ID counter for galaxies in the main progenitor line.
- `TotHalos`: Total number of halos in the simulation.
- `TotGalaxies`: Array tracking the number of galaxies per snapshot.

### File and Tree Identifiers

- `FileNum`: The current file number being processed.
- `TreeID`: The current tree ID being processed.

### Snapshot Information

- `MAXSNAPS`: Maximum number of snapshots in the simulation.
- `Snaplistlen`: Length of the snapshot list.
- `ListOutputSnaps`: Array listing the snapshot numbers to output.

## Usage Guidelines

During the transition phase, the `SimulationState` structure is used alongside global variables to maintain backward compatibility. The `sync_globals_with_config()` and `sync_config_with_globals()` functions in `core_allvars.c` handle the synchronization between the structure fields and their global counterparts.

When modifying any of these variables, prefer updating the `SimulationState` structure field first, then call `sync_config_with_globals()` to update the corresponding global variable.

### Example:

```c
// Set galaxy counter to zero
SimState.GalaxyCounter = 0;
sync_config_with_globals();  // Update global variable for backward compatibility
```

## Future Directions

Eventually, all code will transition to use the `SimulationState` structure directly, eliminating the need for the corresponding global variables and synchronization functions.

For new code, always use the structure fields rather than the global variables to promote this transition.
