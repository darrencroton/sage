# Final Report: SAGE Codebase Refactoring Plan

## Knowledge Development

Throughout this research process, our understanding of the SAGE codebase has evolved significantly from initial structural analysis to deep comprehension of specific implementation patterns. When we began examining the Semi-Analytic Galaxy Evolution model codebase, we first identified its general organization—a modular structure with separate files for different astrophysical processes and core functionality. This provided a foundation for deeper analysis.

As we progressed to code quality assessment, we discovered that while the code follows a logical organization at the file level, it suffers from several internal design patterns that impact maintainability. The extensive use of global variables, large monolithic functions, and inconsistent error handling emerged as key concerns. These patterns are common in scientific computing codes that have evolved over time through contributions from multiple researchers with varying programming backgrounds.

Our investigation into memory management revealed more specific implementation details and trade-offs. The custom memory management system with its LIFO (Last-In-First-Out) restriction represents an interesting design choice—while providing strong safeguards against memory leaks, it significantly constrains memory usage patterns. The fixed-increment reallocation strategy for galaxy arrays suggested optimization opportunities that weren't initially apparent.

The I/O efficiency analysis deepened our understanding of how SAGE handles large datasets. The conditional compilation approach for supporting both binary and HDF5 formats demonstrates a practical solution for balancing performance with cross-platform compatibility, though the implementations showed inconsistent error handling and buffer management.

Our final exploration of algorithmic patterns unveiled the most complex aspects of the codebase, revealing how physical models are intertwined with numerical algorithms. The recursive approach to tree processing and fixed time-stepping emerged as areas where modern computational techniques could offer substantial improvements without changing the underlying physics.

This progressive deepening of understanding—from high-level structure to specific implementation details—has provided us with comprehensive insight into both the strengths and limitations of the SAGE codebase, allowing us to develop targeted, effective refactoring recommendations.

## Comprehensive Analysis

### Code Structure and Organization

The SAGE codebase demonstrates a logical organization at the file level, with a clear separation between core functionality and astrophysical models. The directory structure divides code into main source files (`/code`), I/O-related functionality (`/code/io`), auxiliary data (`/extra`), parameter files (`/input`), and analysis scripts (`/output`). This high-level organization provides a solid foundation for the codebase.

Files are generally named according to their function, with a prefix indicating their role: `core_*.c/h` for core functionality, `model_*.c` for astrophysical processes, and `io/*.c/h` for input/output operations. This naming convention aids in understanding the codebase's structure at a glance. The modular approach allows for separation of concerns, with different physical processes implemented in different files.

However, despite this logical file organization, the internal structure of the code shows several limitations. The header organization relies heavily on a single large header file (`core_allvars.h`) that contains a mix of constants, structure definitions, and global variable declarations. This approach violates the principle of separation of concerns and creates implicit dependencies between components.

The execution flow starts in `main.c`, which initializes the simulation environment, processes merger tree files, constructs galaxies, evolves them through time, and saves the results. The core galaxy evolution is handled by the `construct_galaxies()` function, which recursively processes the merger tree, and `evolve_galaxies()`, which applies various physical processes to each galaxy. This structure is conceptually clear but suffers from implementation issues that impact maintainability.

### Code Quality and Maintainability

The code quality analysis revealed several issues that affect maintainability. The extensive use of global variables throughout the codebase creates implicit dependencies, makes the code harder to test, and increases the risk of subtle bugs. For example, parameters like `FirstFile`, `LastFile`, and configuration flags such as `ReionizationOn` are declared as globals in `core_allvars.h` and accessed throughout the code.

Many functions in the codebase are overly long and handle multiple responsibilities. For instance, `evolve_galaxies()` in `core_build_model.c` manages time integration, gas cooling, star formation, feedback, and merger events all within a single function. This complexity makes the code harder to understand, test, and maintain. Similarly, `join_galaxies_of_progenitors()` combines multiple operations with complex conditional logic.

The code style shows inconsistencies in indentation, bracing style, variable naming, and commenting patterns. While some functions are well-documented with explanatory comments, others have minimal documentation. The error handling approach mixes assertions, direct checks, and the `ABORT` macro, which terminates the program rather than allowing for graceful recovery in many cases.

The parameter handling in `core_read_parameter_file.c` uses a verbose, manual approach with a long list of parameter definitions. This pattern is error-prone and makes it difficult to add, remove, or modify parameters. A more structured, data-driven approach would improve maintainability and reduce the risk of errors.

