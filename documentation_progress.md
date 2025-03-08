# SAGE Documentation Expansion Progress

## Project Overview
This file tracks the progress of adding comprehensive documentation to the SAGE codebase as part of the Phase 2 refactoring effort.

## Documentation Status

| File | @file Added | @brief Functions | Status | Completion Date | Notes |
|------|------------|------------------|--------|-----------------|-------|
| model_cooling_heating.c | ✅ | ✅ | Complete | 2025-03-08 | Added @file documentation |
| model_mergers.c | ✅ | ✅ | Complete | 2025-03-08 | Added @file documentation and @brief for estimate_merging_time() |
| model_starformation_and_feedback.c | ✅ | ✅ | Complete | 2025-03-08 | Added @file documentation |
| model_disk_instability.c | ✅ | ✅ | Complete | | Already well documented |
| model_infall.c | ✅ | ✅ | Complete | | Already well documented |
| model_reincorporation.c | ✅ | ✅ | Complete | | Already well documented |
| model_misc.c | ✅ | ✅ | Complete | 2025-03-08 | Added @file documentation and @brief for all functions |
| core_build_model.c | ✅ | ✅ | Complete | 2025-03-08 | Added @file documentation and @brief for all functions |
| core_save.c | ✅ | ✅ | Complete | 2025-03-08 | Added @file documentation and @brief for all functions |
| core_io_tree.c | ✅ | ✅ | Complete | 2025-03-08 | Added @file documentation and @brief for all functions |
| core_cool_func.c | ✅ | ✅ | Complete | 2025-03-08 | Added @file documentation and @brief for all functions |
| core_init.c | ✅ | ✅ | Complete | 2025-03-08 | Added @file documentation and @brief for all functions |
| core_read_parameter_file.c | ✅ | ✅ | Complete | 2025-03-08 | Added @file documentation and @brief for read_parameter_file() |
| core_simulation_state.c | ✅ | ✅ | Complete | 2025-03-08 | Added @file documentation and improved @brief for all functions |
| core_allvars.c | ✅ | N/A | Complete | 2025-03-08 | Added @file documentation (no functions to document) |
| error_handling.c | ✅ | ✅ | Complete | 2025-03-08 | Added @file documentation and @brief for all functions |
| io/tree_hdf5.c | ✅ | ✅ | Complete | 2025-03-08 | Added @file documentation and @brief for all functions |
| io/tree_binary.c | ✅ | ✅ | Complete | 2025-03-08 | Added @file documentation and @brief for all functions |
| io/io_save_hdf5.c | ✅ | ✅ | Complete | 2025-03-08 | Added @file documentation and @brief for key functions |
| parameter_table.c | ✅ | ✅ | Complete | 2025-03-08 | Added @file documentation and @brief for all functions |
| main.c | ✅ | ✅ | Complete | | Already well documented |
| core_mymalloc.c | ✅ | ✅ | Complete | | Already well documented |

## Documentation Templates

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
