# SAGE Plotting Tool

A centralized plotting tool for the Semi-Analytic Galaxy Evolution (SAGE) model.

## Overview

This tool provides a single, comprehensive entry point for generating plots from SAGE galaxy formation model outputs. It features:

- A centralized `sage-plot.py` script that handles command-line arguments, parameter parsing, and plot management
- Self-contained figure modules in the `figures/` directory, each implementing a specific plot type
- Support for both snapshot plots (single snapshots) and evolution plots (across multiple snapshots)
- Integration with SAGE parameter files for consistent configuration
- Customizable output formatting and figure selection
- Consistent styling and formatting across all plot types
- Robust error handling and fallback mechanisms

## Usage

### Basic Usage

```bash
# Generate all snapshot plots using parameter file
python sage-plot.py --param-file=/path/to/sage_params.par

# Generate specific snapshot plots
python sage-plot.py --param-file=/path/to/sage_params.par --plots=stellar_mass_function,gas_mass_function

# Generate evolution plots
python sage-plot.py --param-file=/path/to/sage_params.par --evolution

# Specify file range and output options
python sage-plot.py --param-file=/path/to/sage_params.par --first-file=0 --last-file=7 --output-dir=my_plots --format=.pdf

# Specify snapshot number and don't use LaTeX
python sage-plot.py --param-file=/path/to/sage_params.par --snapshot=63 --no-tex
```

### Command-Line Options

```
--param-file=<file>    SAGE parameter file (required)
--first-file=<num>     First file to read [default: 0]
--last-file=<num>      Last file to read [default: use MaxFileNum from param file]
--snapshot=<num>       Process only this snapshot number
--all-snapshots        Process all available snapshots
--evolution            Generate evolution plots
--snapshot-plots       Generate snapshot plots [default]
--output-dir=<dir>     Output directory for plots [default: ./plots]
--format=<format>      Output format (.png, .pdf) [default: .png]
--plots=<list>         Comma-separated list of plots to generate [default: all]
--use-tex              Use LaTeX for text rendering (not recommended)
--verbose              Show detailed output
--help                 Show this help message
```

## Available Plots

### Snapshot Plots (Single Redshift)

#### Galaxy Properties:
- `stellar_mass_function`: Stellar mass function with comparison to observations
- `baryonic_mass_function`: Baryonic (stars + cold gas) mass function
- `gas_mass_function`: Cold gas mass function
- `baryonic_tully_fisher`: Baryonic Tully-Fisher relation
- `specific_sfr`: Specific star formation rate vs. stellar mass
- `black_hole_bulge_relation`: Black hole mass vs. bulge mass relation
- `gas_fraction`: Gas fraction vs. stellar mass
- `metallicity`: Gas-phase metallicity vs. stellar mass
- `bulge_mass_fraction`: Bulge mass fraction vs. stellar mass
- `quiescent_fraction`: Fraction of quiescent galaxies vs. stellar mass

#### Halo Properties:
- `halo_occupation`: Halo occupation distribution
- `baryon_fraction`: Baryon fraction vs. halo mass
- `mass_reservoir_scatter`: Mass in different components vs. halo mass

#### Kinematic Properties:
- `spin_distribution`: Distribution of galaxy spin parameters
- `velocity_distribution`: Distribution of galaxy velocities 

#### Spatial Properties:
- `spatial_distribution`: Spatial distribution of galaxies in the simulation volume

### Evolution Plots (Multiple Redshifts)

- `smf_evolution`: Evolution of the stellar mass function
- `sfr_density_evolution`: Cosmic star formation rate density vs. redshift
- `stellar_mass_density_evolution`: Stellar mass density vs. redshift

## Adding New Plot Types

To add a new plot type, follow these steps:

1. **Create a new Python module** in the `figures/` directory with a descriptive name (e.g., `new_plot_type.py`)

