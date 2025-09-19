# SAGE Master Implementation Plan v2.1
**Version**: 2.1 (Architecturally-Aligned for sage)  
**Date**: September 2024  
**Legacy Baseline**: `sage` codebase

---

## ⚠️ CRITICAL ARCHITECTURAL COMPLIANCE

This implementation plan has been restructured to strictly adhere to the **8 Core Architectural Principles** defined in `logs/sage-architecture-vision.md`. Each phase explicitly states which principles it addresses and ensures no violations occur.

### 🎯 Core Architectural Principles
1. **Physics-Agnostic Core Infrastructure** - Core has zero physics knowledge
2. **Runtime Modularity** - Module combinations configurable without recompilation  
3. **Metadata-Driven Architecture** - Structure defined by metadata, not hardcoded
4. **Single Source of Truth** - One authoritative data representation
5. **Unified Processing Model** - Consistent merger tree processing
6. **Memory Efficiency and Safety** - Bounded, predictable memory usage
7. **Format-Agnostic I/O** - Multiple formats through unified interfaces
8. **Type Safety and Validation** - Type-safe access with automatic validation

---

## Executive Summary

This Master Implementation Plan guides the transformation of SAGE from a monolithic galaxy evolution model into a modern, modular architecture with a **physics-agnostic core and runtime-configurable physics modules**, while strictly maintaining architectural principle compliance throughout development.

### Key Transformation Goals
1. **Physics-Agnostic Core** (Principle 1): Core infrastructure with zero physics knowledge
2. **Type-Safe Property System** (Principles 3,4,8): Metadata-driven, compile-time validated access
3. **Runtime Module System** (Principle 2): Physics modules loaded/configured at runtime
4. **Unified I/O Architecture** (Principle 7): Format-agnostic with property-based output

### Implementation Approach
- **Architecture-First**: Establish foundational principles before building complexity
- **Scientific Preservation**: Every phase validates against reference results
- **Principle Compliance**: No architectural violations tolerated during development
- **Incremental Value**: Each phase delivers working, compliant functionality

---

## Phase Overview

| Phase | Name | Principles | Duration | Entry Criteria | Exit Criteria |
|-------|------|------------|----------|----------------|---------------|
| 1 | Infrastructure & Modernization Foundation | 6,7 | 3 weeks | `sage` codebase | CMake build, abstraction layers |
| 2A | Core/Physics Separation | 1,5 | 3 weeks | Phase 1 complete | Physics-agnostic core operational |
| 2B | Property System Integration | 3,4,8 | 4 weeks | Phase 2A complete | Properties applied in modular context |
| 3 | Memory Management | 6 | 3 weeks | Phase 2B complete | Module-aware, bounded memory |
| 4 | Configuration & Module System | 2 | 4 weeks | Phase 3 complete | Runtime module configuration |
| 5 | I/O Modernization | 7 | 3 weeks | Phase 4 complete | Property-based, module-aware I/O |
| 6 | Validation & Polish | All | 2 weeks | Phase 5 complete | Full validation, documentation |

**Total Duration**: ~20 weeks (5 months)

---

## Phase 1: Infrastructure and Modernization Foundation

### Architectural Principles Addressed
- This phase prepares the ground for all 8 principles by establishing a modern build environment and creating crucial abstraction layers. It directly contributes to **Principle 6 (Memory Safety)** and **Principle 7 (Format-Agnostic I/O)** by creating the necessary interfaces.

### Objectives
- **Primary**: Replace the legacy Makefile with a modern CMake build system.
- **Secondary**: Reorganize the codebase into a logical directory structure that anticipates the core/physics split.
- **Critical**: Create abstraction layers for Memory, Configuration, and I/O to decouple core logic from specific implementations, enabling a smooth, incremental refactoring in later phases.

### Entry Criteria
- `sage` codebase selected as the starting point.
- Development environment (C compiler, Git, Python for scripting) is set up.

### Tasks

#### Task 1.1: CMake Build System Setup
- **Objective**: Replace the existing Makefile with a modern CMake configuration.
- **Implementation**:
  - Create a root `CMakeLists.txt` with project configuration (e.g., `project(SAGE)`).
  - Set up source file discovery and define compilation flags (e.g., `-Wall`, `-g`).
  - Configure detection for optional dependencies like HDF5 and MPI using `find_package`.
  - Enable out-of-tree builds to keep the source directory clean.
