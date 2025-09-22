<!-- Purpose: Current project phase context -->
<!-- Update Rules:
- 500-word limit!
- Include:
  • Phase objectives
  • Current progress as a checklist
  • Completion criteria
  • Inter-phase dependencies
- At major phase completion archive as phase-[X].md and refresh for next phase
-->

# Current Phase: 1/6 (Infrastructure Foundation) - Preparing for Modularization

## 🟡 Next Task Ready for Implementation
#### Task 1.5: I/O Abstraction Layer
- **Objective**: Create an abstraction layer for tree input and galaxy output to prepare for a unified, format-agnostic I/O system.
- **Implementation**:
  - Create a new `io_manager.h` header.
  - Define a generic `io_manager_t` struct containing function pointers for key I/O operations (e.g., `load_tree_table`, `load_tree`, `save_galaxies`, `finalize_galaxy_files`).
  - In `main.c`, create an `io_manager_t` instance and initialize its function pointers to point to the existing functions in `tree.c` and `save_binary.c`/`save_hdf5.c`.
  - Replace direct calls to these I/O functions in the main loop with calls through the `io_manager_t` function pointers.
- **Testing**: The code compiles and runs, producing identical output files. Both binary and HDF5 I/O function correctly through the abstraction layer.
- **Effort**: 2 sessions (moderate complexity)

## 🎯 Architectural Principles Addressed
- **Principle 6**: Memory Efficiency and Safety (preparation)
- **Principle 7**: Format-Agnostic I/O (preparation)

## Phase Objectives
- **PRIMARY**: Replace legacy Makefile with modern CMake build system
- **SECONDARY**: Reorganize codebase into logical directory structure preparing for core/physics split
- **CRITICAL**: Create abstraction layers for Memory, Configuration, and I/O to enable smooth modular refactoring
- **FOUNDATION**: Establish modern development infrastructure (testing, CI/CD)

## Current Progress

### Task 1.1: CMake Build System Setup ✅ COMPLETE
- [x] Create root `CMakeLists.txt` with project configuration
- [x] Set up source file discovery and compilation flags
- [x] Configure detection for optional dependencies (HDF5, MPI)
- [x] Enable out-of-tree builds
- [x] Test CMake build produces scientifically identical binary to Makefile
- [x] Update `README.md` with CMake build instructions

### Task 1.2: Directory Reorganization ✅ COMPLETE
- [x] Create `src/core`, `src/physics`, `src/io`, `src/utils` directories
- [x] Use `git mv` to move source files into appropriate subdirectories
- [x] Update all `#include` paths for new directory structure
- [x] Create `docs/` and `tests/` directories
- [x] Test project builds successfully with new structure
- [x] Create `docs/directory-structure.md` explaining layout
- [x] Update plotting system path resolution for new structure
- [x] Update all documentation (README.md, CLAUDE.md, docs/) for new paths

### Task 1.3: Memory Management Centralization ✅ COMPLETE
- [x] Create `src/core/memory.h` that includes `util_memory.h`
- [x] Replace all `malloc/calloc/realloc/free` with `mymalloc/mycalloc/myrealloc/myfree`
- [x] Ensure all memory allocations go through centralized system
- [x] Test memory usage with Address Sanitizer (`-fsanitize=address`) - no memory errors or leaks

### Task 1.4: Configuration Abstraction Layer ✅ COMPLETE
- [x] Design `config_t` structure for unified access
- [x] Create `config_reader.c` with YAML support (using libyaml library)
- [x] Convert existing `.par` files to YAML format
- [x] Implement comprehensive configuration validation framework
- [x] Test YAML configuration files load correctly into `config_t`

### Task 1.5: I/O Abstraction Layer
- [ ] Create `io_manager.h` header with generic interface
- [ ] Define `io_manager_t` struct with function pointers for I/O operations
- [ ] Initialize function pointers to existing I/O functions in `main.c`
- [ ] Replace direct I/O function calls with abstraction layer calls
- [ ] Test both binary and HDF5 I/O work through abstraction

### Task 1.6: Development Infrastructure ✅ COMPLETE
- [x] Create `docs/` directory structure
- [x] Create `docs/physics-module-guide.md` placeholder (for Phase 2A)
- [x] Create `docs/user-guide.md` placeholder (iterative development, final in Phase 6)
- [x] Create `docs/quick-reference.md` as central index
- [x] Comprehensive documentation system with professional standards

### Task 1.7: Testing and Automation Framework ✅ COMPLETE
- [x] Integrate CTest with CMake build system
- [x] Create `tests/` directory with initial unit tests (`test_numeric.c`)
- [x] Create comprehensive test framework (`tests/test_runner.h`)
- [x] Create `.github/workflows/ci.yml` for GitHub Actions
- [x] Validate existing `test_yaml_config.c` with framework
- [x] Test framework fully operational with `ctest --output-on-failure`
- [x] Create `docs/testing-guide.md` with comprehensive testing documentation
- [x] Update all project documentation with testing information

## Completion Criteria
**Phase 1 Complete When:**
- Project builds successfully using CMake
- Codebase organized into new directory structure
- Abstraction layers for Memory, Configuration, and I/O are in place and used
- YAML-based configuration system replaces legacy `.par` files completely
- Unit testing framework integrated with CI pipeline
- Simulation produces scientifically identical results to original baseline
- Foundational documentation structure established

## Validation Requirements
- **Build System**: CMake produces identical binary to Makefile
- **Scientific Accuracy**: All results identical to pre-refactor baseline
- **Code Quality**: Clean compilation with no new warnings
- **Documentation**: Clear instructions for new build process

## Why Phase 1 is Essential
This phase establishes the **foundation infrastructure** required for all subsequent modular development:

- **CMake Build**: Enables modular compilation and modern dependency management
- **Directory Structure**: Logical organization preparing for core/physics separation
- **Abstraction Layers**: Decouple implementations to enable incremental refactoring
- **Testing Infrastructure**: Validates correctness throughout transformation

**Without these foundations, modular development becomes significantly more complex and error-prone.**

## Inter-Phase Dependencies
- **To Phase 2A**: CMake build system needed for module compilation
- **To Phase 2A**: Memory abstraction enables module-aware memory management
- **To Phase 2B**: Directory structure prepares for property system integration
- **To Phase 3**: Memory centralization enables module-aware tracking
- **To Phase 4**: Configuration abstraction enables YAML-based module configuration
- **To Phase 5**: I/O abstraction enables unified, modular I/O system

## Critical Success Factors
1. **Scientific Preservation**: No changes to physics calculations or results
2. **Build System Reliability**: CMake must be as robust as current Makefile
3. **Abstraction Quality**: Layers must enable smooth future refactoring
4. **Documentation Standards**: Clear, maintainable documentation from start
5. **Testing Foundation**: Reliable testing to validate all future changes

**This phase creates the stable platform enabling all architectural transformation work.**