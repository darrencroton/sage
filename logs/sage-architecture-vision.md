# SAGE Architectural Vision
**Version**: 2.1 (Guiding the `sage` Refactor)  
**Date**: September 2024  
**Purpose**: To provide comprehensive architectural guidance for the transformation of the SAGE `sage-vanilla` codebase into a modern, modular galaxy evolution framework.  
**Audience**: Developers (human and AI) and contributors working on the SAGE refactoring project.  
**Status**: Foundational principles guiding the development outlined in the Master Implementation Plan.

---

## Vision Statement

SAGE is being transformed from a monolithic galaxy evolution model into a **physics-agnostic core with runtime-configurable physics modules**, enabling scientific flexibility, maintainability, and extensibility while preserving exact scientific accuracy.

This architecture enables researchers to easily experiment with different physics combinations, developers to work independently on core infrastructure and physics modules, and the system to evolve gracefully as new scientific understanding emerges.

---

## 8 Core Architectural Principles

These principles guide all design decisions and implementation choices in SAGE:

### 1. Physics-Agnostic Core Infrastructure

**Principle**: The core SAGE infrastructure has zero knowledge of specific physics implementations.

**Requirements**:
- Core systems (memory management, I/O, tree processing, configuration) operate independently of physics.
- Physics modules interact with the core only through well-defined interfaces.  
- The core can execute successfully with no physics modules loaded (physics-free mode).
- Physics modules are pure add-ons that extend core functionality.

**Benefits**: Enables independent development of physics and infrastructure, simplifies testing, allows for future physics module hot-swapping, and reduces complexity in core systems.

**In Practice**: The core evolution loop will not contain any `#include "model_cooling.h"` or direct calls to `starformation_and_feedback()`. Instead, it will iterate through a list of registered physics modules and call a generic `execute()` function on each one.

### 2. Runtime Modularity

**Principle**: Physics module combinations should be configurable at runtime without recompilation.

**Requirements**:
- Module selection is done via configuration files, not compile-time flags.
- Modules self-register and declare their capabilities and dependencies.
- The execution pipeline adapts dynamically to the loaded module set.
- An empty pipeline (core-only execution) is a valid configuration.

**Benefits**: Provides scientific flexibility for different research questions, makes it easier to experiment with physics combinations, offers deployment flexibility, and simplifies the testing of different physics scenarios.

**In Practice**: A user should be able to switch from one cooling model to another, or disable AGN feedback entirely, by only changing a JSON or YAML configuration file and re-running the executable.

### 3. Metadata-Driven Architecture

**Principle**: The system's structure should be defined by metadata rather than hardcoded implementations.

**Requirements**:
- Galaxy properties (e.g., `StellarMass`, `ColdGas`) are defined in metadata files (like YAML), not hardcoded in C structs.
- Parameters are defined in metadata with automatic validation generation.
- The build system generates type-safe C code (headers, accessors) from this metadata.
- Output formats adapt automatically to the properties defined in the metadata.

**Benefits**: Reduces code duplication, eliminates manual synchronization between different representations, enables build-time optimization, and simplifies maintenance by creating a single source of truth for data structures.

### 4. Single Source of Truth

**Principle**: Galaxy data should have one authoritative representation with consistent access patterns.

**Requirements**:
- Eliminate any dual property systems or synchronization code.
- All access to galaxy data must go through a unified property system.
- This property system must be type-safe and allow for compile-time optimization.
- The lifecycle of a property (creation, modification, destruction) is managed consistently.

**Benefits**: Eliminates a class of synchronization bugs, simplifies debugging by having a single data path, reduces memory overhead, and improves performance through unified access patterns.

### 5. Unified Processing Model

**Principle**: SAGE should have one consistent, well-understood method for processing merger trees.

**Requirements**:
- A single tree traversal algorithm that handles all scientific requirements.
- Consistent galaxy inheritance and property calculation methods.
- Robust orphan galaxy handling that prevents data loss.
- A clear separation between the logic of tree traversal and the physics calculations performed at each node.

**Benefits**: Eliminates complexity from maintaining multiple processing modes, simplifies validation by having a single algorithm to verify, reduces the bug surface area, and makes the system easier for new developers to understand and modify.

### 6. Memory Efficiency and Safety

**Principle**: Memory usage should be bounded, predictable, and safe.