- **Testing**: The CMake build produces a binary that is scientifically identical to the one produced by the Makefile.
- **Documentation**: Update `README.md` with clear, step-by-step instructions for configuring and building the project with CMake.
- **Effort**: 2 sessions (moderate complexity)

#### Task 1.2: Directory Reorganization
- **Objective**: Implement a modern project structure to prepare for modularization.
- **Implementation**:
  - Create the `src/core`, `src/physics`, `src/io`, and `src/utils` directories.
  - Use `git mv` to move source files into the appropriate subdirectories.
    - **core**: `core_*.c`, `main.c`, `globals.h`, `types.h`, etc.
    - **physics**: `model_*.c` files.
    - **io**: `io_*.c` files.
    - **utils**: `util_*.c` files.
  - Create `docs/` for documentation and `tests/` for future tests.
- **Testing**: The project builds successfully with the new directory structure. All `#include` paths are updated and correct.
- **Documentation**: Create a `docs/directory-structure.md` file explaining the layout.
- **Effort**: 1 session (low complexity)

#### Task 1.3: Memory Management Centralization
- **Objective**: Centralize all memory allocations through the existing `util_memory.c` system to prepare for module-aware tracking.
- **Implementation**:
  - Create a new header `src/core/memory.h` that includes `util_memory.h`.
  - Perform a codebase-wide replacement of `malloc`, `calloc`, `realloc`, and `free` with the corresponding functions from `util_memory.c` (`mymalloc`, `myrealloc`, `myfree`). Ensure `mycalloc` is implemented if needed.
  - All memory allocations must go through this centralized system.
- **Testing**: The code compiles and runs with no new memory errors. Valgrind reports no leaks, and the memory tracking system functions as before.
- **Effort**: 1 session (low complexity)

#### Task 1.4: Configuration Abstraction Layer
- **Objective**: Create a unified interface for accessing configuration parameters, decoupling the code from the global `SageConfig` struct.
- **Implementation**:
  - Design a `config_manager.h` interface with functions like `config_get_int(param_name)`, `config_get_double(param_name)`, etc.
  - The initial implementation of this interface will simply read from the populated global `SageConfig` struct.
  - Replace direct access to `SageConfig` fields throughout the codebase with calls to this new interface.
- **Testing**: The simulation runs and produces identical results, confirming that all parameters are accessed correctly through the new abstraction layer.
- **Effort**: 2 sessions (moderate complexity)

#### Task 1.5: I/O Abstraction Layer
- **Objective**: Create an abstraction layer for tree input and galaxy output to prepare for a unified, format-agnostic I/O system.
- **Implementation**:
  - Create a new `io_manager.h` header.
  - Define a generic `io_manager_t` struct containing function pointers for key I/O operations (e.g., `load_tree_table`, `load_tree`, `save_galaxies`, `finalize_galaxy_files`).
  - In `main.c`, create an `io_manager_t` instance and initialize its function pointers to point to the existing functions in `io_tree.c` and `io_save_binary.c`/`io_save_hdf5.c`.
  - Replace direct calls to these I/O functions in the main loop with calls through the `io_manager_t` function pointers.
- **Testing**: The code compiles and runs, producing identical output files. Both binary and HDF5 I/O function correctly through the abstraction layer.
- **Effort**: 2 sessions (moderate complexity)

#### Task 1.6: Development and Documentation Infrastructure
- **Objective**: Establish the foundational documentation structure for the project.
- **Implementation**:
  - Create the `docs/` directory.
  - Create `docs/physics-module-guide.md` as a placeholder for the guide that will be written in Phase 2A.
  - Create `docs/user-guide.md` as a placeholder for the end-user guide that will be written in Phase 6.
  - Create a `docs/quick-reference.md` to serve as a central index for all documentation.
  - Create `docs/testing-guide.md` that describes the sage testing framework, including a unit test template.
- **Testing**: The directory structure is in place and files are correctly linked.
- **Effort**: 1 session (low complexity)

