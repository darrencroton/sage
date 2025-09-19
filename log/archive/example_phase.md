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

# Current Phase: 2A/6 (Core/Physics Separation) - Physics-Agnostic Core Infrastructure

## 🎯 Architectural Principles Addressed
- **Principle 1**: Physics-Agnostic Core Infrastructure ⭐ **PRIMARY**
- **Principle 5**: Unified Processing Model

## Phase Objectives
- **PRIMARY**: Remove all physics knowledge from core infrastructure
- **SECONDARY**: Enable physics-free mode operation
- **CRITICAL**: Establish foundation for all subsequent development
- **COMPLIANCE**: Achieve Principle 1 (Physics-Agnostic Core) before any other major work

## Current Progress

### Task 2A.1: Physics Module Interface Design
- [x] Design `physics_module_t` structure with execution phases
- [x] Define standard module lifecycle (init, execute, cleanup)
- [x] Create module registration and lookup functions
- [x] Add module capability declarations
- [x] Test interface compiles and supports basic operations

### Task 2A.2: Core Evolution Pipeline Abstraction
- [ ] Remove direct `#include` of physics headers from core
- [ ] Replace physics function calls with module interface calls
- [ ] Remove ALL physics property handling from core (initialization, normalization, calculations)
- [ ] **CRITICAL**: Use `CORE_PROP_*` naming for core property access (NOT `GALAXY_PROP_*`)
- [ ] Move physics utility functions to physics_essential_functions.c/h (for Task 2A.4)
- [ ] Create conditional execution based on loaded modules
- [ ] Maintain identical execution order and logic
- [ ] Verify core compiles without physics dependencies

### Task 2A.3: Physics-Free Mode Implementation
- [ ] Core processes merger trees without physics calculations
- [ ] Galaxies have core properties only (halo inheritance, tracking)
- [ ] Tree traversal and basic galaxy lifecycle functional
- [ ] Scientific output limited to halo properties
- [ ] Test physics-free mode runs complete tree processing

### Task 2A.4: Legacy Physics Module Wrapping
- [ ] Create adapter modules for each physics function
- [ ] Maintain identical physics calculations
- [ ] Register modules using new interface
- [ ] Preserve all existing scientific behavior
- [ ] Verify physics calculations produce identical results

### Task 2A.5: Module Dependency Framework
- [ ] Modules declare their dependencies
- [ ] Simple topological sort for execution order
- [ ] Error handling for missing dependencies
- [ ] Support for optional module loading
- [ ] Test module dependencies resolved correctly

### Task 2A.6: Create Physics Module Developer Guide
- [ ] Create `docs/physics-module-guide.md` based on the final implementation.
- [ ] Document the module lifecycle, interface, and best practices.
- [ ] Provide a step-by-step tutorial for creating a new module, using the wrapped legacy modules as examples.
- [ ] Update `docs/quick-reference.md` to link to the new guide.

## Completion Criteria
**Phase 2A Complete When:**
- Core compiles and runs without physics modules loaded
- Physics modules wrapped in interface, calculations unchanged
- Module interface enables conditional physics execution
- Physics-free mode processes merger trees successfully
- All existing tests pass with wrapped physics modules

**Phase 2A Status**: 1/5 tasks completed

## Validation Requirements
- **Architecture Compliance**: Core has zero physics knowledge
- **Scientific Accuracy**: Physics results identical to pre-modular
- **Functionality**: Both physics-free and full-physics modes work
- **Performance**: No significant runtime degradation

## Why Phase 2A Must Come First
This phase addresses the **fundamental architectural violation** identified in the legacy code:
- `src/core/core_build_model.c` directly includes physics headers
- Core makes direct calls to physics functions
- Core cannot run without physics (violates Principle 1)

**All subsequent work must build on architecturally compliant foundations.**

## Inter-Phase Dependencies
- **From Phase 1**: ✅ CMake build system (for module compilation)
- **From Phase 1**: ✅ Memory abstraction (for module memory management)
- **From Phase 1**: ✅ Property system infrastructure (Tasks 2.1-2.2)
- **To Phase 2B**: Modular architecture needed before property migration
- **To Phase 3**: Module-aware memory management requires established modules
- **To Phase 4**: Runtime module system builds on basic module interface

## Critical Success Factors
1. **Physics-Agnostic Core**: Core must have zero physics knowledge
2. **Interface Design**: Module interface must support all existing physics
3. **Scientific Preservation**: Wrapped physics must produce identical results
4. **Performance**: Module interface overhead must be minimal
5. **Foundation Quality**: This enables all subsequent modular development

**This phase establishes the architectural foundation that makes SAGE's vision achievable.**