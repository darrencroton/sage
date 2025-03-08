# SAGE Documentation Expansion Handover

## Project Context

This document provides detailed information for expanding documentation coverage in the SAGE (Semi-Analytic Galaxy Evolution) codebase, as part of the Phase 2 refactoring effort. The previous task focused on error handling improvements has been successfully completed, with the implementation of a standardized error handling system throughout the codebase.

## Current Documentation Status

The codebase currently has:
- Documentation standards established in `doc_standards.md`
- Only 5 of 22 .c files have `@file` documentation (23%)
- Only 8 of 22 .c files have functions with `@brief` documentation (36%)
- Some files like `model_starformation_and_feedback.c` have excellent documentation that follows the standard
- The documentation quality is inconsistent across the codebase

Files with `@file` documentation:
1. core_mymalloc.c
2. main.c
3. model_disk_instability.c
4. model_infall.c
5. model_reincorporation.c

Files with `@brief` function documentation:
1. core_mymalloc.c
2. main.c
3. model_cooling_heating.c
4. model_disk_instability.c
5. model_infall.c
6. model_mergers.c
7. model_reincorporation.c
8. model_starformation_and_feedback.c

## Documentation Standards

According to `doc_standards.md`, the following templates should be used:

### Function Documentation Template

```c
/**
 * @brief   Brief description of what the function does
 *
 * @param   param1    Description of first parameter
 * @param   param2    Description of second parameter
 * ...
 * @return  Description of return value (if applicable)
 *
 * Detailed description of the function, including:
 * - Purpose and functionality
 * - Algorithm explanation for complex functions
 * - References to scientific papers or equations (if applicable)
 * - Edge cases or special considerations
 * - Relation to other functions
 * - Usage examples (when helpful)
 */
```

### File Documentation Template

```c
/**
 * @file    filename.c
 * @brief   Brief description of file contents
 *
 * Detailed description of the file's purpose, what functionality 
 * it provides, and how it relates to other components.
 *
 * Key functions:
 * - function1(): Brief description
 * - function2(): Brief description
 *
 * References:
 * - Paper/publication references if applicable
 */
```

## Task Breakdown

### 1. Documentation Inventory and Prioritization

1. Create a detailed inventory of all 22 .c files in the codebase, noting:
   - Current documentation status
   - File complexity/importance
   - Prioritization score

2. Prioritize documentation effort based on:
   - Core functionality (core_*.c files)
   - Scientific importance (model_*.c files)
   - File complexity (number and size of functions)
   - Current documentation status

### 2. File-by-File Documentation Implementation

For each file (in priority order):

1. Add `@file` documentation at the top of the file
2. Add `@brief` documentation to all significant functions
3. Add parameter descriptions for all function parameters
4. Add detailed descriptions of complex algorithms
5. Add scientific references where appropriate
6. Ensure inline comments explain complex calculations

### 3. Consistency Checking

1. Verify consistent terminology throughout the documentation
2. Ensure units are specified for physical quantities
3. Check that all parameters are documented
4. Verify that scientific references are accurate

## Files to Document (In Priority Order)

### High Priority (Core Scientific Models)

1. model_cooling_heating.c (has @brief, needs @file)
2. model_mergers.c (has @brief, needs @file)
3. model_starformation_and_feedback.c (has @brief, needs @file)
4. model_disk_instability.c (already well documented)
5. model_infall.c (already well documented)
6. model_reincorporation.c (already well documented)
7. model_misc.c (needs @brief and @file)

### Medium Priority (Core Framework)

8. core_build_model.c (needs @brief and @file)
9. core_save.c (needs @brief and @file)
10. core_io_tree.c (needs @brief and @file)
11. core_cool_func.c (needs @brief and @file)
12. core_init.c (needs @brief and @file)
13. core_read_parameter_file.c (needs @brief and @file)
14. core_simulation_state.c (needs @brief and @file)
15. core_allvars.c (needs @brief and @file)

### Lower Priority (I/O and Supporting)

16. error_handling.c (needs @brief and @file)
17. io/tree_hdf5.c (needs @brief and @file)
18. io/tree_binary.c (needs @brief and @file)
19. io/io_save_hdf5.c (needs @brief and @file)
20. parameter_table.c (needs @brief and @file)
21. main.c (already well documented)
22. core_mymalloc.c (already well documented)

## Example of Good Documentation

`model_starformation_and_feedback.c` provides an excellent example of well-documented functions:

```c
/**
 * @brief   Updates galaxy properties due to star formation
 *
 * @param   p             Index of the galaxy in the Gal array
 * @param   stars         Mass of stars formed in this time step
 * @param   metallicity   Current metallicity of the cold gas
 *
 * This function implements the changes to galaxy properties caused by
 * star formation. It:
 * 1. Reduces cold gas mass (accounting for recycling)
 * 2. Reduces cold gas metal content
 * 3. Increases stellar mass
 * 4. Increases stellar metal content
 *
 * The recycling fraction (RecycleFraction) represents the portion of
 * stellar mass that is returned to the ISM immediately through stellar
 * winds and supernovae. This implements a simple form of the instantaneous
 * recycling approximation.
 */
```

## Domain Knowledge Requirements

To effectively document the SAGE codebase, the following domain knowledge is helpful:

1. **Astrophysics Concepts**:
   - Galaxy formation and evolution
   - Dark matter halos and merger trees
   - Star formation and stellar feedback
   - Gas cooling and heating processes
   - Galaxy mergers and morphological transformations

2. **Semi-Analytic Modeling**:
   - Understanding of merger tree structure and processing
   - Physical prescriptions for baryonic processes
   - Numerical time integration techniques
   - Relationship between dark matter and baryonic components

3. **Key Scientific References**:
   - White & Frenk (1991) - Original semi-analytic model framework
   - Croton et al. (2006) - AGN feedback implementation
   - Kauffmann et al. (1999) - Star formation implementation
   - Bower et al. (2006) - Cooling implementation

## Implementation Strategy

1. Start with scientific model files, as they contain the most important physical processes
2. For each file:
   - First understand the scientific context and purpose
   - Document the file header with @file
   - Document each significant function with @brief
   - Document parameter meaning, especially physical units
   - Add detailed explanations of complex algorithms

3. When documenting core infrastructure files:
   - Focus on explaining the relationship to other components
   - Document key data structures and their purpose
   - Explain design decisions and trade-offs

4. Update `refactoring_progress.md` after each file is completed

## Progress Tracking

Create a tracking table in `refactoring_progress.md`:

| File | @file Added | @brief Functions | Status |
|------|------------|------------------|--------|
| model_cooling_heating.c | ❌ | ✅ (partial) | In Progress |
| model_mergers.c | ❌ | ✅ (partial) | Pending |
| ... | ... | ... | ... |

## Conclusion

This documentation expansion effort will significantly improve the maintainability, understandability, and scientific transparency of the SAGE codebase. By focusing on the high-priority files first and ensuring consistent documentation practices, the codebase will become more accessible to new developers and easier to maintain for current ones.

The well-documented scientific models will also improve the transparency and reproducibility of the scientific results, which is essential for computational astrophysics research.