**Requirements**:
- Memory usage must be bounded and not grow with the total number of forests processed over a run.
- Memory management for galaxy arrays and properties should be automatic where possible.
- Memory should be allocated on a per-forest scope, with guaranteed cleanup after each forest is processed.
- The system must include tools for memory leak detection and prevention.

**Benefits**: Allows SAGE to handle large simulations reliably without running out of memory, reduces debugging overhead by preventing memory-related bugs, improves performance predictability, and enables processing of simulations that may be larger than available RAM.

**In Practice**: Memory for a given merger tree (halos, galaxies, module-specific data) will be allocated at the start of its processing and guaranteed to be freed upon completion, preventing memory growth over a run.

### 7. Format-Agnostic I/O

**Principle**: SAGE should support multiple input/output formats through unified interfaces.

**Requirements**:
- A common, abstract interface for all tree reading operations.
- A property-based output system that adapts to the data available at runtime.
- Proper handling of cross-platform issues like endianness.
- Graceful fallback mechanisms for unsupported features in certain formats.

**Benefits**: Ensures scientific compatibility with different simulation codes and analysis tools, future-proofs the model against format changes, simplifies validation across different formats, and makes it easier to integrate with external tools.

### 8. Type Safety and Validation

**Principle**: Data access should be type-safe with automatic validation.

**Requirements**:
- Type-safe property accessors (macros or functions) are generated from metadata.
- The system provides automatic bounds checking and validation where appropriate.
- The model should fail fast and with clear error messages upon invalid data access.

**Benefits**: Reduces runtime errors by catching problems at compile-time, improves the debugging experience with clear error messages, catches problems early in the development cycle, and increases confidence in the scientific accuracy of the results.

---

## Implementation Philosophy

### Metadata-Driven Development
- **Single Source of Truth**: YAML metadata prevents synchronization bugs between different code representations.
- **Code Generation**: Automatically generate type-safe C code from metadata definitions.
- **Build Integration**: Code generation is integrated into the build system, not a manual step.

### Physics-Agnostic Core  
- **Zero Physics Knowledge**: The core infrastructure has no understanding of specific physics processes.
- **Interface-Based Interaction**: Physics modules interact with the core only through well-defined interfaces.
- **Independent Development**: Core infrastructure and physics modules can be developed independently.

### Type Safety First
- **Compile-Time Validation**: Catch errors at compile-time rather than runtime.
- **Generated Access Patterns**: Type-safe property access is generated from metadata.
- **IDE Integration**: The system is designed to support full autocomplete, go-to-definition, and refactoring in modern IDEs.

### Standard Tools
- **Industry Standards**: Leverage proven tools (CMake, HDF5, JSON) rather than custom solutions.
- **Professional Workflow**: A modern development environment with IDE integration.
- **Debugging Support**: All standard debugging tools (like GDB and Valgrind) must work out of the box.

---

## Target Architecture

### System Components

