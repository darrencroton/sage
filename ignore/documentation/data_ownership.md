# Data Ownership Documentation for SAGE Global Data Structures

## Overview

This document specifies the ownership rules and lifecycle management for SAGE's large global data structures. While we're migrating configuration parameters and runtime state variables to structured containers, certain large arrays that represent the core simulation data will remain global. This document establishes clear conventions for accessing and modifying these structures to reduce the risks associated with global state.

## Core Data Structures

### Galaxy Arrays

```c
struct GALAXY *Gal, *HaloGal;
```

These arrays hold the galaxy data that is the primary output of the simulation.

#### Ownership
- **Primary Owner**: `construct_galaxies()` in `core_build_model.c`
- **Memory Allocation**: `load_tree()` and `construct_galaxies()`
- **Memory Deallocation**: `free_galaxies_and_tree()`

#### Access Rules
1. **Read Access**: Any function may read from these arrays.
2. **Write Access**: Limited to the following functions and their callees:
   - `init_galaxy()`
   - `join_galaxies_of_progenitors()`
   - `evolve_galaxies()`
   - `starformation_and_feedback()`
   - `update_from_star_formation()`
   - `update_from_feedback()`
   - `check_disk_instability()`
   - `cooling_recipe()`
   - `deal_with_galaxy_merger()`
   - `add_galaxies_together()`
   - `grow_black_hole()`
   - `quasar_mode_wind()`
   - `disrupt_satellite_to_ICS()`
   - `strip_from_satellite()`
   - `reincorporate_gas()`

3. **Array Resizing**: Only performed in `construct_galaxies()` using `myrealloc()`

### Halo Data

```c
struct halo_data *Halo;
struct halo_aux_data *HaloAux;
```

These arrays contain the dark matter halo data that forms the backbone of the merger trees.

#### Ownership
- **Primary Owner**: `load_tree()` in tree I/O module
- **Memory Allocation**: `load_tree()`
- **Memory Deallocation**: `free_galaxies_and_tree()`

#### Access Rules
1. **Read Access**: Any function may read from these arrays.
2. **Write Access for Halo**: Limited to functions in the tree I/O module.
3. **Write Access for HaloAux**: 
   - `construct_galaxies()`
   - `join_galaxies_of_progenitors()`
   - Initial values set in `load_tree()`

### Snapshot Arrays

```c
double ZZ[ABSOLUTEMAXSNAPS];
double AA[ABSOLUTEMAXSNAPS];
double *Age;
int ListOutputSnaps[ABSOLUTEMAXSNAPS];
int *TreeNgals[ABSOLUTEMAXSNAPS];
int TotGalaxies[ABSOLUTEMAXSNAPS];
```

These arrays track snapshot-related data including redshifts, scale factors, ages, and galaxy counts.

#### Ownership
- **Primary Owner**: 
  - For ZZ, AA, Age: `init()` in `core_init.c`
  - For ListOutputSnaps: `read_parameter_file()` in `core_read_parameter_file.c`
  - For TreeNgals, TotGalaxies: `load_tree_table()` and `save_galaxies()`
- **Memory Allocation/Deallocation**: 
  - Static arrays (ZZ, AA, ListOutputSnaps, TotGalaxies): No explicit allocation
  - Age: Allocated in `init()`, deallocated in `main()`
  - TreeNgals: Allocated in `load_tree_table()`, deallocated in `free_tree_table()`

#### Access Rules
1. **Read Access**: Any function may read from these arrays.
2. **Write Access**:
   - ZZ, AA: Only in `init()`
   - Age: Only in `init()`
   - ListOutputSnaps: Only in `read_parameter_file()`
   - TreeNgals: Only in `load_tree_table()` and `save_galaxies()`
   - TotGalaxies: Only in `load_tree_table()` and `save_galaxies()`

### Tree Structure Arrays

```c
int *TreeNHalos;
int *TreeFirstHalo;
int *FirstHaloInSnap;
```

These arrays define the structure of merger trees.

#### Ownership
- **Primary Owner**: `load_tree_table()` in tree I/O module
- **Memory Allocation**: `load_tree_table()`
- **Memory Deallocation**: `free_tree_table()`

#### Access Rules
1. **Read Access**: Any function may read from these arrays.
2. **Write Access**: Limited to functions in the tree I/O module.

### Random Number Generator

```c
gsl_rng *random_generator;
```

The GSL random number generator used throughout the simulation.

#### Ownership
- **Primary Owner**: `init()` in `core_init.c`
- **Memory Allocation**: `init()`
- **Memory Deallocation**: `main()`

#### Access Rules
1. **Read Access**: Any function may get random numbers using `gsl_rng_uniform()`.
2. **Write Access**: Only `init()` and `main()` may modify the generator state except for seed setting.
3. **Seed Setting**: Only in `main()` before constructing galaxies for deterministic results.

## Tree and File Identifiers

```c
int TreeID;
int FileNum;
```

These variables track the current tree and file being processed.

#### Ownership
- **Primary Owner**: `main()`
- **Memory Allocation**: Static globals, no allocation needed
- **Memory Deallocation**: N/A

#### Access Rules
1. **Read Access**: Any function may read these values.
2. **Write Access**: Only `main()` should modify these values.

## General Guidelines for Global Data

1. **Initialization Order**:
   - Configuration parameters (SageConfig) are initialized first in `read_parameter_file()`
   - Physics state (PhysicsState) is initialized in `set_units()`
   - Simulation state (SimState) is initialized and updated throughout execution
   - Data structures are allocated and initialized as needed during execution

2. **Synchronization**:
   - Always call `sync_globals_with_config()` after modifying SageConfig fields
   - Always call `sync_config_with_globals()` after modifying global variables that have structure counterparts

3. **Error Handling**:
   - Always check allocation results for data structures
   - Use assertions to validate critical assumptions about data structures
   - Handle edge cases explicitly where appropriate

4. **Cleanup Order**:
   - Data structures should be freed in the reverse order of allocation
   - Special case: Age array requires pointer adjustment before freeing
