# SAGE Codebase Organization

## Directory Structure

The SAGE (Semi-Analytic Galaxy Evolution) codebase follows a modular organization:

- **code/** - Main source code directory
  - **io/** - Input/output routines for tree reading and output writing
- **extra/** - Auxiliary data (cooling tables, etc.)
- **input/** - Parameter files
- **output/** - Analysis scripts and output data

## Code Organization

### Core Components

- **main.c** - Entry point, handles execution flow and tree processing
- **core_allvars.h/c** - Global variables and structures
- **core_build_model.c** - Galaxy construction and evolution algorithms
- **core_mymalloc.c** - Custom memory management
- **core_io_tree.c** - Merger tree loading
- **core_read_parameter_file.c** - Configuration handling
- **core_save.c** - Galaxy catalog output
- **error_handling.h/c** - Error messaging system
- **parameter_table.h/c** - Parameter definitions and validation

### Header Organization

- **config.h** - Configuration parameters and flags
- **constants.h** - Physical and numerical constants
- **types.h** - Structure definitions
- **globals.h** - Global variable declarations
- **core_proto.h** - Function prototypes

### Physical Models

- **model_starformation_and_feedback.c** - Star formation and supernova feedback
- **model_cooling_heating.c** - Gas cooling and AGN heating
- **model_mergers.c** - Galaxy mergers and morphological transformation
- **model_disk_instability.c** - Disk instability detection and bulge formation
- **model_infall.c** - Cosmological gas infall and stripping
- **model_reincorporation.c** - Gas reincorporation from ejected reservoir

### I/O Components

- **io/tree_binary.c/h** - Binary merger tree reading
- **io/tree_hdf5.c/h** - HDF5 merger tree reading
- **io/io_save_hdf5.c/h** - HDF5 galaxy catalog writing

## Execution Flow

1. **Initialization** (`main.c`):
   - Process command-line arguments
   - Read parameter file
   - Initialize error handling, memory management
   - Set up physical constants and tables

2. **Tree Processing** (`main.c`):
   - Loop through merger tree files
   - Process each tree in the file

3. **Galaxy Construction** (`core_build_model.c`):
   - Recursively process merger trees
   - Connect progenitor galaxies to descendants
   - Apply physical processes for evolution

4. **Physical Processes** (various model_*.c files):
   - Gas cooling and heating
   - Star formation and feedback
   - Galaxy mergers and morphological changes
   - Black hole growth and AGN feedback
   - Gas infall and reincorporation

5. **Output Generation** (`core_save.c`):
   - Save processed galaxies to output files
