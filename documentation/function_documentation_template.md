# SAGE Function Documentation Template

## Function Documentation Format

All significant functions in the SAGE codebase should follow this documentation format:

```c
/**
 * @brief   Brief description of what the function does
 *
 * @param   param1    Description of first parameter
 * @param   param2    Description of second parameter
 * ...
 * @return  Description of return value (if applicable)
 *
 * This function provides detailed explanation of the functionality, including:
 * - Purpose and what problem it solves
 * - Algorithm details for complex functions
 * - Key equations or physical models being implemented
 * - References to scientific papers when applicable
 * - Special cases, edge cases, or limitations
 */
```

## Inline Comments Format

Inline comments should explain complex calculations, non-obvious algorithm steps, and the physical meaning of variables:

```c
/* Calculate cooling radius where t_cool = t_dyn */
rcool = sqrt(rho0 / rho_rcool);

/* Determine cooling regime:
 * 1. rcool > Rvir: cold accretion - rapid cooling throughout halo
 * 2. rcool < Rvir: hot halo regime - cooling only within rcool */
if(rcool > Gal[gal].Rvir)
  coolingGas = Gal[gal].HotGas / (Gal[gal].Rvir / Gal[gal].Vvir) * dt;
else
  coolingGas = (Gal[gal].HotGas / Gal[gal].Rvir) * (rcool / (2.0 * tcool)) * dt;
```

## File Documentation Format

Each source file should begin with a comprehensive header comment:

```c
/**
 * @file    filename.c
 * @brief   Brief description of file contents
 *
 * This file implements [specific functionality], including:
 * - Feature 1
 * - Feature 2
 * - Feature 3
 *
 * Key functions:
 * - function1(): Brief description
 * - function2(): Brief description
 *
 * Scientific references:
 * - Author et al. (YEAR) for [specific model]
 * - Author (YEAR) for [specific algorithm]
 */
```

## Best Practices

1. **Be Explicit About Units**: Always document the physical units for parameters and return values.

2. **Explain Complex Physics**: Document the physical meaning behind complex equations.

3. **Document Edge Cases**: Note how special cases are handled (e.g., zero values, limits).

4. **Cross-Reference**: Mention related functions when applicable.

5. **Focus on Why, Not Just How**: Explain why a particular approach was chosen, not just what it does.

6. **Keep Comments Updated**: When modifying code, always update the corresponding documentation.

## Example: Well-Documented Function

```c
/**
 * @brief   Calculates gas cooling based on halo properties and cooling functions
 *
 * @param   gal    Index of the galaxy in the Gal array
 * @param   dt     Time step size in Gyr
 * @return  Mass of gas that cools from hot to cold phase in 10^10 Msun/h
 *
 * This function implements the standard cooling model where hot gas cools
 * from an isothermal density profile based on a cooling radius. The cooling
 * rate depends on the gas temperature (determined by the virial velocity),
 * the gas metallicity, and the corresponding cooling function.
 *
 * Two cooling regimes are considered:
 * 1. "Cold accretion" when rcool > Rvir: rapid cooling throughout the halo
 * 2. "Hot halo cooling" when rcool < Rvir: cooling only within cooling radius
 *
 * References:
 * - White & Frenk (1991) for the cooling model
 * - Springel et al. (2001) for implementation details
 */
```
