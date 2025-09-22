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
├── src/                     # Modern modular structure
│   ├── core/                # Core infrastructure and coordination
│   │   ├── auxdata/         # Auxiliary data
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
├── tests/                   # Testing framework
├── .github/workflows/       # CI/CD pipeline
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
- **YAML format**: Structured configuration with nested sections and arrays
- **Enhanced validation**: Comprehensive parameter checking
- **No legacy support**: .par format completely removed

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

### I/O System ✅ ABSTRACTION LAYER COMPLETE
Modern I/O abstraction layer with unified interface:

```
┌─────────────────────────────────────────────────┐
│ io_manager.h - Format-Agnostic Interface       │
│ ┌─────────────────────────────────────────────┐ │
│ │ io_manager_t struct:                        │ │
│ │ - load_tree_table()   - save_galaxies()    │ │
│ │ - load_tree()         - finalize_output()  │ │
│ │ - context* (future module awareness)       │ │
│ └─────────────────────────────────────────────┘ │
└─────────────────────┬───────────────────────────┘
                      │
        ┌─────────────┴─────────────┐
        │                           │
┌───────▼──────────┐      ┌─────────▼─────────┐
│ Binary Format    │      │ HDF5 Format       │
│ - tree_binary.c  │      │ - tree_hdf5.c     │
│ - save_binary.c  │      │ - save_hdf5.c     │
└──────────────────┘      └───────────────────┘
```

**✅ Achievements:**
- ✅ Unified function pointer interface
- ✅ Runtime format selection
- ✅ Foundation for module-aware output (Phase 5)
- ✅ Enhanced error handling and logging
- ✅ **Ready for Phase 5 hierarchical HDF5 output**

## Build System
Modern CMake-based build with integrated testing:
- Out-of-tree compilation with dependency detection (HDF5/MPI)
- **CTest testing framework** (Task 1.7 Complete)
- **GitHub Actions CI** - Multi-platform automated testing
- **Ready for modular compilation support**

## Current Development Status

**✅ Phase 1 Architecture State - FOUNDATION COMPLETE:**
1. ✅ **Modern Directory Structure**: Completed Task 1.2 - organized into logical src/ subdirectories
2. ✅ **Centralized Memory Management**: Completed Task 1.3 - all allocations through util_memory.c system
3. ✅ **Testing Framework**: Completed Task 1.7 - CTest framework with professional test utilities and CI/CD
4. ✅ **I/O Abstraction Layer**: Completed Task 1.5 - format-agnostic I/O through io_manager_t interface
5. **Direct Data Access**: Galaxy properties accessed via direct struct members (Phase 2B target)
6. **Hardcoded Physics**: Core cannot run without physics modules (Phase 2A target)
7. **Limited Modularity**: No runtime configuration capability (Phase 4 target)
8. ✅ **Modern Build**: CMake-based with dependency detection and out-of-tree builds

**🎉 Phase 1 Infrastructure Foundation - COMPLETE:**
- ✅ **Task 1.1**: CMake build system operational
- ✅ **Task 1.2**: Directory reorganization complete
- ✅ **Task 1.3**: Memory management centralization complete
- ✅ **Task 1.4**: Configuration abstraction layer complete (YAML)
- ✅ **Task 1.5**: I/O abstraction layer complete (format-agnostic interface)
- ✅ **Task 1.6**: Development infrastructure complete (comprehensive documentation system)
- ✅ **Task 1.7**: Testing and automation framework complete (CTest + CI)

**Immediate Violations to Address in Phase 2A:**
- `src/core/evolution.c` includes all physics headers directly
- Core makes direct physics function calls
- No physics-agnostic mode possible
- Fixed galaxy structure regardless of physics needs

## 🚀 Ready for Phase 2A: Core/Physics Separation

**Phase 1 Foundation Complete - Ready for Next Phase:**

With all infrastructure abstraction layers now in place, SAGE is ready for the critical Phase 2A transformation that will establish the physics-agnostic core architecture.

**Next: Phase 2A** (Core/Physics Separation - CRITICAL):
- Remove all physics knowledge from core
- Create physics module interface
- Enable physics-free mode operation
- Wrap legacy physics in new interface

**✅ Infrastructure Foundation Achieved:**
1. **Modern Build System**: CMake with testing and CI/CD
2. **Organized Codebase**: Logical directory structure for modularization
3. **Memory Abstraction**: Centralized, categorized memory management
4. **Configuration Abstraction**: Modern YAML-based configuration system
5. **I/O Abstraction**: Format-agnostic interface ready for module-aware output

**🎯 Target Architecture Benefits (Phase 2+ Goals):**
1. **Physics-Agnostic Core**: Core runs independently of physics
2. **Runtime Modularity**: Configure physics without recompilation
3. **Type Safety**: Validated property access throughout
4. **Memory Efficiency**: Only load needed physics properties

The current codebase represents a mature, scientifically validated galaxy evolution model with a **solid infrastructure foundation** ready for careful architectural modernization to achieve modularity while preserving scientific integrity.