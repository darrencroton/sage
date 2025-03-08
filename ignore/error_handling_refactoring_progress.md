# Error Handling Refactoring Progress

This document tracks the progress of refactoring the legacy error handling approach in the SAGE codebase, replacing direct `printf` calls with the new error handling system.

## Completed Files

- [x] `code/core_read_parameter_file.c` - Replaced all direct printf calls with appropriate logging macros (ERROR_LOG, INFO_LOG, DEBUG_LOG)
- [x] `code/core_io_tree.c` - Replaced all fprintf and ABORT calls with FATAL_ERROR
- [x] `code/core_mymalloc.c` - Already using the new error handling system
- [x] `code/io/tree_binary.c` - Replaced fprintf and ABORT calls with FATAL_ERROR
- [x] `code/io/tree_hdf5.c` - Replaced fprintf/printf and ABORT calls with appropriate logging macros

## Files Still Needing Refactoring

This section lists files that still need to be refactored to use the new error handling system.

### Code Files
- [x] `code/core_build_model.c` - No error handling code found that needs updating
- [x] `code/core_init.c` - Already using FATAL_ERROR, added INFO_LOG for informational messages
- [x] `code/core_save.c` - Replaced fprintf and ABORT calls with FATAL_ERROR, added ERROR_LOG for recoverable errors
- [ ] `code/model_cooling_heating.c`
- [ ] `code/model_disk_instability.c`
- [ ] `code/model_infall.c`
- [ ] `code/model_mergers.c`
- [ ] `code/model_misc.c`
- [ ] `code/model_reincorporation.c`
- [ ] `code/model_starformation_and_feedback.c`

### I/O Files
<!-- All I/O files have been refactored -->

## Refactoring Approach

The refactoring follows these guidelines:

1. Replace error messages with the appropriate logging macro:
   - `ERROR_LOG` for recoverable errors
   - `FATAL_ERROR` for unrecoverable errors
   - `WARNING_LOG` for warnings
   - `INFO_LOG` for informational messages
   - `DEBUG_LOG` for debugging information

2. Maintain existing control flow:
   - Keep error flag patterns where used
   - Replace final `ABORT` calls with `FATAL_ERROR`

3. Format messages consistently:
   - Remove trailing newlines (the logging system adds them)
   - Consistent capitalization and punctuation
   - Clear and descriptive error messages

## Testing

Each refactored file should be tested to ensure:
1. Successful compilation without warnings
2. Correct behavior with valid input
3. Appropriate error reporting with invalid input
