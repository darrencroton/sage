# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

**PHYSICS DISABLED**: This version of SAGE has been converted to a **dark matter (DM) halo tracker only**. All baryonic physics has been removed.

## Quick Setup

For new repository clones, use the automated setup script:

```bash
# Complete setup from fresh clone
./first_run.sh

# This creates directories, downloads data, sets up Python environment
# Creates sage_venv/ virtual environment with plotting dependencies
```

## Build Commands

```bash
# Basic compilation
make

# With HDF5 support for HDF5 tree format
make USE-HDF5=yes

# With MPI support for parallel processing
make USE-MPI=yes

# Clean build artifacts
make clean

# Remove object files but keep executable
make tidy
```

## Code Formatting

```bash
# Format all C and Python code
./beautify.sh

# Format only C code (requires clang-format)
./beautify.sh --c-only

# Format only Python code (requires black and isort)
./beautify.sh --py-only
```

## Running SAGE

```bash
# Basic execution
./sage input/millennium.par

# With command-line options
./sage --verbose input/millennium.par
./sage --quiet input/millennium.par
./sage --skip input/millennium.par
```

## Testing

```bash
# Test the plotting system (activate virtual environment first)
source sage_venv/bin/activate
cd output/sage-plot
./test_plotting.sh

# Generate all halo plots (both snapshot and evolution - new default behavior)
python sage-plot.py --param-file=../../input/millennium.par

# Generate specific plots
python sage-plot.py --param-file=../../input/millennium.par --plots=halo_mass_function,spin_distribution

# Generate only snapshot plots (5 halo plots)
python sage-plot.py --param-file=../../input/millennium.par --snapshot-plots

# Generate only evolution plots (1 halo plot)
python sage-plot.py --param-file=../../input/millennium.par --evolution-plots

# Cross-directory execution now works from anywhere
cd ../..
python output/sage-plot/sage-plot.py --param-file=input/millennium.par --plots=halo_mass_function

# Deactivate when done
deactivate
```

## Code Architecture

### Core Execution Flow
- **main.c**: Program entry point, handles initialization, file processing loop, and cleanup
- **core_init.c**: System initialization, memory setup, parameter validation
- **core_read_parameter_file.c**: Parameter file parsing and configuration setup
- **core_build_model.c**: Halo tracking and property updates (PHYSICS DISABLED)

### Physical Models (model_*.c files) - **PHYSICS DISABLED**
These files remain in the codebase but their physics functions are no longer called:
- **model_cooling_heating.c**: Gas cooling and heating processes (disabled)
- **model_starformation_and_feedback.c**: Star formation and supernova feedback (disabled)
- **model_mergers.c**: Galaxy merger handling and black hole growth (disabled)
- **model_infall.c**: Gas infall calculations (disabled)
- **model_reincorporation.c**: Gas reincorporation from hot to cold phase (disabled)
- **model_disk_instability.c**: Disk instability and bulge formation (disabled)
- **model_misc.c**: Halo initialization (active, physics-related functions removed)

### I/O System
- **io_tree.c**: Master tree loading interface
- **io_tree_binary.c**: Binary format tree reader (LHalo format)
- **io_tree_hdf5.c**: HDF5 format tree reader (Genesis format)
- **io_save_binary.c**: Binary output format writer (halo properties only)
- **io_save_hdf5.c**: HDF5 output format writer (halo properties only, 24 fields)

### Utilities
- **util_memory.c**: Custom memory management with leak detection and categorization
- **util_error.c**: Comprehensive error handling and logging system
- **util_numeric.c**: Numerical stability functions and safe math operations
- **util_parameters.c**: Parameter processing and validation
- **util_integration.c**: Numerical integration routines

### Data Structures
- **types.h**: Core data structures (halo properties only, physics fields removed)
  - `struct GALAXY`: Halo tracking structure (24 fields)
  - `struct GALAXY_OUTPUT`: Output structure (24 fields)
  - `struct SageConfig`: Configuration parameters
- **globals.h**: Global variable declarations and simulation state
- **constants.h**: Numerical constants (physics constants removed)
- **config.h**: Compile-time configuration options

### Key Design Patterns
1. **Modular Physics**: Each physical process is isolated in its own module (PHYSICS DISABLED - modules remain but are not called)
2. **Memory Categories**: Memory allocation is tracked by category (halos, trees, parameters, etc.)
3. **Error Propagation**: Consistent error handling with context preservation throughout the call stack
4. **Format Abstraction**: I/O operations abstracted to support multiple tree and output formats
5. **State Management**: Simulation state cleanly separated from halo data

### Plotting System (output/sage-plot/)
- **sage-plot.py**: Central plotting script with enhanced command-line interface
- **figures/**: Modular plot implementations (6 halo plot types)
  - 5 snapshot plots: halo_mass_function, halo_occupation, spin_distribution, velocity_distribution, spatial_distribution
  - 1 evolution plot: hmf_evolution (halo mass function evolution)
  - **figures/archive/**: 15 galaxy-physics plots archived for potential future use
- **snapshot_redshift_mapper.py**: Handles snapshot-redshift conversions with robust path resolution
- **Enhanced Features**:
  - Default behavior generates both snapshot and evolution plots
  - Cross-directory execution with automatic path resolution
  - Robust parameter file parsing (handles comments, arrow notation)
  - Consistent flag naming (`--evolution-plots`, `--snapshot-plots`)
- Each plot module follows consistent interface: `plot(galaxies, volume, metadata, params, output_dir, output_format)`

### Parameter File Structure
Parameter files use a key-value format with sections for:
- File information (FirstFile, LastFile, OutputDir)
- Simulation parameters (BoxSize, Hubble_h, Omega, PartMass)
- **PHYSICS DISABLED**: All recipe flags and physical parameters removed (SFprescription, AGNrecipeOn, etc.)

### Tree Processing Flow
1. **load_tree_table()**: Load tree metadata and structure
2. **construct_galaxies()**: Initialize halo tracking structures from merger trees
3. **evolve_galaxies()**: Update halo properties (PHYSICS DISABLED - no physics models applied)
4. **save_galaxies()**: Write halo properties to output files
5. **free_galaxies_and_tree()**: Clean up memory

### Memory Management
Uses custom allocator with categorized tracking:
- MEMORY_GALAXY: Halo tracking data structures (still named "GALAXY" in code)
- MEMORY_TREE: Merger tree data
- MEMORY_PARAMETER: Configuration data
- MEMORY_UTIL: Utility arrays and buffers
Call `print_allocated()` to check for memory leaks.

### Documentation Standards
Follow the documentation template in `code/doc_standards.md`:
- Function headers with @brief, @param, @return
- File headers explaining purpose and key functions
- Inline comments for complex calculations
- Units explicitly stated for physical quantities