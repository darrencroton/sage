<!-- Purpose: Snapshot of current codebase architecture -->
<!-- Update Rules:
- 1000-word limit!
- Overwrite outdated content
- Focus on active components
- Use UML-like text diagrams
-->

# SAGE Current Architecture

## Directory Structure
```
sage/
├── src/                      # Modern modular structure (NEW - Task 1.2 Complete)
│   ├── core/                # Core infrastructure and coordination
│   │   ├── auxdata/         # Auxiliary data (CoolFunctions moved from extra/)
│   │   ├── main.c           # Program entry point
│   │   ├── initialization.c, parameters.c, evolution.c # Core infrastructure components
│   │   ├── globals.h        # Global declarations
│   │   ├── types.h          # Central data structures
│   │   └── config.h         # Compile-time configuration
│   ├── physics/             # Physical process implementations
│   │   └── cooling_heating.c, mergers.c, etc. # Physics model files
│   ├── io/                  # Input/output operations
│   │   ├── tree.c, save_binary.c, etc. # I/O handlers
│   │   └── format support   # Binary, HDF5 formats
│   ├── utils/               # Utility functions and system management
│   │   └── memory.c, error.c, numeric.c # Utility functions
│   └── scripts/             # Build and utility scripts
│       ├── beautify.sh      # Code formatting
│       └── first_run.sh     # Environment setup
├── input/                   # Input parameter files
├── output/                  # Output files and plotting system
├── docs/                    # Documentation
├── tests/                   # Testing framework (NEW)
└── log/                     # Project tracking logs
```

## Current Core Infrastructure

### Main Execution Flow
The current SAGE uses a traditional monolithic architecture with tightly coupled physics:

```
┌─────────────────────┐
│ main.c              │
└─────────┬───────────┘
          │
          ├─▶┌─────────────────┐
          │  │ initialization.c│
          │  │ - Memory setup  │
          │  └─────────────────┘
          │
          ├─▶┌─────────────────┐
          │  │ Parameter file  │
          │  │ parsing         │
          │  └─────────────────┘
          │
          ├─▶┌─────────────────┐
          │  │ Forest loop     │
          │  │ - load_tree     │
          │  │ - evolve        │
          │  │ - save_galaxies │
          │  └─────────────────┘
          │
          └─▶┌─────────────────┐
             │ Cleanup         │
             └─────────────────┘
```

### Galaxy Evolution Pipeline (CURRENT VIOLATION)
The core evolution pipeline has direct physics coupling - **violates Principle 1**:

```
┌─────────────────────────────┐
│ evolution.c                 │◄─ ARCHITECTURAL VIOLATION
│ - Direct physics includes   │
│ - Hardcoded physics calls   │
│ - Cannot run physics-free   │
└─────────┬───────────────────┘
          │
          ├─▶┌─────────────────┐
          │  │ cooling_        │
          │  │ heating.c       │
          │  └─────────────────┘
          │
          ├─▶┌─────────────────┐
          │  │ starformation_  │
          │  │ feedback.c      │
          │  └─────────────────┘
          │
          ├─▶┌─────────────────┐
          │  │ mergers.c       │
          │  └─────────────────┘
          │
          └─▶┌─────────────────┐
             │ Additional      │
             │ models...       │
             └─────────────────┘
```

### Memory Management System ✅ CENTRALIZED
Centralized memory management system through `memory.c`:
- **All allocations centralized**: mycalloc*, mymalloc*, myrealloc*, myfree
- **Category tracking**: MEM_GALAXIES, MEM_HALOS, MEM_TREES, MEM_IO, MEM_UTILITY, MEM_UNKNOWN
- **Leak detection and reporting**: Built-in check_memory_leaks()
- **High-water mark monitoring**: Peak memory usage tracking
- **Centralized header**: src/core/memory.h provides unified interface
- **Address Sanitizer validated**: No memory corruption detected

```
┌─────────────────────┐      ┌─────────────────────┐
│ memory.c            │      │ Memory Categories   │
│ - mycalloc/mymalloc │─────▶│ - MEM_GALAXIES      │
│ - myrealloc/myfree  │      │ - MEM_HALOS         │
│ - Category tracking │      │ - MEM_TREES         │
│ - Leak detection    │      │ - MEM_IO            │
│ - src/core/memory.h │      │ - MEM_UTILITY       │
└─────────────────────┘      └─────────────────────┘
```

