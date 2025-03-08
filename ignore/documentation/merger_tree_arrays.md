# Merger Tree Arrays Documentation

## Overview

The merger tree arrays in SAGE represent the dark matter halo merger history that forms the foundation for galaxy evolution. These structures remain global due to their size and central importance in tracking the hierarchical formation of structure.

## Primary Arrays

### Halo Data Arrays

```c
struct halo_data *Halo;
struct halo_aux_data *HaloAux;
```

- **Halo**: Contains all physical properties of dark matter halos from the simulation
- **HaloAux**: Contains auxiliary data for each halo, primarily used for tracking galaxies

### Tree Structure Arrays

```c
int *TreeNHalos;      // Number of halos in each tree
int *TreeFirstHalo;   // Index of the first halo in each tree
int *FirstHaloInSnap; // Index of the first halo in each snapshot
```

These arrays define the structure of merger trees by providing indexing information.

### Tree Indexing

```c
int Ntrees;           // Number of trees in current file
int TreeID;           // Current tree ID being processed
```

These variables track the current merger tree being processed.

## Lifecycle Management

### Allocation

- **Halo Data**: Allocated in `load_tree()` based on the number of halos in the current tree
- **Tree Structure Arrays**: Allocated in `load_tree_table()` based on the number of trees in the file
- **HaloAux**: Allocated in `load_tree()` alongside `Halo`

### Deallocation

- **Halo, HaloAux**: Freed in `free_galaxies_and_tree()`
- **Tree Structure Arrays**: Freed in `free_tree_table()`

## Primary Operations

### 1. Loading Tree Table

`load_tree_table()` reads the tree structure from a file and initializes:
- `Ntrees`: Number of trees in the file
- `TreeNHalos`: Array of halo counts per tree
- `TreeFirstHalo`: Array of first halo indices per tree

### 2. Loading Tree Data

`load_tree()` loads the complete data for a specific tree:
- Reads halo properties from the file
- Allocates and populates the `Halo` array
- Allocates and initializes the `HaloAux` array

### 3. Tree Traversal

Merger trees are typically processed recursively from leaves to root in `construct_galaxies()`:
- Processes each branch of the tree recursively
- Marks halos as done using `HaloAux[i].DoneFlag`
- Builds a hierarchy of galaxies that matches the halo merger history

## Critical Sections

### Tree Navigation in `construct_galaxies()`

```c
int ngal;
ngal = find_most_massive_progenitor(halonr);
for(i = 0; i < Halo[halonr].Len; i++)
  if(Halo[halonr].Prog[i] >= 0 && Halo[halonr].Prog[i] != ngal)
    construct_galaxies(Halo[halonr].Prog[i], treenr);
```

This recursive traversal follows the merger tree structure from leaves to root, ensuring every halo is processed in the correct order. The algorithm relies on accurate connectivity information in the `Halo` structure.

### Galaxy-Halo Mapping in `init_galaxy()`

```c
Gal[p].HaloNr = halonr;
HaloAux[halonr].NGalaxies++;
if(HaloAux[halonr].FirstGalaxy == -1)
  HaloAux[halonr].FirstGalaxy = p;
```

This establishes the critical mapping between galaxies and their host halos, linking the galaxy formation model to the underlying dark matter structure.

## Access Patterns

### Safe Tree Navigation Pattern:

```c
void process_halo_and_descendants(int halonr)
{
  // Validate halo index before access
  if(halonr >= 0 && halonr < TreeNHalos[TreeID]) {
    // Process this halo
    process_halo(halonr);
    
    // Process each descendant
    int descendant = Halo[halonr].Descendant;
    if(descendant >= 0)
      process_halo_and_descendants(descendant);
  }
}
```

### Unsafe Access Pattern (Avoid):

```c
void process_halo_chain(int halonr)
{
  // Missing bounds check and null termination check!
  while(halonr >= 0) {
    process_halo(halonr);
    halonr = Halo[halonr].Descendant;
  }
}
```

## Best Practices

1. **Validation**: Always check that halo indices are valid before access.
2. **Cycle Prevention**: Be careful when traversing links (descendant, progenitor) to avoid infinite loops.
3. **Flag Management**: Use `HaloAux[i].DoneFlag` to track which halos have been processed.
4. **Multiple Access**: When a halo is accessed in multiple places, use consistent patterns to update its properties.
5. **Tree Structure**: Preserve the tree structure - do not modify descendant/progenitor links.

## Related Functions

- `load_tree_table()`: Reads the tree structure from file
- `load_tree()`: Loads complete data for a single tree
- `construct_galaxies()`: Main function for processing trees recursively
- `join_galaxies_of_progenitors()`: Connects galaxies across the merger tree
- `find_most_massive_progenitor()`: Identifies the main progenitor halo
