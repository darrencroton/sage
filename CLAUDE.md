# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Quick Setup

For new repository clones, use the automated setup script:

```bash
# Complete setup from fresh clone
./src/scripts/first_run.sh

# This creates directories, downloads data, sets up Python environment
# Creates sage_venv/ virtual environment with plotting dependencies
```

## Build Commands

```bash
# Out-of-tree build (recommended)
mkdir build
cd build
cmake .. && make -j$(nproc)
cd ..  # Always return to source directory

# Or single command alternative
mkdir build && cd build && cmake --build . --parallel $(nproc) && cd ..

# With optional dependencies
cd build
cmake .. -DUSE_HDF5=ON -DUSE_MPI=ON
make -j$(nproc)
cd ..  # Return to source directory

# Clean and rebuild
cd build && make clean && make -j$(nproc) && cd ..

# Different build types
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release  # Optimized build
cmake .. -DCMAKE_BUILD_TYPE=Debug    # Debug build (default)
make -j$(nproc) && cd ..
```

## Code Formatting

```bash
# Format all C and Python code
./src/scripts/beautify.sh

# Format only C code (requires clang-format)
./src/scripts/beautify.sh --c-only

# Format only Python code (requires black and isort)
./src/scripts/beautify.sh --py-only
```

## Running SAGE

**IMPORTANT**: SAGE must always be run from the source directory where parameter files expect relative paths.

```bash
# SAGE executable is automatically moved to source directory after CMake build
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

# Generate all plots (both snapshot and evolution - new default behavior)
python sage-plot.py --param-file=../../input/millennium.par

# Generate specific plots
python sage-plot.py --param-file=../../input/millennium.par --plots=stellar_mass_function,sfr_density_evolution

# Generate only snapshot plots
python sage-plot.py --param-file=../../input/millennium.par --snapshot-plots

# Generate only evolution plots
python sage-plot.py --param-file=../../input/millennium.par --evolution-plots

# Cross-directory execution now works from anywhere
cd ../..
python output/sage-plot/sage-plot.py --param-file=input/millennium.par --plots=stellar_mass_function

# Deactivate when done
deactivate
```

## Code Architecture

SAGE follows a modern, modular directory structure that organizes source code by functionality:

```
src/
├── core/           # Core infrastructure and coordination
├── physics/        # Physical process implementations
├── io/            # Input/output operations
├── utils/         # Utility functions and system management
└── scripts/       # Build and utility scripts
```

### Core Infrastructure (src/core/)
- **main.c**: Program entry point, handles initialization, file processing loop, and cleanup
- **initialization.c**: System initialization, memory setup, parameter validation
- **parameters.c**: Parameter file parsing and configuration setup
- **evolution.c**: Galaxy construction and evolution coordination
- **auxdata/CoolFunctions/**: Cooling function lookup tables

### Physical Models (src/physics/)
- **cooling_heating.c**: Gas cooling and heating processes
- **starformation_feedback.c**: Star formation and supernova feedback
- **mergers.c**: Galaxy merger handling and black hole growth
- **infall.c**: Gas infall calculations
- **reincorporation.c**: Gas reincorporation from hot to cold phase
- **disk_instability.c**: Disk instability and bulge formation
- **misc.c**: Miscellaneous processes (stripping, disruption)

### I/O System (src/io/)
- **tree.c**: Master tree loading interface
- **tree_binary.c**: Binary format tree reader (LHalo format)
- **tree_hdf5.c**: HDF5 format tree reader (Genesis format)
- **save_binary.c**: Binary output format writer
- **save_hdf5.c**: HDF5 output format writer

### Utilities (src/utils/)
- **memory.c**: Custom memory management with leak detection and categorization
- **error.c**: Comprehensive error handling and logging system
- **numeric.c**: Numerical stability functions and safe math operations
- **parameters.c**: Parameter processing and validation
- **integration.c**: Numerical integration routines

### Data Structures (src/core/)
- **types.h**: Core data structures including `struct GALAXY`, `struct SageConfig`, and `struct GALAXY_OUTPUT`
- **globals.h**: Global variable declarations and simulation state
- **constants.h**: Physical constants and numerical parameters
- **config.h**: Compile-time configuration options

### Scripts (src/scripts/)
- **beautify.sh**: Code formatting script (clang-format + black)
- **first_run.sh**: Automated setup script for new repository clones

### Key Design Patterns
1. **Modular Physics**: Each physical process is isolated in its own module with clear interfaces
2. **Memory Categories**: Memory allocation is tracked by category (galaxies, trees, parameters, etc.)
3. **Error Propagation**: Consistent error handling with context preservation throughout the call stack
4. **Format Abstraction**: I/O operations abstracted to support multiple tree and output formats
5. **State Management**: Simulation state cleanly separated from galaxy data

### Plotting System (output/sage-plot/)
- **sage-plot.py**: Central plotting script with enhanced command-line interface
- **figures/**: Modular plot implementations (19 different plot types)
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
SAGE uses a centralized memory management system with comprehensive tracking:
- **Centralized Allocation**: All memory goes through `mymalloc*`, `mycalloc*`, `myrealloc*`, `myfree`
- **Category Tracking**: MEM_GALAXIES, MEM_HALOS, MEM_TREES, MEM_IO, MEM_UTILITY, MEM_UNKNOWN
- **Leak Detection**: Built-in leak detection with `check_memory_leaks()`
- **High-Water Marking**: Peak memory usage tracking and reporting
- **Memory Validation**: Optional guard bytes for corruption detection (DEBUG_MEMORY)
- **Centralized Header**: Include `src/core/memory.h` for unified memory interface
Call `print_allocated()` to check current usage, `check_memory_leaks()` for leak detection.

### Documentation Standards
Follow the documentation template in `docs/doc_standards.md`:
- Function headers with @brief, @param, @return
- File headers explaining purpose and key functions
- Inline comments for complex calculations
- Units explicitly stated for physical quantities

## Development Guidelines
- All work to highest professional coding standards
- Debug with lldb using a command file (must end with "quit"): `lldb --batch -s debug_commands.txt ./sage`
- Never simplify tests - failing tests indicate real problems
- When running sage always check the exit code for success or failure
- Use logs for continuity - assume no persistent memory
- Report progress in `log/progress.md` with all changed files
- Documentation-as-you-go always
- Ask before committing to git
- Never delete! Archive to `scrap/` instead