### Exit Criteria
- ✅ The project builds successfully using CMake.
- ✅ The codebase is organized into the new directory structure.
- ✅ Abstraction layers for Memory, Configuration, and I/O are in place and used throughout the code.
- ✅ The simulation produces scientifically identical results to the original `sage` baseline.
- ✅ The foundational documentation structure is established.

---

## 🚨 Phase 2A: Core/Physics Separation (NEW PHASE)

### Architectural Principles Addressed
- **Principle 1**: Physics-Agnostic Core Infrastructure ⭐ **PRIMARY**
- **Principle 5**: Unified Processing Model

### Objectives
- **Primary**: Remove all physics knowledge from core infrastructure
- **Secondary**: Enable physics-free mode operation
- **Critical**: Establish foundation for all subsequent development

### Entry Criteria
- ✅ CMake build system operational (from Phase 1)
- ✅ Core currently has hardcoded physics calls (violation to fix)

### Tasks

#### Task 2A.1: Physics Module Interface Design
- **Objective**: Create minimal interface for physics module communication
- **Implementation**:
  - Design `physics_module_t` structure with execution phases
  - Define standard module lifecycle (init, execute, cleanup)
  - Create module registration and lookup functions
  - Add module capability declarations
- **Principles**: Establishes foundation for Principle 1 compliance
- **Testing**: Interface compiles and supports basic module operations
- **Effort**: 2 sessions

#### Task 2A.2: Core Evolution Pipeline Abstraction
- **Objective**: Replace hardcoded physics calls with module interface
- **Implementation**:
  - Remove direct `#include` of physics headers from core
  - Replace physics function calls with module interface calls
  - **CRITICAL**: Use `CORE_PROP_*` naming for core property access (NOT `GALAXY_PROP_*`)
  - Create conditional execution based on loaded modules
  - Maintain identical execution order and logic
- **Principles**: Achieves Principle 1 compliance in core
- **Testing**: Core compiles without physics dependencies
- **Effort**: 3 sessions

#### Task 2A.3: Physics-Free Mode Implementation
- **Objective**: Enable core to run without any physics modules
- **Implementation**:
  - Core processes merger trees without physics calculations
  - Galaxies have core properties only (halo inheritance, tracking)
  - Tree traversal and basic galaxy lifecycle functional
  - Scientific output limited to halo properties
- **Principles**: Validates Principle 1 compliance
- **Testing**: Physics-free mode runs complete merger tree processing
- **Effort**: 2 sessions

#### Task 2A.4: Legacy Physics Module Wrapping
- **Objective**: Wrap existing physics modules in new interface + create shared physics utilities
- **Implementation**:
  - Create `src/physics/physics_essential_functions.c/h` for shared physics utilities
  - Move `get_disk_radius()`, `estimate_merging_time()` to essential functions
  - Create adapter modules for each physics function
  - Modules handle all physics property initialization and normalization
  - Maintain identical physics calculations
  - Register modules using new interface
  - Preserve all existing scientific behavior
- **Principles**: Maintains Principle 5 (unified processing)
- **Testing**: Physics calculations produce identical results
- **Effort**: 3 sessions (enhanced scope)

#### Task 2A.5: Module Dependency Framework
- **Objective**: Basic dependency resolution for module execution
- **Implementation**:
  - Modules declare their dependencies
  - Simple topological sort for execution order
  - Error handling for missing dependencies
  - Support for optional module loading
- **Principles**: Enables Principle 2 foundation
- **Testing**: Module dependencies resolved correctly
- **Effort**: 2 sessions

### Exit Criteria
- ✅ Core compiles and runs without physics modules loaded
- ✅ Physics modules wrapped in interface, calculations unchanged
- ✅ Module interface enables conditional physics execution  
- ✅ Physics-free mode processes merger trees successfully
- ✅ All existing tests pass with wrapped physics modules

### Validation
- **Architecture Compliance**: Core has zero physics knowledge
- **Scientific Accuracy**: Physics results identical to pre-modular
- **Functionality**: Both physics-free and full-physics modes work
- **Performance**: No significant runtime degradation

---

## Phase 2B: Property System Integration (REVISED)

### Architectural Principles Addressed
- **Principle 3**: Metadata-Driven Architecture ⭐ **PRIMARY**
- **Principle 4**: Single Source of Truth
- **Principle 8**: Type Safety and Validation

