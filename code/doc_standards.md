# SAGE Documentation Standards

## Function Documentation Template

Every significant function in the codebase should include a header comment that follows this structure:

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

## File Documentation Template

Each source file should start with a header comment that explains:

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

## Inline Comments

Inline comments should be used to explain:
- Complex calculations and their physical meaning
- Non-obvious algorithm steps
- Magic numbers or hardcoded values
- Potential pitfalls or edge cases

Format for inline comments:
- Use `//` for single-line comments
- Place comments before the code they describe
- For multi-line explanations, use `/* ... */`
- Keep comments concise and focused

## Variable Naming and Documentation

- All global variables should have explanatory comments in their declaration files
- Complex data structures should have member descriptions
- Units should be explicitly stated for physical quantities (e.g., Msun/h, Mpc/h, km/s)

## Documentation Maintenance

- Update documentation when code is modified
- Keep documentation synchronized with actual behavior
- Use consistent terminology throughout
