# SAGE Directory Structure

This document explains the reorganized directory structure of the SAGE semi-analytic galaxy evolution model, implemented as part of Task 1.2: Directory Reorganization (Phase 1).

## Overview

The monolithic `code/` directory has been reorganized into a logical, modular structure that prepares SAGE for the core/physics separation planned in Phase 2A. This reorganization maintains full scientific compatibility while improving code organization and maintainability.

## Directory Layout

```
sage/
├── src/                        # Source code organized by functionality
│   ├── core/                   # Core infrastructure and galaxy evolution coordination
│   │   ├── auxdata/            # Auxiliary data files
│   │   │   └── CoolFunctions/  # Cooling function lookup tables (moved from extra/)
│   │   ├── main.c              # Program entry point and execution loop
│   │   ├── core_*.c/.h         # Core initialization, parameter reading, galaxy construction
│   │   ├── globals.h           # Global variable declarations
│   │   ├── types.h             # Core data structures (GALAXY, SageConfig, etc.)
│   │   ├── constants.h         # Physical constants and numerical parameters
│   │   ├── config.h            # Compile-time configuration options
│   │   └── git_version.h.in    # Git version tracking template
│   ├── physics/                # Physical process implementations
│   │   ├── model_cooling_heating.c         # Gas cooling and heating processes
│   │   ├── model_starformation_and_feedback.c  # Star formation and supernova feedback
│   │   ├── model_mergers.c                 # Galaxy merger handling and black hole growth
│   │   ├── model_infall.c                  # Gas infall calculations
│   │   ├── model_reincorporation.c         # Gas reincorporation from hot to cold phase
│   │   ├── model_disk_instability.c        # Disk instability and bulge formation
│   │   └── model_misc.c                    # Miscellaneous processes (stripping, disruption)
│   ├── io/                     # Input/output operations
│   │   ├── io_tree.c           # Master tree loading interface
│   │   ├── io_tree_binary.c    # Binary format tree reader (LHalo format)
│   │   ├── io_tree_hdf5.c      # HDF5 format tree reader (Genesis format)
│   │   ├── io_save_binary.c    # Binary output format writer
│   │   ├── io_save_hdf5.c      # HDF5 output format writer
│   │   ├── io_util.c           # I/O utility functions and error handling
│   │   └── *.h                 # Corresponding header files
│   ├── utils/                  # Utility functions and system management
│   │   ├── util_memory.c       # Memory management with leak detection and categorization
│   │   ├── util_error.c        # Comprehensive error handling and logging system
│   │   ├── util_numeric.c      # Numerical stability functions and safe math operations
│   │   ├── util_parameters.c   # Parameter processing and validation
│   │   ├── util_integration.c  # Numerical integration routines
│   │   ├── util_version.c      # Version tracking and build information
│   │   └── *.h                 # Corresponding header files
│   └── scripts/                # Build and utility scripts
│       ├── beautify.sh         # Code formatting script (clang-format + black)
│       └── first_run.sh        # Automated setup script for new repository clones
├── docs/                       # Documentation
│   ├── directory-structure.md  # This file
│   └── quick-reference.md      # Central documentation index
├── tests/                      # Testing framework (future development)
├── input/                      # Parameter files and simulation inputs
├── output/                     # Output files and plotting system
├── build/                      # Out-of-tree build directory (CMake)
└── [other files]               # README.md, CMakeLists.txt, etc.
```

## Rationale

### Core Infrastructure (`src/core/`)
Contains the fundamental SAGE infrastructure that coordinates galaxy evolution:
- **Program entry and flow control**: `main.c` handles initialization, file processing loops, and cleanup
- **Core evolution logic**: `core_build_model.c` coordinates galaxy construction and evolution
- **System initialization**: `core_init.c` sets up memory, validates parameters
- **Configuration management**: `core_read_parameter_file.c` handles parameter file parsing
- **Data structures**: `types.h` defines core structures like `GALAXY` and `SageConfig`
- **Auxiliary data**: `auxdata/CoolFunctions/` contains cooling function lookup tables

### Physics Models (`src/physics/`)
Isolates all physical process implementations in dedicated modules:
- Each physics process is self-contained in its own file
- Clear interfaces between physics modules and core infrastructure
- Prepares for Phase 2A core/physics separation where these will become runtime-configurable modules

### I/O Operations (`src/io/`)
Centralizes all input/output functionality:
- **Tree input**: Support for multiple formats (binary LHalo, HDF5 Genesis)
- **Galaxy output**: Flexible output writers for different formats
- **Format abstraction**: Prepares for unified I/O interface in Phase 5

### Utilities (`src/utils/`)
Provides essential system services:
- **Memory management**: Categorized tracking, leak detection, high-water mark monitoring
- **Error handling**: Consistent error propagation with context preservation
- **Numerical stability**: Safe math operations and integration routines
- **Parameter validation**: Robust configuration processing

### Scripts (`src/scripts/`)
Build and development tools:
- **Code formatting**: Automated C and Python code formatting
- **Environment setup**: One-command repository initialization

## Benefits

### 1. **Logical Organization**
- Related functionality grouped together
- Clear separation of concerns
- Easier navigation for developers

### 2. **Modular Preparation**
- Physics models isolated for future runtime modularity
- Core infrastructure separated from implementations
- Foundation for Phase 2A core/physics separation

### 3. **Build System Modernization**
- CMake configuration updated for new structure
- Multiple include directories for clean compilation
- Maintains exact same compilation order and scientific results

### 4. **Maintainability**
- Clear file organization reduces cognitive load
- Easier to locate specific functionality
- Better separation of responsibilities

## Migration Notes

### Include Path Updates
All source files have been updated to use relative include paths:
```c
// Physics files include core and utils
#include "../core/core_allvars.h"
#include "../core/core_proto.h"
#include "../utils/util_numeric.h"

// I/O files include core and utils
#include "../core/core_proto.h"
#include "../utils/util_error.h"
```

### CMake Configuration
- All source and header file paths updated to use `src/` prefix
- Include directories configured: `src/core src/io src/physics src/utils`
- Git version tracking updated for new location: `src/core/git_version.h.in`
- HDF5 conditional sources updated for new paths

### Path Dependencies
- Cooling function path updated: `src/core/auxdata/CoolFunctions/`
- All relative paths maintain correct relationships
- Build artifacts isolated in `build/` directory

## Validation

### Build System
- ✅ CMake configuration successful
- ✅ Clean compilation with no new warnings
- ✅ All source files compile in correct order
- ✅ Executable generated and moved to source directory

### Scientific Accuracy
- ✅ SAGE runs successfully with exit code 0
- ✅ All output files generated at multiple redshifts
- ✅ No memory leaks detected
- ✅ Memory usage consistent with baseline

### Architecture Compliance
- ✅ File moves preserved git history (`git mv`)
- ✅ No hardcoded paths broken
- ✅ Include dependencies correctly resolved
- ✅ Auxiliary data accessible from new location

## Future Development

This directory structure prepares SAGE for upcoming architectural transformations:

- **Phase 2A**: Core/physics separation will build on the `src/core/` and `src/physics/` distinction
- **Phase 2B**: Property system will leverage the modular organization
- **Phase 3**: Memory management will use the `src/utils/` utilities infrastructure
- **Phase 4**: Module system will organize around the established directory boundaries
- **Phase 5**: I/O modernization will build on the `src/io/` foundation

The reorganization establishes a clean foundation for all subsequent modular development while maintaining full scientific compatibility.