### Objectives
- **Primary**: Apply property system within modular architecture
- **Secondary**: Enable type-safe property access throughout codebase
- **Foundation**: Property migration occurs in architecturally compliant context

### Entry Criteria
- ✅ Core/physics separation complete (Phase 2A)
- ✅ Physics modules use interface, not direct coupling
- ✅ Property system infrastructure ready (Tasks 2.1-2.2 from original plan, now assumed complete)

### Tasks

#### Task 2B.1: Core Property Migration
- **Objective**: Migrate core properties using property system
- **Implementation**:
  - Apply property macros to core files only
  - Core properties: `SnapNum`, `Type`, `Pos`, `Mvir`, `Rvir`, etc.
  - Update core_build_model.c, core_save.c using property access
  - Maintain physics-agnostic property boundaries
- **Principles**: Principle 4 - unified core property access
- **Testing**: Core functionality identical with property macros
- **Effort**: 2 sessions

#### Task 2B.2: Physics Property Integration
- **Objective**: Migrate physics properties within modular context
- **Implementation**:
  - Physics modules use property macros for physics properties
  - Properties: `ColdGas`, `StellarMass`, `HotGas`, etc.
  - Property access only within appropriate modules
  - Core never accesses physics properties directly
- **Principles**: Maintains Principle 1 compliance during property migration
- **Testing**: Physics calculations unchanged with property access
- **Effort**: 3 sessions

#### Task 2B.3: Property Availability System
- **Objective**: Runtime property availability based on loaded modules
- **Implementation**:
  - Property system adapts to loaded module set
  - Core properties always available
  - Physics properties conditional on module loading
  - Type-safe access with compile-time checking
- **Principles**: Principle 2 - runtime modularity for properties
- **Testing**: Property access reflects loaded modules correctly
- **Effort**: 2 sessions

#### Task 2B.4: CMake Integration Enhancement
- **Objective**: Integrate property generation with modular build
- **Implementation**:
  - Property generation considers module configurations
  - Different property sets for different build modes
  - Automatic regeneration on YAML changes
  - IDE support for generated property access
- **Principles**: Principle 3 - build-time metadata processing
- **Testing**: Build system regenerates properties correctly
- **Effort**: 1 session

### Exit Criteria
- ✅ All galaxy properties accessed via type-safe macros
- ✅ Property availability adapts to loaded modules
- ✅ Core uses core properties, physics modules use physics properties
- ✅ Build system integrated with property generation
- ✅ Scientific results unchanged from Phase 2A

---

## Phase 3: Memory Management (ENHANCED)

### Architectural Principles Addressed
- **Principle 6**: Memory Efficiency and Safety ⭐ **PRIMARY**

### Objectives
- **Primary**: Adapt and enhance the existing `util_memory.c` system to be fully module-aware.
- **Secondary**: Ensure memory usage is bounded and predictable within the new modular architecture.
- **Foundation**: Leverage the robust memory tracking of `sage` to build a safe, modular memory environment.

### Entry Criteria
- ✅ Modular architecture established (Phase 2A complete)
- ✅ Property system operational within modules (Phase 2B complete)
- ✅ Module interface supports a memory lifecycle (init/cleanup phases).

### Tasks

#### Task 3.1: Integrate Memory System with Module Lifecycle
- **Objective**: Make the existing memory tracker in `util_memory.c` aware of module scopes.
- **Implementation**:
  - Introduce memory categories (`MemoryCategory` enum in `util_memory.h`) for each physics module.
  - Update `mymalloc_cat` and `myrealloc_cat` to be the primary allocation functions used by modules, ensuring all allocations are tagged by category.
  - Implement functions to report memory usage per module (`print_allocated_by_category`).
  - Ensure `check_memory_leaks` can report leaks on a per-module basis.
- **Principles**: Principle 6 with module awareness.
- **Testing**: Memory usage is correctly tracked and reported for each module.

#### Task 3.2: Property-Based Memory Allocation
- **Objective**: Memory allocation for galaxy properties is driven by the set of loaded modules.
- **Implementation**:
  - The core `GALAXY` struct will be split into a `core_galaxy_properties` struct and multiple `physics_galaxy_properties` structs (one per module).
  - The main galaxy object will contain pointers to these physics property structs.
  - Memory for a module's properties is only allocated if that module is loaded at runtime.
  - The memory system will track these allocations under the appropriate module category.