```
┌─────────────────────────────────────────────────────────────┐
│                    SAGE Application                        │
├─────────────────────────────────────────────────────────────┤
│  Configuration System     │  Module System                  │
│  - JSON/Legacy .par       │  - Runtime loading              │
│  - Schema validation      │  - Dependency resolution        │
├─────────────────────────────────────────────────────────────┤
│                  Physics-Agnostic Core                     │
│  ┌─────────────────┬─────────────────┬─────────────────┐   │
│  │ Memory Mgmt     │ Property System │ I/O System      │   │
│  │ - Scoped alloc  │ - Type-safe     │ - Format unified│   │
│  │ - Auto cleanup  │ - Generated     │ - Cross-platform│   │
│  └─────────────────┴─────────────────┴─────────────────┘   │
│  ┌─────────────────┬─────────────────┬─────────────────┐   │
│  │ Tree Processing │ Pipeline Exec   │ Test Framework  │   │
│  │ - Unified model │ - Configurable  │ - Multi-level   │   │
│  │ - Inheritance   │ - Module phases │ - Scientific    │   │
│  └─────────────────┴─────────────────┴─────────────────┘   │
├─────────────────────────────────────────────────────────────┤
│                    Physics Modules                         │
│  ┌─────────────────┬─────────────────┬─────────────────┐   │
│  │ Cooling         │ Star Formation  │ AGN Feedback    │   │
│  │ Mergers         │ Reincorporation │ Disk Instability│   │
│  └─────────────────┴─────────────────┴─────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

### Data Flow

1. **Configuration Loading**: JSON/legacy parameter files are loaded and validated.
2. **Module Registration**: Physics modules self-register based on configuration.  
3. **Pipeline Creation**: An execution pipeline is built from the registered modules.
4. **Tree Processing**: The core loads and processes merger trees using a unified algorithm.
5. **Module Execution**: Physics modules execute in a dependency-resolved order.
6. **Output Generation**: A property-based output system adapts to the available properties.

---

## Success Criteria

### Technical Success
- **Architecture Validation**: Physics modules can be added or removed without any changes to the core.
- **Performance Validation**: Runtime performance is within 10% of the legacy implementation.  
- **Memory Validation**: Memory usage is bounded for large simulations.
- **I/O Validation**: Successful processing of all supported tree formats.

### Scientific Success
- **Accuracy Validation**: Identical scientific results to the legacy implementation for equivalent physics.
- **Completeness Validation**: All legacy functionality is preserved and accessible through modules.
- **Flexibility Validation**: Different physics combinations produce expected and scientifically plausible results.
- **Robustness Validation**: The system handles edge cases and error conditions gracefully.

### Developer Experience Success
- **Migration Success**: Existing users can transition with minimal effort.
- **Development Success**: New physics modules can be developed independently of the core.
- **Maintenance Success**: The system is maintainable by the development team.
- **Documentation Success**: Users and developers can understand and use the system effectively.

---

## Quality Attributes

### Maintainability
- **Modularity**: A clear separation of concerns with well-defined interfaces.
- **Documentation**: Comprehensive documentation for developers and users.
- **Code Quality**: Professional coding standards with a consistent style.
- **Testing**: Automated testing covering all major functionality.

### Extensibility  
- **Module Development**: Clear patterns for adding new physics modules.
- **Format Support**: A straightforward process for adding new I/O formats.
- **Property Extension**: Easy addition of new galaxy properties via metadata.
- **Configuration**: A flexible configuration system that supports new use cases.

### Reliability
- **Error Handling**: Robust error detection and recovery mechanisms.
- **Validation**: Comprehensive input and state validation.
- **Memory Safety**: Automatic memory management preventing leaks and corruption.
- **Debugging**: Clear error messages and debugging capabilities.

### Usability
- **Scientific Workflow**: Intuitive configuration for different scientific use cases.
- **Performance Analysis**: Built-in tools for understanding system performance.
- **Debugging Support**: Clear diagnostics for troubleshooting problems.
- **Migration Path**: A smooth transition from legacy usage patterns.

---

## Design Constraints

### Scientific Accuracy Preservation
All architectural changes must preserve the exact scientific behavior of the legacy implementation:
- Preservation of the original galaxy evolution algorithms.
- Identical property calculations and inheritance rules.
- Exact merger tree traversal ordering.
- Consistent numerical precision and rounding.

### Legacy Compatibility
The transformed system must maintain compatibility with:
- Existing parameter file formats (`.par` files).
- Analysis tools expecting specific output formats.
- Simulation data from various merger tree codes.
- Existing scientific validation datasets.

### Performance Constraints  
The new architecture must not significantly degrade performance:
- Memory usage should be comparable to or better than the legacy implementation.
- Runtime performance should be within 10% of the legacy implementation.
- I/O performance should be maintained or improved.
- Compilation time must remain reasonable.

---

## Current Implementation Status

This document outlines the architectural vision at the **start of the refactoring project**. The `sage` codebase serves as the baseline.

- **Baseline Strengths**: The `sage` codebase provides a scientifically validated foundation with robust utility libraries for memory management (`util_memory.c`), parameter handling (`util_parameters.c`), and error logging (`util_error.c`).

- **Project State**: All phases of the Master Implementation Plan are **planned** and have **not yet started**. The first step is to implement Phase 1: Infrastructure and Modernization Foundation.

---

## Conclusion

This architectural vision transforms SAGE into a modern, maintainable scientific software framework while preserving the scientific integrity that makes it valuable to the astrophysics community. The eight core principles provide a clear foundation for all development decisions, ensuring the system remains focused on its primary goals: scientific accuracy, developer productivity, and long-term maintainability.

The key insight driving this transformation is that **scientific accuracy and architectural elegance are not mutually exclusive**. By applying proven software engineering principles to scientific computing, we create a system that accelerates scientific discovery through improved flexibility, reliability, and maintainability.

Success will be measured not just by technical metrics, but by the system's ability to enable new scientific insights through easier experimentation, more reliable results, and faster development of new physics models.