### Memory Management Patterns

The memory management analysis revealed a custom memory tracking system implemented in `core_mymalloc.c`. This system uses arrays (`Table[]` and `SizeTable[]`) to track allocations and prints memory usage information when high watermarks are reached. It aligns allocations to 8-byte boundaries for performance and includes error checking for allocation failures.

However, the most significant limitation is the LIFO restriction, which requires that memory blocks be freed in exactly the reverse order they were allocated. This constraint is enforced by checking that the pointer being freed is the last allocated block:

```c
if(p != Table[Nblocks - 1])
{
  printf("Wrong call of myfree() - not the last allocated block!\n");
  ABORT(0);
}
```

While this approach may help catch memory management bugs early, it significantly limits the flexibility of memory usage patterns and complicates the code in various places.

The galaxy array reallocation strategy in `core_build_model.c` uses a fixed increment approach, increasing the size by 10,000 elements whenever the array is full. This simplistic approach could lead to frequent reallocations for large datasets or excessive memory usage for small ones. A growth factor approach (e.g., multiplying by 1.5 or 2) would be more efficient.

The code also uses fixed-size arrays based on maximum possible values (e.g., `ABSOLUTEMAXSNAPS` defined as 1000), which can lead to wasted memory if the actual data size is much smaller. A more dynamic approach would allocate memory based on actual needs.

### I/O Efficiency and Data Handling

The I/O analysis showed that SAGE supports both binary and HDF5 file formats through conditional compilation. This flexibility is valuable for different use cases, with binary I/O being simpler but less portable, and HDF5 offering better structure and cross-platform compatibility.

The code uses static file handles in several places to avoid repeatedly opening and closing files, which is generally good for performance. However, there is limited explicit buffer management for I/O operations, which could impact performance for large files.

The binary I/O implementation writes structures directly to disk, which is efficient but may cause cross-platform compatibility issues due to potential differences in structure alignment and padding. There is no explicit handling of endianness, which could cause problems when transferring binary files between different architectures.

The HDF5 implementation in `io/io_save_hdf5.c` is more complex but offers better structure with metadata and uses chunks for potential performance optimization. It provides better cross-platform compatibility but adds complexity to the codebase.

Error handling for I/O operations is inconsistent, with some operations having thorough error checking and others having minimal or no checks. A more consistent approach would improve robustness and help identify issues during execution.

### Algorithmic Clarity and Modernization

The algorithmic analysis identified several patterns that could benefit from modernization. The function `construct_galaxies()` uses recursion to process the merger tree structure, which is elegant but could potentially lead to stack overflow for very deep trees. An iterative approach using explicit stacks or queues would be more robust.

The code uses GSL (GNU Scientific Library) for numerical integration with hardcoded parameters for tolerance and method. Making these configurable would allow tuning the balance between performance and accuracy for different simulations.

The time integration uses a fixed number of steps (`STEPS` defined as 10) between snapshots. An adaptive time-stepping algorithm that takes smaller steps when properties are changing rapidly would be more efficient and accurate.

Physical models are directly implemented with conditional logic based on configuration flags. For example, the star formation prescription in `model_starformation_and_feedback.c` uses a simple conditional:

```c
if(SFprescription == 0)
{
  // Star formation code for prescription 0
}
else
{
  printf("No star formation prescription selected!\n");
  ABORT(0);
}
```

This approach works but has limited extensibility. A strategy pattern with different implementations for each prescription would be more maintainable and extensible.

The main evolution loop in `evolve_galaxies()` has complex nested loops and conditional logic that make the code hard to follow. Refactoring to separate different phases into distinct functions would improve clarity.

Throughout the code, floating-point values are compared directly, which can be problematic due to precision issues. Using epsilon values for these comparisons would make the code more robust.

## Practical Implications

### Prioritized Refactoring Roadmap

Based on our comprehensive analysis, we recommend the following prioritized refactoring roadmap, focusing on high-impact, low-effort improvements first:

#### Phase 1: Code Structure and Organization Improvements

1. **Refactor Header Organization**
   - Split `core_allvars.h` into multiple focused headers:
     - `constants.h` for physical and numerical constants
     - `types.h` for structure definitions
     - `globals.h` for global variable declarations
     - `config.h` for configuration parameters
   - Update include statements throughout the codebase to reference the appropriate headers
   - Add header guards to all header files