- **Principles**: Integrates Principles 3 (Metadata-Driven) and 6 (Memory Efficiency).
- **Testing**: `sizeof(galaxy)` is smaller when fewer modules are loaded. Memory usage adapts correctly to the runtime module configuration.

#### Task 3.3: Bounded Memory Architecture
- **Objective**: Ensure memory usage is scoped to a single forest and does not grow with the number of forests processed.
- **Implementation**:
  - Confirm that all memory for a given forest (halos, galaxies, module data) is allocated within the `load_tree` or `construct_galaxies` functions.
  - Ensure that `free_galaxies_and_tree` and module cleanup functions correctly free all memory associated with a forest.
  - Use the memory tracker's high-water mark (`HighMarkMem`) to verify that memory usage resets after each forest is processed.
- **Principles**: Principle 6 - bounded memory.
- **Testing**: The memory high-water mark remains stable over a long run processing many forests.

### Exit Criteria
- ✅ The `util_memory.c` system is fully integrated with the module lifecycle.
- ✅ Memory for galaxy properties is allocated dynamically based on loaded modules.
- ✅ Memory usage is confirmed to be bounded on a per-forest basis.
- ✅ Standard debugging tools (e.g., Valgrind) report no leaks for a full run.

---

## Phase 4: Configuration & Module System (ENHANCED)

### Architectural Principles Addressed
- **Principle 2**: Runtime Modularity ⭐ **PRIMARY**

### Objectives
- **Primary**: Runtime module configuration without recompilation
- **Secondary**: Complete unified configuration system
- **Foundation**: Full runtime modularity achieved

### Entry Criteria
- ✅ Module interface mature from Phase 2A
- ✅ Property system supports module-aware configurations
- ✅ Memory management supports dynamic module loading

### Tasks

#### Task 4.1: Runtime Module Configuration
- **Objective**: Load/configure modules based on configuration files
- **Implementation**:
  - JSON-based module selection
  - Module parameter configuration
  - Runtime module loading/unloading
  - Module combination validation
- **Principles**: Achieves Principle 2 fully
- **Testing**: Different module combinations work correctly
- **Effort**: 3 sessions

#### Task 4.2: Module Dependency Resolution
- **Objective**: Automatic dependency handling with validation
- **Implementation**:
  - Enhanced dependency graph processing
  - Circular dependency detection
  - Missing dependency error handling
  - Optimal execution order determination
- **Principles**: Supports Principle 2 robustness
- **Testing**: Complex dependency graphs resolved correctly
- **Effort**: 2 sessions

#### Task 4.3: Configuration Validation Framework
- **Objective**: Validate configurations against available modules
- **Implementation**:
  - Schema validation for module configurations
  - Parameter bounds checking
  - Module compatibility validation
  - Clear error reporting
- **Principles**: Principle 8 - validation for modules
- **Testing**: Invalid configurations caught with clear errors
- **Effort**: 2 sessions

### Exit Criteria
- ✅ Module combinations configurable at runtime
- ✅ No recompilation needed for different physics
- ✅ Dependency resolution robust and automatic
- ✅ Configuration validation comprehensive

---

## Phase 5: I/O Modernization (ENHANCED)

### Architectural Principles Addressed
- **Principle 7**: Format-Agnostic I/O ⭐ **PRIMARY**

### Objectives
- **Primary**: Replace the direct file I/O functions in `sage` with a new, unified, module-aware I/O system.
- **Secondary**: Create a single, format-agnostic interface for all tree input and galaxy output.
- **Foundation**: The I/O system will adapt automatically to the properties made available by loaded modules.

### Entry Criteria
- ✅ Module system fully operational (Phase 4 complete)
- ✅ Property system reflects properties of loaded modules at runtime.
- ✅ Runtime module configuration is working.

### Tasks

#### Task 5.1: Module-Aware Property Output
- **Objective**: Output file contents adapt automatically to the set of loaded modules.
- **Implementation**:
  - The I/O system will query the property system at runtime to get a list of all available properties (core + physics).
  - The output writer will dynamically create datasets/columns only for the available properties.
  - Core properties are always written; physics properties are only written if their corresponding module is loaded.
