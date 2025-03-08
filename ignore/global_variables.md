# SAGE Global Variables Categorization

This document categorizes all global variables in the SAGE codebase and specifies the intended approach for each.

## Categories

1. **Configuration Parameters**: Move to SageConfig structure
2. **Runtime State Variables**: Group into specialized structures
3. **Temporary Calculation Variables**: Make local when possible
4. **Large Data Structures**: Keep global with clear ownership documentation

## Configuration Parameters (Move to SageConfig)

These parameters typically don't change during execution and represent simulation configuration:

| Variable | Description | Status | Notes |
|----------|-------------|--------|-------|
| UnitLength_in_cm | Length unit in cm | Migrated | Added to SageConfig, synchronized with global |
| UnitTime_in_s | Time unit in seconds | Not in SageConfig | Derived from other units, could be calculated on demand |
| UnitVelocity_in_cm_per_s | Velocity unit in cm/s | Migrated | Added to SageConfig, synchronized with global |
| UnitMass_in_g | Mass unit in g | Migrated | Added to SageConfig, synchronized with global |
| NOUT | Number of output snapshots | Migrated | Added to SageConfig, synchronized with global |
| ListOutputSnaps[ABSOLUTEMAXSNAPS] | List of output snapshot numbers | Not migrated | Array of snapshot indices for output |
| MAXSNAPS | Maximum number of snapshots | Not migrated | Derived from LastSnapShotNr |
| Snaplistlen | Length of snapshot list | Not migrated | Count of snapshots read from file |

## Runtime State Variables (Group into structures)

These variables represent the current state of the simulation and change during execution:

### Simulation State (Create SimulationState struct)

| Variable | Description | Status | Notes |
|----------|-------------|--------|-------|
| Ntrees | Number of trees in current file | Not grouped | Core runtime variable |
| NumGals | Total galaxies in current tree | Not grouped | Core runtime variable |
| GalaxyCounter | Unique galaxy ID counter | Not grouped | Counter for galaxy assignment |
| TotHalos | Total number of halos | Not grouped | Tracking halos across simulation |
| TotGalaxies[ABSOLUTEMAXSNAPS] | Galaxy count per snapshot | Not grouped | Output tracking |
| MaxGals | Maximum galaxies for current tree | Not grouped | Dynamic allocation control |
| FoF_MaxGals | Maximum galaxies for FoF | Not grouped | Dynamic allocation control |
| FileNum | Current file number | Not grouped | File tracking |
| TreeID | Current tree ID | Not grouped | Tree tracking |

### Units and Physical Constants (Keep as globals with clear documentation)

| Variable | Description | Status | Notes |
|----------|-------------|--------|-------|
| UnitDensity_in_cgs | Density unit in CGS | Not grouped | Derived unit |
| UnitPressure_in_cgs | Pressure unit in CGS | Not grouped | Derived unit |
| UnitCoolingRate_in_cgs | Cooling rate unit in CGS | Not grouped | Derived unit |
| UnitEnergy_in_cgs | Energy unit in CGS | Not grouped | Derived unit |
| UnitTime_in_Megayears | Time unit in Myr | Not grouped | Derived unit |
| G | Gravitational constant | Not grouped | Physical constant in simulation units |
| Hubble | Hubble constant | Not grouped | Physical constant in simulation units |
| RhoCrit | Critical density | Not grouped | Derived cosmological value |
| a0, ar | Scale factors for reionization | Not grouped | Derived from reionization parameters |
| EnergySNcode, EtaSNcode | Supernova energy/efficiency in code units | Not grouped | Derived from input parameters |

## Large Data Structures (Keep global with clear ownership)

These are the core data arrays that represent the simulation data:

| Variable | Description | Status | Notes |
|----------|-------------|--------|-------|
| struct GALAXY *Gal, *HaloGal | Galaxy data arrays | Global | Main simulation data |
| struct halo_data *Halo | Halo data array | Global | Main simulation data |
| struct halo_aux_data *HaloAux | Auxiliary halo data | Global | Main simulation data |
| double ZZ[ABSOLUTEMAXSNAPS] | Redshift at each snapshot | Global | Simulation data |
| double AA[ABSOLUTEMAXSNAPS] | Scale factor at each snapshot | Global | Simulation data |
| double *Age | Age at each snapshot | Global | Simulation data |
| int *TreeNgals[ABSOLUTEMAXSNAPS] | Galaxies per tree per snapshot | Global | Output tracking |
| int *TreeNHalos | Halos per tree | Global | Tree structure data |
| int *TreeFirstHalo | First halo in each tree | Global | Tree structure data |
| int *FirstHaloInSnap | First halo in each snapshot | Global | Snapshot structure data |
| gsl_rng *random_generator | Random number generator | Global | Simulation state |

## Parameter Handling (Keep as is or refactor separately)

| Variable | Description | Status | Notes |
|----------|-------------|--------|-------|
| NParam | Number of parameters | Global | Parameter table state |
| char ParamTag[MAXTAGS][50] | Parameter tag names | Global | Parameter table state |
| int ParamID[MAXTAGS] | Parameter type IDs | Global | Parameter table state |
| void *ParamAddr[MAXTAGS] | Parameter addresses | Global | Parameter table state |

## HDF5-specific (Consider creating HDF5State structure)

| Variable | Description | Status | Notes |
|----------|-------------|--------|-------|
| int HDF5Output | HDF5 output flag | Global | I/O configuration |
| char *core_output_file | HDF5 output file | Global | I/O state |
| size_t HDF5_dst_size | HDF5 structure size | Global | I/O state |
| size_t *HDF5_dst_offsets | HDF5 field offsets | Global | I/O state |
| size_t *HDF5_dst_sizes | HDF5 field sizes | Global | I/O state |
| const char **HDF5_field_names | HDF5 field names | Global | I/O state |
| hid_t *HDF5_field_types | HDF5 field types | Global | I/O state |
| int HDF5_n_props | HDF5 property count | Global | I/O state |

## MPI-specific (Keep as globals for now)

| Variable | Description | Status | Notes |
|----------|-------------|--------|-------|
| int ThisTask, NTask, nodeNameLen | MPI task info | Global | MPI state |
| char *ThisNode | MPI node name | Global | MPI state |