2. **Reduce Global Variable Usage**
   - Create a configuration structure to hold parameters
   - Update `read_parameter_file()` to populate this structure
   - Pass the configuration structure to functions that need it
   - Start with a subset of globals (e.g., file paths and simulation parameters)

3. **Improve Parameter Handling**
   - Create a table-driven approach for parameter definition
   - Implement a structured parameter registration system
   - Update `read_parameter_file()` to use this system
   - Add better error handling for parameter reading

#### Phase 2: Function Refactoring and Code Quality

1. **Break Down Large Functions**
   - Split `evolve_galaxies()` into smaller, focused functions:
     - `apply_physical_processes()`
     - `handle_mergers()`
     - `update_galaxy_properties()`
   - Refactor `join_galaxies_of_progenitors()` into smaller functions
   - Update function calls to maintain the same behavior

2. **Standardize Error Handling**
   - Create a consistent error handling approach
   - Distinguish between recoverable and unrecoverable errors
   - Add more informative error messages
   - Implement a logging system for warnings and errors

3. **Improve Code Documentation**
   - Add consistent function header comments
   - Document complex algorithms with clear explanations
   - Create high-level documentation explaining the code structure
   - Add more inline comments for complex calculations

#### Phase 3: Memory Management Optimization

1. **Relax LIFO Restriction**
   - Implement a more flexible memory management system
   - Use reference counting or another tracking approach
   - Update `myfree()` and `myrealloc()` to support this
   - Maintain backward compatibility for existing code

2. **Improve Array Allocation Strategy**
   - Replace fixed increment strategy with growth factor approach
   - Update galaxy array allocation in `core_build_model.c`
   - Implement dynamic sizing for fixed arrays where appropriate
   - Add memory usage optimization for large simulations

3. **Enhance Memory Debugging**
   - Add memory leak detection capabilities
   - Implement memory usage tracking per component
   - Add optional memory usage reporting
   - Create functions to validate memory integrity

#### Phase 4: I/O and Algorithm Modernization

1. **Standardize I/O Operations**
    - Create a consistent I/O interface for both binary and HDF5 formats
    - Implement explicit buffer management for better performance
    - Add consistent error handling for all I/O operations
    - Address endianness and structure alignment issues for binary I/O

2. **Modernize Algorithmic Patterns**
    - Replace recursion with iteration for tree processing
    - Implement a strategy pattern for physical models
    - Make numerical integration parameters configurable
    - Explore adaptive time-stepping for improved efficiency

3. **Improve Numerical Stability**
    - Use epsilon values for floating-point comparisons
    - Implement robust handling of division operations
    - Add bounds checking for critical calculations
    - Ensure consistent handling of edge cases

### Implementation Guidelines

For each refactoring task, we recommend the following implementation approach:

1. **Context Understanding**
   - Review the specific code component in detail
   - Understand the underlying physics or algorithms
   - Identify dependencies and potential side effects
   - Create unit tests to verify current behavior (if possible)

2. **Incremental Changes**
   - Make small, focused changes to minimize risk
   - Test each change before moving to the next
   - Maintain backward compatibility where possible
   - Document changes thoroughly

3. **Verification Process**
   - Run existing tests to ensure correctness
   - Create new tests for refactored components
   - Compare outputs with the original code
   - Validate against expected physical behavior

4. **Documentation Updates**
   - Update function and file documentation
   - Explain the rationale for significant changes
   - Provide examples for new patterns or approaches
   - Document any API changes or behavior differences

### Future Research Directions

While our refactoring plan addresses immediate code quality and structure issues, there are several directions for future research and development:

1. **Performance Optimization**
   - Profiling to identify computational bottlenecks
   - Parallel processing for independent calculations
   - Algorithm optimization for core physical processes
   - Memory access pattern optimization

2. **Enhanced Modularity**
   - Clearer separation of physical models from numerical methods
   - Plugin system for different physical implementations
   - Component-based architecture for easier extension
   - Independent testing of isolated components

3. **Improved User Experience**
   - Better error messages and diagnostics
   - Runtime configuration validation
   - Interactive visualization of simulation results
   - Integration with analysis tools

4. **Scientific Extensions**
   - Support for additional physical processes
   - Integration with other astrophysical models
   - Enhanced comparison with observational data
   - Parameter space exploration capabilities

By implementing these refactoring recommendations and exploring future directions, the SAGE codebase can become more maintainable, extensible, and potentially more efficient, while preserving the scientific validity of the model.