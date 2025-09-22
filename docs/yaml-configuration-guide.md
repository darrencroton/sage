# SAGE YAML Configuration Guide

This document describes the modern YAML-based configuration system for SAGE, which replaces the legacy .par file format. The YAML configuration provides a structured, human-readable format with enhanced validation and extensibility.

## Configuration File Structure

SAGE configuration files use YAML format with the following top-level sections:

### Files Section

Controls input/output file locations and formats:

```yaml
files:
  galaxy_file_name: "model"           # Base name for galaxy output files
  output_dir: "./output/results/"     # Directory for output files
  tree_name: "trees_063"              # Base name of merger tree files
  tree_type: "lhalo_binary"           # Tree format: 'lhalo_binary' or 'genesis_lhalo_hdf5'
  simulation_dir: "./input/data/"     # Directory containing simulation data
  snapshot_list: "./input/snaplist"   # File containing snapshot redshift list
```

**Required fields:** All fields in the files section are required.

**Tree Types:**
- `lhalo_binary`: LHalo binary format (default)
- `genesis_lhalo_hdf5`: Genesis HDF5 format (requires HDF5 support)

### Simulation Section

Defines simulation parameters and cosmology:

```yaml
simulation:
  first_file: 0                       # First tree file to process
  last_file: 7                        # Last tree file to process
  last_snapshot: 63                   # Highest snapshot number
  num_outputs: 8                      # Number of output snapshots (-1 = all)
  output_snapshots: [63, 37, 32, 27, 23, 20, 18, 16]  # Selected snapshots

  # Simulation properties
  num_tree_files: 8                   # Total number of tree files
  box_size: 62.5                      # Simulation box size (Mpc/h)

  # Cosmological parameters
  omega: 0.25                         # Matter density parameter
  omega_lambda: 0.75                  # Dark energy density parameter
  baryon_fraction: 0.17               # Cosmic baryon fraction
  hubble_h: 0.73                      # Hubble parameter (H0 = 100h km/s/Mpc)
  particle_mass: 0.0860657            # Dark matter particle mass (10^10 Msun/h)
```

**Required fields:** `first_file`, `last_file`, `last_snapshot`, `omega`, `omega_lambda`, `hubble_h`

### Physics Section

Controls physical processes and their parameters:

```yaml
physics:
  # Star formation
  star_formation:
    prescription: 0                   # SF prescription (0 = Croton et al. 2006)
    efficiency: 0.05                  # Star formation efficiency

  # Active galactic nuclei
  agn:
    recipe: 2                         # AGN recipe (0=off, 1=empirical, 2=Bondi-Hoyle, 3=cold cloud)
    radio_mode_efficiency: 0.08       # Radio mode heating efficiency
    quasar_mode_efficiency: 0.005     # Quasar mode wind efficiency
    black_hole_growth_rate: 0.015     # BH growth during mergers

  # Supernova feedback
  supernova:
    recipe: 1                         # Supernova feedback (0=off, 1=on)
    energy: 1.0e51                    # Energy per supernova (erg)
    efficiency: 5.0e-3                # Supernova efficiency

  # Gas physics
  feedback:
    reheating_epsilon: 3.0            # Mass ratio of reheated gas to formed stars
    ejection_efficiency: 0.3          # Efficiency of gas ejection

  # Reionization
  reionization:
    enabled: 1                        # Reionization effects (0=off, 1=on)
    z0: 8.0                          # Reionization redshift parameter
    zr: 7.0                          # Reionization transition parameter

  # Disk instabilities
  disk_instability:
    enabled: 1                        # Disk instability (0=off, 1=on)

  # Other physics parameters
  reincorporation_factor: 0.15        # Fraction of ejected gas reincorporated per dynamical time
  threshold_major_merger: 0.3         # Mass ratio threshold for major mergers
  threshold_sat_disruption: 1.0       # Disruption threshold for satellites

  # Chemical evolution
  yield: 0.025                        # Metal yield from star formation
  recycle_fraction: 0.43              # Fraction of stellar mass recycled
  frac_z_leave_disk: 0.0             # Fraction of metals ejected to hot phase
```

### Units Section

Defines unit conversions (typically unchanged):

```yaml
units:
  length_cm: 3.08568e+24              # Length unit (Mpc/h in cm)
  mass_g: 1.989e+43                   # Mass unit (10^10 Msun in g)
  velocity_cm_per_s: 100000           # Velocity unit (km/s in cm/s)
```

### Options Section

Additional configuration options:

```yaml
options:
  overwrite_output_files: 1           # Overwrite existing files (1=yes, 0=no)
```

## Complete Example

See `input/millennium.yaml` for a complete, working configuration file.

## Migration from .par Files

The YAML format provides the same functionality as legacy .par files with improved organization:

| .par Parameter | YAML Location |
|----------------|---------------|
| `OutputDir` | `files.output_dir` |
| `TreeName` | `files.tree_name` |
| `SfrEfficiency` | `physics.star_formation.efficiency` |
| `AGNrecipeOn` | `physics.agn.recipe` |
| `Omega` | `simulation.omega` |

## Validation

The configuration system provides comprehensive validation:

- **Required fields**: Missing required parameters trigger clear error messages
- **Type checking**: Numeric parameters validated for correct type and range
- **File paths**: Output directories created if they don't exist
- **Physics consistency**: Parameter combinations checked for physical validity

## Error Handling

Common configuration errors and solutions:

### File Not Found
```
ERROR: Configuration file 'config.yaml' not found
```
**Solution**: Check file path and ensure file exists

### Invalid YAML Syntax
```
ERROR: Invalid YAML syntax at line 15: mapping values are not allowed here
```
**Solution**: Check YAML formatting, indentation, and syntax

### Missing Required Parameter
```
ERROR: Required parameter 'files.output_dir' missing
```
**Solution**: Add the missing parameter to your configuration file

### Invalid Parameter Value
```
ERROR: Parameter 'simulation.omega' value 2.5 is outside valid range [0.0, 1.0]
```
**Solution**: Correct the parameter value to be within the valid range

## Advanced Features

### Comments
YAML supports comments using `#`:

```yaml
simulation:
  omega: 0.25        # Matter density parameter
  # omega_lambda: 0.75  # Uncomment to override default
```

### Multi-line Values
For long file paths or complex configurations:

```yaml
files:
  output_dir: >
    /very/long/path/to/output/
    directory/for/results/
```

### Environment Variables
Use environment variables in paths:

```yaml
files:
  output_dir: "${SAGE_OUTPUT_DIR}/results/"
```

Note: Environment variable substitution requires additional implementation.

## Schema Validation

The configuration system validates against the following schema:

- **String fields**: Must be valid paths or identifiers
- **Numeric fields**: Must be within physically reasonable ranges
- **Boolean fields**: Must be 0 (false) or 1 (true)
- **Array fields**: Must contain the correct number of elements

For detailed parameter descriptions and valid ranges, see the SAGE physics documentation.