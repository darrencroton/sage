# SAGE Code Structure Documentation

## Overview

SAGE (Semi-Analytic Galaxy Evolution) is a semi-analytic model for galaxy formation and evolution. The code processes dark matter merger trees to simulate the formation and evolution of galaxies within a cosmological context. This document provides an overview of the code structure, major components, and execution flow.

## Directory Structure

The codebase is organized as follows:

- `/code`: Main source code files
  - `/code/io`: Input/output routines for reading trees and writing galaxy catalogs
- `/extra`: Auxiliary data files, including cooling tables
- `/input`: Parameter files for configuring simulations
- `/output`: Analysis scripts and output data

## Major Components

### Core Components

- **Main Program Flow** (`main.c`): Entry point that handles initialization, tree processing, and output
- **Memory Management** (`core_mymalloc.c`): Custom memory allocation system for tracking usage
- **Tree Handling** (`core_io_tree.c`): Functions for loading merger trees from different formats
- **Galaxy Construction** (`core_build_model.c`): Functions for creating and evolving galaxies
- **Parameter Handling** (`core_read_parameter_file.c`, `parameter_table.c`): Reading and validating parameters
- **Error Handling** (`error_handling.c`): Structured error reporting system

### Physical Model Components

- **Star Formation** (`model_starformation_and_feedback.c`): Star formation and supernova feedback
- **Gas Cooling** (`model_cooling_heating.c`): Cooling of hot gas onto galaxies
- **Mergers** (`model_mergers.c`): Galaxy merging and resulting starbursts
- **Disk Instability** (`model_disk_instability.c`): Disk instability-triggered bulge formation
- **Gas Infall** (`model_infall.c`): Cosmological gas accretion onto halos
- **Reincorporation** (`model_reincorporation.c`): Return of ejected gas to the hot halo
- **Miscellaneous Physics** (`model_misc.c`): Various supporting physical calculations

### I/O Components

- **Binary Tree I/O** (`io/tree_binary.c`): Reading merger trees in binary format
- **HDF5 Tree I/O** (`io/tree_hdf5.c`): Reading merger trees in HDF5 format
- **Galaxy Output** (`core_save.c`, `io/io_save_hdf5.c`): Writing galaxy catalogs

## Execution Flow

1. **Initialization**:
   - Parse command-line arguments
   - Read parameter file
   - Initialize random number generator, cooling tables, output snapshot list
   - Setup memory allocation tracking

2. **Tree Processing Loop**:
   - Loop over all merger tree files
   - For each file, load the tree table and process each tree

3. **Galaxy Construction** (per tree):
   - Recursively process merger trees (depth-first traversal)
   - Construct galaxies for each halo, inheriting properties from progenitors
   - Evolve galaxies through time by applying physical processes

4. **Time Evolution** (per timestep):
   - Apply gas infall
   - Calculate cooling rates
   - Run star formation and feedback
   - Process galaxy mergers
   - Check for disk instabilities

5. **Output**:
   - Write galaxies to output files
   - Generate summary statistics

## Key Algorithms

### Merger Tree Processing

The code uses a recursive, depth-first algorithm to process merger trees, ensuring that all progenitors are processed before their descendants, maintaining proper causality in the galaxy formation process.

### Galaxy Evolution

Galaxies evolve using a set of coupled differential equations that track the flow of mass and metals between different components (hot gas, cold gas, stars, etc.). The time integration uses a fixed number of steps between snapshots.

### Feedback Model

The feedback model follows a two-stage process:
1. Supernova energy reheats cold gas to the hot phase
2. Additional energy may eject hot gas from the halo entirely

## Customization

The code behavior can be customized through the parameter file, allowing:
- Selection of different physical recipes
- Adjustment of efficiency parameters
- Configuration of input/output options
- Selection of cosmological parameters

## Header Files

- `constants.h`: Physical and numerical constants
- `types.h`: Structure definitions for galaxies and halos
- `globals.h`: Global variable declarations
- `config.h`: Configuration parameters and flags
- `core_proto.h`: Function prototypes
- `error_handling.h`: Error handling system definitions

Each header follows the principle of separation of concerns, with a specific focus on a particular aspect of the code.
