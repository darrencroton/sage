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

## 🟢 Current Task in Progress
#### Task 1.4: Configuration Abstraction Layer
- **Objective**: Create modern YAML-based configuration system replacing legacy `.par` files.
- **Implementation**:
  - Design `config_t` structure for unified access.
  - Create `config_reader.c` with YAML support (using `libyaml`).
  - Convert existing `.par` parameter files to modern YAML format.
  - Implement comprehensive configuration validation framework.
- **Testing**: YAML configuration files load correctly into the `config_t` struct with full validation.
- **Documentation**: YAML configuration format specification and migration guide.
- **Risk**: YAML library integration - use proven `libyaml` library.
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

### Task 1.4: Configuration Abstraction Layer
- [ ] Design `config_t` structure for unified access
- [ ] Create `config_reader.c` with YAML support (using libyaml library)
- [ ] Convert existing `.par` files to YAML format
- [ ] Implement comprehensive configuration validation framework
- [ ] Test YAML configuration files load correctly into `config_t`

### Task 1.5: I/O Abstraction Layer
- [ ] Create `io_manager.h` header with generic interface
- [ ] Define `io_manager_t` struct with function pointers for I/O operations
- [ ] Initialize function pointers to existing I/O functions in `main.c`
- [ ] Replace direct I/O function calls with abstraction layer calls
- [ ] Test both binary and HDF5 I/O work through abstraction

### Task 1.6: Development Infrastructure
- [ ] Create `docs/` directory structure
- [ ] Create placeholder documentation files
- [ ] Create `docs/quick-reference.md` as central index

### Task 1.7: Testing and Automation Framework
- [ ] Integrate CTest with CMake build system
- [ ] Create `tests/` directory with initial unit tests
- [ ] Create `.github/workflows/ci.yml` for GitHub Actions
- [ ] Test CI workflow builds and runs tests successfully
- [ ] Create `docs/testing-guide.md` with testing framework documentation

## Completion Criteria
**Phase 1 Complete When:**
- Project builds successfully using CMake
- Codebase organized into new directory structure
- Abstraction layers for Memory, Configuration, and I/O are in place and used
- YAML-based configuration system replaces legacy `.par` files completely
- Unit testing framework integrated with CI pipeline
- Simulation produces scientifically identical results to original baseline
- Foundational documentation structure established

**Phase 1 Status**: 3/7 tasks complete

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