### Configuration System ✅ MODERNIZED
Modern YAML-based configuration system (Task 1.4 Complete):
- **YAML format**: Structured, human-readable configuration with nested sections
- **Array support**: Output snapshots parsed as arrays with validation
- **Parameter validation**: Enhanced validation including num_outputs ≤ last_snapshot+1
- **No backward compatibility**: Legacy .par format completely removed
- **Comprehensive documentation**: Complete YAML configuration guide provided

### Galaxy Data Structure (CURRENT STATE)
Direct struct member access throughout codebase:

```c
struct GALAXY {
    // Halo properties
    float Pos[3], Vel[3];
    float Mvir, Rvir;
    int   SnapNum, Type;

    // Physics properties (DIRECT ACCESS - VIOLATION)
    float ColdGas, HotGas;
    float StellarMass, BulgeMass;
    float BlackHoleMass;
    // ... 50+ more physics properties

    // Structure properties
    int FirstProgGal;
    int NextGal, FirstGal;
};
```

**Architectural Issues:**
- Direct member access (`g->StellarMass`) throughout codebase
- Core and physics properties mixed
- No type safety or validation
- Fixed memory allocation regardless of physics needs

### I/O System
Current I/O supports multiple formats but lacks abstraction:

```
┌─────────────────────┐      ┌─────────────────────┐
│ Tree Input          │      │ Galaxy Output       │
│ - tree_binary.c     │      │ - save_binary.c     │
│ - tree_hdf5.c       │      │ - save_hdf5.c       │
└─────────────────────┘      └─────────────────────┘
```

**Current Limitations:**
- Format-specific implementations
- No property-based output
- Fixed output schemas
- **Needs unified interface**

## Build System
Modern CMake-based build:
- Out-of-tree compilation with clean source/build separation
- Automatic dependency detection (HDF5/MPI)
- Cross-platform compatibility and modern standards
- Git version tracking integrated
- **Ready for modular compilation support**

## Current Development Status

**Current Architecture State:**
1. ✅ **Modern Directory Structure**: Completed Task 1.2 - organized into logical src/ subdirectories
2. ✅ **Centralized Memory Management**: Completed Task 1.3 - all allocations through util_memory.c system
3. **Direct Data Access**: Galaxy properties accessed via direct struct members
4. **Hardcoded Physics**: Core cannot run without physics modules
5. **Limited Modularity**: No runtime configuration capability
6. ✅ **Modern Build**: CMake-based with dependency detection and out-of-tree builds

**Phase 1 Progress (Infrastructure Foundation):**
- ✅ **Task 1.1**: CMake build system operational
- ✅ **Task 1.2**: Directory reorganization complete - src/core, src/physics, src/io, src/utils structure
- ✅ **Task 1.3**: Memory management centralization complete - mycalloc/mymalloc/myrealloc/myfree
- ✅ **Task 1.4**: Configuration abstraction layer complete - YAML system with validation and arrays
- **Tasks 1.5-1.7**: I/O abstraction, Development infrastructure, Testing framework

**Immediate Violations to Address in Phase 2A:**
- `src/core/evolution.c` includes all physics headers directly
- Core makes direct physics function calls
- No physics-agnostic mode possible
- Fixed galaxy structure regardless of physics needs

## Required Architectural Transformation

**Phase 1 Goals** (Infrastructure Foundation):
- ✅ CMake build system replacing Makefile
- ✅ Directory reorganization preparing for modularization
- ✅ Memory abstraction layer (centralized allocation system)
- ✅ Configuration abstraction layer (YAML system)
- Abstraction layers for I/O and testing framework

**Phase 2A Goals** (Core/Physics Separation - CRITICAL):
- Remove all physics knowledge from core
- Create physics module interface
- Enable physics-free mode operation
- Wrap legacy physics in new interface

**Phase 2B Goals** (Property System):
- Replace direct struct access with type-safe properties
- Metadata-driven property definitions
- Separate core and physics properties

**Target Architecture Benefits:**
1. **Physics-Agnostic Core**: Core runs independently of physics
2. **Runtime Modularity**: Configure physics without recompilation
3. **Type Safety**: Validated property access throughout
4. **Memory Efficiency**: Only load needed physics properties
5. **Scientific Preservation**: Identical results for identical configurations

## Critical Success Factors

1. **Architectural Compliance**: No shortcuts that violate principles
2. **Scientific Accuracy**: Preserve all existing physics exactly
3. **Performance**: Modular architecture without performance cost
4. **Development Quality**: Professional standards throughout transformation

The current codebase represents a mature, scientifically validated galaxy evolution model that requires careful architectural modernization to achieve modularity while preserving its scientific integrity and performance characteristics.