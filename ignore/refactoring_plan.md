# SAGE Codebase Refactoring Plan

## Introduction

This document outlines the comprehensive refactoring plan for the SAGE (Semi-Analytic Galaxy Evolution) codebase. The refactoring aims to improve code organization, reduce global variable usage, enhance maintainability, and ensure proper encapsulation of data, while maintaining full backward compatibility with existing simulations.

## Overall Goals

1. **Improve Code Organization**: Structure the code with clear separation of concerns
2. **Reduce Global Variable Usage**: Encapsulate state in appropriate structures
3. **Enhance Readability and Maintainability**: Add proper documentation and consistent style
4. **Ensure Backward Compatibility**: Maintain compatibility with existing data formats and workflows
5. **Improve Error Handling**: Implement better error detection and reporting

## Refactoring Phases

### Phase 1: Header Organization and Basic Structure

#### Phase 1.1: Header Organization (COMPLETED)
- Split monolithic `core_allvars.h` into focused headers
  - `constants.h`: Physical and numerical constants
  - `types.h`: Type definitions and structures
  - `globals.h`: Global variable declarations
  - `config.h`: Configuration settings and macros
- Add proper header guards
- Update include statements throughout the codebase

#### Phase 1.2: Reducing Global Variable Usage (COMPLETED)
- Create `SageConfig` structure for configuration parameters
- Create `SimulationState` structure for runtime state
- Update parameter file reading to populate structures
- Add synchronization with global variables for backward compatibility
- Fix memory management issues in the LIFO allocation system

### Phase 2: Accessor Functions and Code Organization

#### Phase 2.1: Accessor Functions (IN PROGRESS)
- Create accessor header `accessors.h` with comprehensive getter/setter functions
- Implement accessor functions in `accessors.c`
- Add accessor files to Makefile
- Update code to use accessor functions instead of direct globals
- Add validation in accessor functions where appropriate

#### Phase 2.2: Physical Constants as Defined Constants (COMPLETED)
- Enhance `constants.h` with comprehensive documented constants
- Add missing constants from physics calculations
- Organize constants into logical groupings with comments
- Update code to use constants from `constants.h`

#### Phase 2.3: Function-Specific Globals as Static Variables (IN PROGRESS)
- Convert memory tracking variables in `core_mymalloc.c` to static
- Convert cooling table variables in `core_cool_func.c` to static
- Make helper functions static when appropriate
- Identify and convert remaining function-specific globals

### Phase 3: Modularization and Advanced Refactoring (FUTURE)

#### Phase 3.1: Enhanced Error Handling
- Add comprehensive error detection and reporting
- Standardize error handling across the codebase
- Implement logging system with configurable detail levels

#### Phase 3.2: Physics Module Isolation
- Restructure physics modules with clear interfaces
- Reduce interdependencies between modules
- Improve unit testability

#### Phase 3.3: Memory Management Improvements
- Further optimize memory allocation and usage
- Add memory tracking and diagnostic tools
- Update LIFO system with more flexibility

#### Phase 3.4: I/O Module Modernization
- Refactor I/O to support multiple formats more cleanly
- Add support for modern formats (HDF5, FITS, etc.)
- Improve file handling error recovery

## Implementation Approach

### Backward Compatibility

Throughout all phases, backward compatibility is maintained by:
1. Preserving all existing global variables while adding new structures
2. Using synchronization functions to keep both versions of state in sync
3. Gradually migrating code to use new interfaces while maintaining old ones
4. Extensive testing to ensure simulation results are identical

### Testing Strategy

For each refactoring step:
1. Run test simulation to establish baseline results
2. Implement refactoring changes
3. Run the same test simulation with refactored code
4. Verify results are identical to baseline

### Documentation

All changes are documented in:
1. Inline code comments explaining changes
2. Function documentation headers
3. Updated README and other documentation files
4. Refactoring implementation report detailing progress

## Timeline and Priorities

The refactoring is being implemented in phases, with each phase focusing on specific aspects of the codebase. Priority is given to changes that:
1. Have the highest impact on maintainability
2. Reduce dependencies and improve modularity
3. Improve error detection and reporting
4. Fix potential memory management issues

## Conclusion

This refactoring plan provides a structured approach to improving the SAGE codebase while maintaining its functionality and compatibility. By implementing these changes, we aim to create a more maintainable, robust, and flexible codebase that can continue to evolve with the needs of galaxy formation modeling.