- **Principles**: Integrates Principles 2 (Runtime Modularity), 3 (Metadata-Driven), and 7 (Format-Agnostic I/O).
- **Testing**: Output files for different module combinations contain exactly the correct set of properties.

#### Task 5.2: Unified I/O Interface
- **Objective**: Create a common interface to replace the separate tree input and galaxy output functions.
- **Implementation**:
  - Design a generic `io_manager_t` struct with function pointers for `open`, `read_forest`, `write_galaxies`, `close`.
  - Create concrete implementations of this interface for HDF5 (`io_hdf5.c`) and binary (`io_binary.c`) formats.
  - Refactor `main.c` and `core_build_model.c` to use the `io_manager_t` interface, removing direct calls to functions in `io_tree_hdf5.c`, `io_save_binary.c`, etc.
  - The specific implementation (HDF5 or binary) will be chosen at runtime based on the parameter file.
- **Principles**: Principle 7 - format agnostic.
- **Testing**: Both HDF5 and binary formats for input and output work correctly through the unified interface.

#### Task 5.3: HDF5 Hierarchical Output
- **Objective**: Implement a modern, hierarchical HDF5 output format that is module-aware.
- **Implementation**:
  - The HDF5 writer will create a root group for core properties.
  - It will create a separate HDF5 group for each loaded physics module (e.g., `/Cooling/`, `/StarFormation/`).
  - Properties belonging to a module will be written as datasets within that module's group.
  - Property metadata (descriptions, units) from the YAML files will be written as HDF5 attributes for each dataset.
- **Principles**: Principle 7 with modern standards.
- **Testing**: The generated HDF5 files have a clean, hierarchical structure and are easily readable by standard analysis tools like `h5py`.

### Exit Criteria
- ✅ The I/O system automatically adapts its output to the set of loaded modules.
- ✅ All I/O operations are handled through the unified `io_manager_t` interface.
- ✅ The monolithic functions in `io_tree_*.c` and `io_save_*.c` have been replaced.
- ✅ Analysis tool compatibility is maintained for the new HDF5 output format.

---

## Phase 6: Validation & Polish (ENHANCED)

### Architectural Principles Addressed
- **All Principles**: Comprehensive validation of complete system

### Objectives
- **Primary**: Comprehensive scientific and architectural validation
- **Secondary**: Performance optimization and documentation
- **Foundation**: System ready for production use

### Entry Criteria
- ✅ All implementation phases complete
- ✅ Basic testing passing
- ✅ Documentation framework in place (from Phase 1)

### Tasks

#### Task 6.1: Scientific Validation Suite
- **Objective**: Validate all module combinations scientifically
- **Implementation**:
  - Test all supported module combinations
  - Compare with legacy results for identical configurations
  - Validate edge cases and error conditions
  - Performance regression testing
- **Principles**: Validates all principles working together
- **Testing**: Scientific accuracy maintained across all configurations
- **Effort**: 3 sessions

#### Task 6.2: Architectural Principle Validation
- **Objective**: Comprehensive compliance testing
- **Implementation**:
  - Test physics-agnostic core (Principle 1)
  - Validate runtime modularity (Principle 2)
  - Verify metadata-driven behavior (Principle 3)
  - Test all remaining principles systematically
- **Principles**: All 8 principles validated
- **Testing**: Comprehensive principle compliance confirmed
- **Effort**: 2 sessions

#### Task 6.3: Performance Optimization
- **Objective**: Achieve performance parity with legacy
- **Implementation**:
  - Profile modular architecture performance
  - Optimize critical paths
  - Module interface performance tuning
  - Memory access pattern optimization
- **Principles**: Maintain performance while achieving modularity
- **Testing**: Performance within 10% of legacy
- **Effort**: 2 sessions

### Exit Criteria
- ✅ All 8 architectural principles fully validated
- ✅ Scientific accuracy maintained across all module combinations
- ✅ Performance parity with legacy implementation
- ✅ System ready for production deployment

---

## Success Metrics & Validation