2. **Implement the plot function** with the appropriate signature:
   
   For snapshot plots:
   ```python
   def plot(galaxies, volume, metadata, params, output_dir="plots", output_format=".png"):
       """
       Create your new plot type.
       
       Args:
           galaxies: Galaxy data as a numpy recarray
           volume: Simulation volume in (Mpc/h)^3
           metadata: Dictionary with additional metadata
           params: Dictionary with SAGE parameters
           output_dir: Output directory for the plot
           output_format: File format for the output
       
       Returns:
           Path to the saved plot file
       """
       # Your implementation here
   ```
   
   For evolution plots:
   ```python
   def plot(snapshots, params, output_dir="plots", output_format=".png"):
       """
       Create your new evolution plot type.
       
       Args:
           snapshots: Dictionary mapping snapshot numbers to tuples of (galaxies, volume, metadata)
           params: Dictionary with SAGE parameters
           output_dir: Output directory for the plot
           output_format: File format for the output
       
       Returns:
           Path to the saved plot file
       """
       # Your implementation here
   ```

3. **Use consistent styling** by importing and using helper functions from the `figures` package:
   ```python
   from figures import setup_plot_fonts, setup_legend, get_stellar_mass_label, AXIS_LABEL_SIZE
   ```

4. **Add robust error handling** for empty selections, missing data, and division by zero.

5. **Update `figures/__init__.py`** to include your new module:
   ```python
   # Add import
   from . import new_plot_type
   
   # Add to the appropriate list
   SNAPSHOT_PLOTS.append('new_plot_type')
   # or
   EVOLUTION_PLOTS.append('new_plot_type')
   
   # Add to the mapping
   PLOT_FUNCS['new_plot_type'] = new_plot_type.plot
   ```

6. **Test your plot** with the central script:
   ```bash
   python sage-plot.py --param-file=params.par --plots=new_plot_type --verbose
   ```

### Example Implementation

Here's a minimal example of a new plot module:

```python
#!/usr/bin/env python

"""
SAGE Example Plot

This module generates an example plot from SAGE galaxy data.
"""

import os
import numpy as np
import matplotlib.pyplot as plt
from figures import setup_plot_fonts, setup_legend, get_stellar_mass_label, AXIS_LABEL_SIZE

def plot(galaxies, volume, metadata, params, output_dir="plots", output_format=".png"):
    """
    Create an example plot.
    
    Args:
        galaxies: Galaxy data as a numpy recarray
        volume: Simulation volume in (Mpc/h)^3
        metadata: Dictionary with additional metadata
        params: Dictionary with SAGE parameters
        output_dir: Output directory for the plot
        output_format: File format for the output
    
    Returns:
        Path to the saved plot file
    """
    # Set up the figure
    fig, ax = plt.subplots(figsize=(8, 6))
    
    # Apply consistent font settings
    setup_plot_fonts(ax)
    
    # Your plotting code here...
    
    # Save the figure
    os.makedirs(output_dir, exist_ok=True)
    output_path = os.path.join(output_dir, f"ExamplePlot{output_format}")
    plt.savefig(output_path)
    plt.close()
    
    return output_path
```

## Architecture

The plotting system is organized around these key components:

1. **Master Script (`sage-plot.py`)**: The central entry point that handles:
   - Command-line argument parsing
   - Parameter file reading
   - Data loading
   - Figure generation coordination
   
2. **Snapshot-Redshift Mapper**: Handles mapping between snapshot numbers and redshifts

3. **Figure Modules**: Self-contained modules in the `figures/` directory:
   - Each implements a specific plot type
   - All have consistent interfaces
   - Can be easily extended with new types

## Requirements

- Python 3.x
- NumPy
- Matplotlib (>=3.0.0)
- tqdm (for progress bars)
- (Optional) LaTeX installation for high-quality text rendering in plots

## License

This tool is part of the SAGE galaxy formation model. Please see the main SAGE license for details.
