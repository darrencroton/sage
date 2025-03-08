# PhysicsState Structure Documentation

## Overview

The `PhysicsState` structure is part of SAGE's refactoring effort to reduce global variable usage and improve code organization. This structure encapsulates the physics-related derived values used throughout the simulation, making the physical relationships more explicit and easier to maintain.

## Structure Definition

```c
struct PhysicsState
{
  /* Units */
  double UnitTime_in_s;           /* Time unit in seconds */
  double UnitTime_in_Megayears;   /* Time unit in megayears */
  double UnitDensity_in_cgs;      /* Density unit in CGS */
  double UnitPressure_in_cgs;     /* Pressure unit in CGS */
  double UnitCoolingRate_in_cgs;  /* Cooling rate unit in CGS */
  double UnitEnergy_in_cgs;       /* Energy unit in CGS */
  
  /* Physical constants in simulation units */
  double G;                       /* Gravitational constant */
  double Hubble;                  /* Hubble constant */
  double RhoCrit;                 /* Critical density */
  
  /* Derived model parameters */
  double EnergySNcode;            /* Supernova energy in code units */
  double EtaSNcode;               /* Supernova efficiency in code units */
  
  /* Reionization parameters */
  double a0;                      /* Scale factor for reionization onset */
  double ar;                      /* Scale factor for reionization completion */
};
```

## Field Descriptions

### Units

- `UnitTime_in_s`: Time unit in seconds, derived from length and velocity units.
- `UnitTime_in_Megayears`: Time unit in millions of years, for convenience in astrophysical contexts.
- `UnitDensity_in_cgs`: Density unit in CGS, derived from mass and length units.
- `UnitPressure_in_cgs`: Pressure unit in CGS, derived from mass, length, and time units.
- `UnitCoolingRate_in_cgs`: Cooling rate unit in CGS, derived from pressure and time units.
- `UnitEnergy_in_cgs`: Energy unit in CGS, derived from mass, length, and time units.

### Physical Constants

- `G`: Gravitational constant in simulation units.
- `Hubble`: Hubble constant in simulation units.
- `RhoCrit`: Critical density of the universe in simulation units.

### Derived Model Parameters

- `EnergySNcode`: Supernova energy in code units, derived from SageConfig.EnergySN.
- `EtaSNcode`: Supernova efficiency in code units, derived from SageConfig.EtaSN.

### Reionization Parameters

- `a0`: Scale factor for the onset of reionization, derived from SageConfig.Reionization_z0.
- `ar`: Scale factor for the completion of reionization, derived from SageConfig.Reionization_zr.

## Usage Guidelines

During the transition phase, the `PhysicsState` structure is used alongside global variables to maintain backward compatibility. The `sync_globals_with_config()` and `sync_config_with_globals()` functions in `core_allvars.c` handle the synchronization between the structure fields and their global counterparts.

The `PhysicsState` structure is initialized in the `set_units()` function, which calculates all derived physical values based on the input parameters in `SageConfig`.

### Example:

```c
// Calculate supernova feedback energy
ejected_mass = (SageConfig.FeedbackEjectionEfficiency * 
               (PhysicsState.EtaSNcode * PhysicsState.EnergySNcode) / 
               (Gal[centralgal].Vvir * Gal[centralgal].Vvir) - 
               SageConfig.FeedbackReheatingEpsilon) * stars;
```

## Future Directions

Eventually, all code will transition to use the `PhysicsState` structure directly, eliminating the need for the corresponding global variables and synchronization functions.

For new code, always use the structure fields rather than the global variables to promote this transition.