### Architectural Compliance Metrics
- **Principle 1**: Core runs successfully with zero physics modules loaded
- **Principle 2**: 5+ different module combinations work without recompilation
- **Principle 3**: All system structure generated from metadata
- **Principle 4**: Single property access system throughout codebase
- **Principle 5**: Unified merger tree processing for all configurations
- **Principle 6**: Memory usage bounded regardless of simulation size
- **Principle 7**: 3+ I/O formats supported through unified interface
- **Principle 8**: Type-safe property access with compile-time validation

### Scientific Validation Metrics
- **Accuracy**: Bit-exact results for identical module configurations
- **Completeness**: All legacy functionality accessible through modules
- **Flexibility**: Multiple module combinations produce expected results
- **Performance**: Runtime within 10% of legacy implementation

### Development Quality Metrics
- **Code Quality**: Professional standards maintained throughout
- **Testing**: Comprehensive coverage of modular functionality
- **Documentation**: Clear architecture and usage documentation
- **Maintainability**: Clean module interfaces and dependencies

---

## Risk Assessment & Mitigation

### High-Priority Risks

#### Risk: Module Interface Performance Overhead
- **Impact**: Degraded scientific performance
- **Mitigation**: Design zero-cost abstractions, profile early
- **Principle Impact**: Could affect adoption if performance suffers

#### Risk: Complex Module Dependencies
- **Impact**: Runtime configuration failures
- **Mitigation**: Limit dependency depth, clear error messages
- **Principle Impact**: Could undermine Principle 2 (runtime modularity)

### Medium-Priority Risks

#### Risk: Property System Complexity
- **Impact**: Developer adoption challenges
- **Mitigation**: Excellent documentation, clear examples
- **Principle Impact**: Affects Principle 3,4,8 usability

#### Risk: Scientific Validation Scope
- **Impact**: Missed edge cases in module combinations
- **Mitigation**: Systematic testing matrix, community beta testing
- **Principle Impact**: Could undermine overall system reliability

---

## Implementation Guidelines

### Development Workflow
1. **Phase Start**: Review architectural principles for this phase
2. **Task Implementation**: Validate principle compliance at each step
3. **Testing**: Both scientific and architectural validation required
4. **Phase End**: Comprehensive principle compliance review

### Architectural Compliance Checks
- Each commit must maintain/improve architectural principle compliance
- No regression in principle adherence tolerated
- Scientific accuracy preserved throughout architectural improvements

### Quality Standards
- **Architecture First**: No shortcuts that violate principles
- **Scientific Rigor**: Results must match legacy when configuration identical
- **Performance Aware**: Modular architecture shouldn't sacrifice performance
- **Documentation**: Principle compliance and usage clearly documented

---

## Conclusion

This restructured Master Implementation Plan establishes **architectural principle compliance as the foundation** rather than a late-stage goal. By implementing **Phase 2A (Core/Physics Separation)** first, all subsequent development occurs within an architecturally sound context.

The plan enables SAGE to achieve:
- **Complete Physics-Agnostic Core**: Principle 1 compliance from Phase 2A onward
- **Runtime Scientific Flexibility**: Different physics without recompilation
- **Modern Development Practices**: Type-safe, modular, well-tested codebase
- **Preserved Scientific Accuracy**: Identical results for identical configurations

Success will be measured by **both technical excellence and architectural integrity**, ensuring SAGE becomes a model for modern scientific software development while accelerating galaxy evolution research.

---

## Appendix A: Architectural Principle Quick Reference

| Principle | Phase | Status | Validation Method |
|-----------|-------|--------|-------------------|
| 1. Physics-Agnostic Core | 2A | 🎯 PRIMARY | Core runs without physics modules |
| 2. Runtime Modularity | 4 | 🎯 PRIMARY | Module configs without recompilation |
| 3. Metadata-Driven | 2B | 🎯 PRIMARY | System structure from YAML |
| 4. Single Source of Truth | 2B | Applied | Unified property access |
| 5. Unified Processing | 2A | Applied | Consistent tree processing |
| 6. Memory Efficiency | 3 | 🎯 PRIMARY | Bounded memory usage |
| 7. Format-Agnostic I/O | 5 | 🎯 PRIMARY | Multiple formats, unified interface |
| 8. Type Safety | 2B | Applied | Compile-time property validation |

This plan ensures no architectural violations occur during development, creating a robust foundation for long-term SAGE evolution.