# Galaxy Arrays Documentation

## Overview

The galaxy arrays (`Gal` and `HaloGal`) are the primary data structures in SAGE that store all galaxy properties during simulation. These arrays remain global due to their size and central importance in the codebase, but should be accessed according to clear ownership rules.

## Structure

```c
struct GALAXY *Gal, *HaloGal;
```

- **Gal**: Array for holding galaxy data during Friends-of-Friends (FoF) group processing
- **HaloGal**: Array for storing the final galaxy data for the current merger tree

## Lifecycle Management

### Allocation

Both arrays are dynamically allocated in `load_tree()` (called from `main()`) with initial sizes:

```c
MaxGals = (int)(MAXGALFAC * TreeNHalos[treenr]);
if(MaxGals < 10000)
  MaxGals = 10000;

FoF_MaxGals = 10000;

HaloGal = mymalloc(sizeof(struct GALAXY) * MaxGals);
Gal = mymalloc(sizeof(struct GALAXY) * FoF_MaxGals);
```

### Reallocation

`HaloGal` may be resized in `construct_galaxies()` if more space is needed:

```c
if(NumGals == MaxGals)
{
  MaxGals += 10000;
  HaloGal = myrealloc(HaloGal, sizeof(struct GALAXY) * MaxGals);
}
```

### Deallocation

Both arrays are freed in `free_galaxies_and_tree()`:

```c
myfree(Gal);
myfree(HaloGal);
```

## Key Variables Controlling Access

- **NumGals**: Total number of galaxies stored for current tree in `HaloGal`
- **MaxGals**: Maximum allocated size of the `HaloGal` array
- **FoF_MaxGals**: Maximum allocated size of the `Gal` array

## Primary Operations

### 1. Galaxy Initialization

New galaxies are initialized in `init_galaxy()`, setting default values for all fields.

### 2. Galaxy Construction

The complete galaxy population is built recursively in `construct_galaxies()`, which:
- Traces merger trees from leaves to root
- Joins galaxies from progenitor halos
- Sets up parent-child relationships
- Initializes new galaxies when needed

### 3. Galaxy Evolution

Galaxies evolve through physical processes in `evolve_galaxies()`:
- Gas cooling
- Star formation
- Feedback
- Mergers
- AGN activity

### 4. Galaxy Mergers

Galaxy mergers are handled in `deal_with_galaxy_merger()`, which:
- Combines galaxy properties
- Triggers starbursts
- Updates morphologies

## Critical Sections

### Galaxy Addition in `add_galaxies_together()`

```c
void add_galaxies_together(int t, int p)
{
  Gal[t].ColdGas += Gal[p].ColdGas;
  Gal[t].MetalsColdGas += Gal[p].MetalsColdGas;
  // ... more properties combined
}
```

This function modifies multiple properties of the target galaxy (`t`) based on the properties of another galaxy (`p`). Care must be taken to ensure `t` and `p` are valid indices and that all relevant properties are updated consistently.

### Array Resizing in `construct_galaxies()`

```c
if(NumGals == MaxGals)
{
  MaxGals += 10000;
  HaloGal = myrealloc(HaloGal, sizeof(struct GALAXY) * MaxGals);
}
```

This section dynamically grows the `HaloGal` array. Any references to `HaloGal` elements should be refreshed after this operation as the memory location may change.

## Access Patterns

### Safe Access Pattern:

```c
void process_galaxy(int gal_index)
{
  // Validate index before access
  if(gal_index >= 0 && gal_index < NumGals) {
    struct GALAXY *g = &HaloGal[gal_index];
    // Process galaxy g
  }
}
```

### Unsafe Access Pattern (Avoid):

```c
void process_galaxy(int gal_index)
{
  // Missing bounds check!
  HaloGal[gal_index].StellarMass += new_stars;
}
```

## Best Practices

1. **Validation**: Always check that galaxy indices are within bounds before access.
2. **Pointers**: When processing a galaxy extensively, use a pointer to its element for clarity and performance.
3. **Synchronization**: After modifying galaxies, ensure any derived quantities are updated.
4. **Initialization**: Always initialize all galaxy fields when creating new galaxies.
5. **Modification**: Modify galaxy fields through dedicated functions where possible, rather than direct access.

## Related Functions

- `construct_galaxies()`: Main function for building the galaxy population
- `evolve_galaxies()`: Main function for evolving galaxies through time
- `join_galaxies_of_progenitors()`: Connects galaxies across snapshots
- `save_galaxies()`: Exports the final galaxy properties to output files
