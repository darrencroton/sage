# SAGE Error Handling Guidelines

This document outlines guidelines for consistent error handling across the SAGE codebase. It defines when to use each error level and provides examples to ensure a uniform approach to error reporting and handling.

## Error Severity Levels

SAGE uses five severity levels for error handling, each with a specific purpose:

1. **FATAL_ERROR**: For unrecoverable errors requiring immediate termination
2. **ERROR_LOG**: For recoverable errors where processing can continue
3. **WARNING_LOG**: For potential issues that don't affect correctness
4. **INFO_LOG**: For important operational information
5. **DEBUG_LOG**: For detailed debugging information

## When to Use Each Error Level

### FATAL_ERROR

Use for unrecoverable errors that prevent further operation and require immediate program termination.

**Use when**:
- File open/read failures that prevent further processing
- Memory allocation failures for critical components
- Invalid parameter values that violate core assumptions
- Unsupported input types or formats
- Unexpected conditions that make it impossible to continue safely

**Examples**:
```c
if (!(file = fopen(filename, "r"))) {
    FATAL_ERROR("Failed to open required input file '%s'. Error: %s", filename, strerror(errno));
}

if (buffer == NULL) {
    FATAL_ERROR("Memory allocation failed for buffer (%zu bytes)", size);
}

if (my_TreeType < 0 || my_TreeType >= NUM_TREE_TYPES) {
    FATAL_ERROR("Unsupported tree type %d. Valid range: 0-%d", my_TreeType, NUM_TREE_TYPES-1);
}
```

### ERROR_LOG

Use for recoverable errors where processing can continue but with potential limitations or fallbacks.

**Use when**:
- Non-critical file write failures
- When operations succeed but with sub-optimal results
- When a fallback approach can be used
- When skipping a processing step is acceptable

**Examples**:
```c
if (nwritten != expected_count) {
    ERROR_LOG("Failed to write complete data to file '%s'. Expected %d elements, wrote %d elements",
              filename, expected_count, nwritten);
    // Continue with limited functionality
}

if (cooling_table == NULL) {
    ERROR_LOG("No cooling table available for metallicity %g. Using nearest available table",
              metallicity);
    // Use nearest available table
}
```

### WARNING_LOG

Use for potential issues that don't affect correctness but might indicate problems or unexpected behavior.

**Use when**:
- Sub-optimal parameters are detected
- Performance concerns arise
- Unusual but valid input values are encountered
- Using a default value because a specified value is out of range

**Examples**:
```c
if (Gal[p].BulgeMass / Gal[p].StellarMass > 0.99) {
    WARNING_LOG("Bulge mass nearly equals total stellar mass in galaxy %d (%.2f%%)",
                p, 100.0 * Gal[p].BulgeMass / Gal[p].StellarMass);
}

if (Gal[p].MetalsColdGas > Gal[p].ColdGas) {
    WARNING_LOG("Metal mass exceeds gas mass in galaxy %d. Metallicity: %.2f",
                p, Gal[p].MetalsColdGas / Gal[p].ColdGas);
}

if (cooling_time < minimum_timestep) {
    WARNING_LOG("Cooling time (%.2e) shorter than minimum timestep (%.2e)",
                cooling_time, minimum_timestep);
}
```

### INFO_LOG

Use for important operational information that doesn't indicate errors or warnings.

**Use when**:
- Reporting processing milestones
- Logging resource usage information
- Reporting configuration settings
- Providing update on progress

**Examples**:
```c
INFO_LOG("Processing tree %d of %d (%d%% complete)",
         treenr+1, Ntrees, (treenr+1)*100/Ntrees);

INFO_LOG("Memory usage: %d MB", (int)(get_memory_usage() / (1024*1024)));

INFO_LOG("Output galaxies written to: %s", output_filename);
```

### DEBUG_LOG

Use for detailed debugging information that is valuable during development but not necessary for regular operation.

**Use when**:
- Tracking function entry/exit points
- Logging intermediate calculation values
- Reporting progress within loops
- Providing detailed state information

**Examples**:
```c
DEBUG_LOG("Entering function with params: x=%g, y=%g", x, y);

DEBUG_LOG("Intermediate calculation result: %g", intermediate_value);

DEBUG_LOG("Galaxy %d properties: StellarMass=%g, ColdGas=%g",
          p, Gal[p].StellarMass, Gal[p].ColdGas);
```

## Error Handling Best Practices

1. **Be Specific**: Include enough context in error messages to identify the problem and its location. Include variable values, file names, and other relevant information.

2. **Be Consistent**: Use the appropriate error level as defined in this document.

3. **Be Helpful**: Where possible, suggest remedies or alternatives in error messages.

4. **Check Return Values**: Always check return values from system calls and library functions.

5. **Memory Management**: Always verify memory allocations and handle failures gracefully.

6. **File Operations**: Check for failures in file operations and provide context in error messages.

7. **Error Propagation**: Consider how errors propagate up the call stack. Lower-level functions should report errors to higher-level functions when appropriate.

## Legacy ABORT Macro

The SAGE codebase contains a legacy `ABORT` macro that redirects to `FATAL_ERROR` for backward compatibility:

```c
#define ABORT(sigterm) FATAL_ERROR("Program aborted with exit code %d", sigterm)
```

For consistency, prefer using `FATAL_ERROR` directly with a descriptive error message rather than `ABORT` for new code.
