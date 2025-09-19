# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

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
./sage --overwrite input/millennium.par
```

## Testing

```bash
# Test the plotting system (activate virtual environment first)
source sage_venv/bin/activate
cd output/sage-plot
./test_plotting.sh

# Generate specific plots
python sage-plot.py --param-file=../../input/millennium.par --plots=stellar_mass_function

# Deactivate when done
deactivate
```

## Code Architecture

### Core Execution Flow
- **main.c**: Program entry point, handles initialization, file processing loop, and cleanup
- **core_init.c**: System initialization, memory setup, parameter validation
- **core_read_parameter_file.c**: Parameter file parsing and configuration setup
- **core_build_model.c**: Galaxy construction and evolution coordination

### Physical Models (model_*.c files)
- **model_cooling_heating.c**: Gas cooling and heating processes
- **model_starformation_and_feedback.c**: Star formation and supernova feedback
- **model_mergers.c**: Galaxy merger handling and black hole growth
- **model_infall.c**: Gas infall calculations
- **model_reincorporation.c**: Gas reincorporation from hot to cold phase
- **model_disk_instability.c**: Disk instability and bulge formation
- **model_misc.c**: Miscellaneous processes (stripping, disruption)

### I/O System
- **io_tree.c**: Master tree loading interface
- **io_tree_binary.c**: Binary format tree reader (LHalo format)
- **io_tree_hdf5.c**: HDF5 format tree reader (Genesis format)
- **io_save_binary.c**: Binary output format writer
- **io_save_hdf5.c**: HDF5 output format writer

### Utilities
- **util_memory.c**: Custom memory management with leak detection and categorization
- **util_error.c**: Comprehensive error handling and logging system
- **util_numeric.c**: Numerical stability functions and safe math operations
- **util_parameters.c**: Parameter processing and validation
- **util_integration.c**: Numerical integration routines

### Data Structures
- **types.h**: Core data structures including `struct GALAXY`, `struct SageConfig`, and `struct GALAXY_OUTPUT`
- **globals.h**: Global variable declarations and simulation state
- **constants.h**: Physical constants and numerical parameters
- **config.h**: Compile-time configuration options

### Key Design Patterns
1. **Modular Physics**: Each physical process is isolated in its own module with clear interfaces
2. **Memory Categories**: Memory allocation is tracked by category (galaxies, trees, parameters, etc.)
3. **Error Propagation**: Consistent error handling with context preservation throughout the call stack
4. **Format Abstraction**: I/O operations abstracted to support multiple tree and output formats
5. **State Management**: Simulation state cleanly separated from galaxy data

### Plotting System (output/sage-plot/)
- **sage-plot.py**: Central plotting script with command-line interface
- **figures/**: Modular plot implementations (19 different plot types)
- **snapshot_redshift_mapper.py**: Handles snapshot-redshift conversions
- Each plot module follows consistent interface: `plot(galaxies, volume, metadata, params, output_dir, output_format)`

### Parameter File Structure
Parameter files use a key-value format with sections for:
- File information (FirstFile, LastFile, OutputDir)
- Simulation parameters (BoxSize, Hubble_h, Omega)
- Recipe flags (SFprescription, AGNrecipeOn, ReionizationOn)
- Physical parameters (SfrEfficiency, FeedbackReheatingEpsilon)

### Tree Processing Flow
1. **load_tree_table()**: Load tree metadata and structure
2. **construct_galaxies()**: Build initial galaxy populations from halos
3. **evolve_galaxies()**: Apply physics models across cosmic time
4. **save_galaxies()**: Write results to output files
5. **free_galaxies_and_tree()**: Clean up memory

### Memory Management
Uses custom allocator with categorized tracking:
- MEMORY_GALAXY: Galaxy data structures
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