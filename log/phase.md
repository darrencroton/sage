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

# Current Phase: 2A/6 (Core/Physics Separation) - Physics-Agnostic Architecture

## 🟢 Next Task Ready for Implementation
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

## 🎯 Architectural Principles Addressed
- **Principle 1**: Physics-Agnostic Core Infrastructure ⭐ **PRIMARY**
- **Principle 5**: Unified Processing Model

## Phase Objectives
- **PRIMARY**: Remove all physics knowledge from core infrastructure
- **SECONDARY**: Enable physics-free mode operation
- **CRITICAL**: Establish foundation for all subsequent development

## Current Progress

### Task 2A.1: Physics Module Interface Design ✅ COMPLETE
- [x] Design `physics_module_t` structure with execution phases
- [x] Define standard module lifecycle (init, execute, cleanup)
- [x] Create module registration and lookup functions
- [x] Add module capability declarations
- [x] Test interface compiles and supports basic module operations

### Task 2A.2: Core Evolution Pipeline Abstraction ⏳ PENDING
- [ ] Remove direct `#include` of physics headers from core
- [ ] Replace physics function calls with module interface calls
- [ ] **CRITICAL**: Use `CORE_PROP_*` naming for core property access (NOT `GALAXY_PROP_*`)
- [ ] Create conditional execution based on loaded modules
- [ ] Maintain identical execution order and logic
- [ ] Test core compiles without physics dependencies

### Task 2A.3: Physics-Free Mode Implementation ⏳ PENDING
- [ ] Enable core to process merger trees without physics calculations
- [ ] Implement galaxies with core properties only (halo inheritance, tracking)
- [ ] Ensure tree traversal and basic galaxy lifecycle functional
- [ ] Limit scientific output to halo properties
- [ ] Test physics-free mode runs complete merger tree processing

### Task 2A.4: Legacy Physics Module Wrapping ⏳ PENDING
- [ ] Create `src/physics/essential_physics_functions.c/h` for shared utilities
- [ ] Move `get_disk_radius()`, `estimate_merging_time()` to essential functions
- [ ] Create adapter modules for each physics function
- [ ] Ensure modules handle all physics property initialization and normalization
- [ ] Maintain identical physics calculations
- [ ] Register modules using new interface
- [ ] Preserve all existing scientific behavior

### Task 2A.5: Module Dependency Framework ⏳ PENDING
- [ ] Implement module dependency declarations
- [ ] Create simple topological sort for execution order
- [ ] Add error handling for missing dependencies
- [ ] Support optional module loading
- [ ] Test module dependencies resolved correctly

### Task 2A.6: Write Physics Module Developer Guide ⏳ PENDING
- [ ] Complete `docs/physics-module-guide.md` based on final implementation
- [ ] Document module lifecycle and `physics_module_t` interface
- [ ] Provide step-by-step tutorial for creating new modules
- [ ] Use wrapped legacy modules as examples
- [ ] Update `docs/quick-reference.md` with guide links
- [ ] Test developer can follow guide to create simple new module

## Completion Criteria
**Phase 2A Complete When:**
- ✅ Core compiles and runs without physics modules loaded
- ✅ Physics modules wrapped in interface, calculations unchanged
- ✅ Module interface enables conditional physics execution
- ✅ Physics-free mode processes merger trees successfully
- ✅ All existing tests pass with wrapped physics modules
- ✅ Physics module developer guide is complete

## Validation Requirements
- **Architecture Compliance**: Core has zero physics knowledge
- **Scientific Accuracy**: Physics results identical to pre-modular
- **Functionality**: Both physics-free and full-physics modes work
- **Performance**: No significant runtime degradation

## Why Phase 2A is Critical
This phase establishes the **core architectural foundation** that enables all subsequent modular development:

- **Physics-Agnostic Core**: Core infrastructure with zero physics knowledge (Principle 1)
- **Module Interface**: Clean separation between core and physics logic
- **Foundation for Modularity**: All future development occurs in architecturally sound context
- **Validation Framework**: Both physics-free and full-physics modes prove separation works

**Without this architectural foundation, the property system, memory management, and I/O modernization cannot be implemented correctly.**

## Inter-Phase Dependencies
- **From Phase 1**: CMake build system, directory structure, abstraction layers
- **To Phase 2B**: Property system integration requires modular context
- **To Phase 3**: Memory management requires module-aware architecture
- **To Phase 4**: Configuration system requires module interface
- **To Phase 5**: I/O system requires module-aware output

## Critical Success Factors
1. **Architectural Purity**: Zero physics knowledge in core components
2. **Scientific Preservation**: Identical physics results after modularization
3. **Interface Design**: Clean, extensible module interface
4. **Testing Rigor**: Both physics-free and full-physics modes validated
5. **Documentation Quality**: Clear module development guidance

**This phase creates the architecturally sound foundation enabling all future SAGE evolution.**