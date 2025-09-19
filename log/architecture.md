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
├── code/                     # Monolithic legacy structure
│   ├── core_*.c/.h          # Core infrastructure components
│   ├── model_*.c/.h         # Physics implementations
│   ├── io_*.c/.h            # Input/output handlers
│   ├── util_*.c/.h          # Utility functions
│   └── types.h              # Central data structures
├── input/                   # Input parameter files
├── output/                  # Output files and plotting system
├── docs/                    # Documentation
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
          │  │ core_init.c     │
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
│ core_build_model.c          │◄─ ARCHITECTURAL VIOLATION
│ - Direct physics includes   │
│ - Hardcoded physics calls   │
│ - Cannot run physics-free   │
└─────────┬───────────────────┘
          │
          ├─▶┌─────────────────┐
          │  │ model_cooling_  │
          │  │ heating.c       │
          │  └─────────────────┘
          │
          ├─▶┌─────────────────┐
          │  │ model_star      │
          │  │ formation.c     │
          │  └─────────────────┘
          │
          ├─▶┌─────────────────┐
          │  │ model_mergers.c │
          │  └─────────────────┘
          │
          └─▶┌─────────────────┐
             │ Additional      │
             │ models...       │
             └─────────────────┘
```

### Memory Management System
Existing robust memory management through `util_memory.c`:
- Categorized memory tracking (MEMORY_GALAXY, MEMORY_TREE, etc.)
- Leak detection and reporting
- High-water mark monitoring
- **Ready for module-aware enhancement**

```
┌─────────────────────┐      ┌─────────────────────┐
│ util_memory.c       │      │ Memory Categories   │
│ - mymalloc/myfree   │─────▶│ - MEMORY_GALAXY     │
│ - Category tracking │      │ - MEMORY_TREE       │
│ - Leak detection    │      │ - MEMORY_PARAMETER  │
└─────────────────────┘      └─────────────────────┘
```

### Configuration System
Legacy parameter file system (`.par` format):
- Key-value pairs for physics parameters
- Hardcoded parameter names
- **Needs abstraction layer for JSON support**

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
│ - io_tree_binary.c  │      │ - io_save_binary.c  │
│ - io_tree_hdf5.c    │      │ - io_save_hdf5.c    │
└─────────────────────┘      └─────────────────────┘
```

**Current Limitations:**
- Format-specific implementations
- No property-based output
- Fixed output schemas
- **Needs unified interface**

## Build System
Traditional Makefile-based build:
- Monolithic compilation
- Optional HDF5/MPI support
- No modular compilation support
- **Ready for CMake modernization**

## Current Development Status

**Legacy Architecture State:**
1. **Monolithic Structure**: All physics directly coupled to core
2. **Direct Data Access**: Galaxy properties accessed via direct struct members
3. **Hardcoded Physics**: Core cannot run without physics modules
4. **Limited Modularity**: No runtime configuration capability
5. **Legacy Build**: Makefile-based with limited flexibility

**Immediate Violations to Address:**
- `core_build_model.c` includes all physics headers directly
- Core makes direct physics function calls
- No physics-agnostic mode possible
- Fixed galaxy structure regardless of physics needs

## Required Architectural Transformation

**Phase 1 Goals** (Infrastructure Foundation):
- CMake build system replacing Makefile
- Directory reorganization preparing for modularization
- Abstraction layers for Memory, Configuration, I/O

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