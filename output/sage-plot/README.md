# SAGE Plotting Tool

A centralized plotting tool for the Semi-Analytic Galaxy Evolution (SAGE) model.

## Overview

This tool provides a single entry point for generating plots from SAGE galaxy formation model outputs. It features:

- A centralized `sage-plot.py` script that handles command-line arguments, parameter parsing, and plot management
- Self-contained figure modules in the `figures/` directory, each implementing a specific plot type
- Support for both snapshot plots (single snapshots) and evolution plots (across multiple snapshots)
- Integration with SAGE parameter files for consistent configuration
- Customizable output formatting and figure selection

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
--no-tex               Don't use LaTeX for text rendering
--verbose              Show detailed output
--help                 Show this help message
```

## Available Plots

### Snapshot Plots (Single Redshift)

- `stellar_mass_function`: Stellar mass function with comparison to observations
- `baryonic_mass_function`: Baryonic (stars + cold gas) mass function
- `gas_mass_function`: Cold gas mass function
- `baryonic_tully_fisher`: Baryonic Tully-Fisher relation
- `specific_sfr`: Specific star formation rate vs. stellar mass

### Evolution Plots (Multiple Redshifts)

- `smf_evolution`: Evolution of the stellar mass function
- `sfr_density_evolution`: Cosmic star formation rate density vs. redshift
- `stellar_mass_density_evolution`: Stellar mass density vs. redshift

## Adding New Plot Types

To add a new plot type:

1. Create a new Python module in the `figures/` directory
2. Implement a `plot()` function with the appropriate signature
3. Add the module to the imports in `figures/__init__.py`
4. Update the appropriate list (`SNAPSHOT_PLOTS` or `EVOLUTION_PLOTS`) in `__init__.py`
5. Add the plot function to `PLOT_FUNCS` mapping in `__init__.py`

## Requirements

- Python 3.x
- NumPy
- Matplotlib
- (Optional) LaTeX installation for high-quality text rendering in plots

## Examples

The resulting plots are saved to the specified output directory (default: `./plots`). For example:

- `StellarMassFunction.png`
- `BaryonicTullyFisher.png`
- `SFR_Density_Evolution.png`

## License

This tool is part of the SAGE galaxy formation model. Please see the main SAGE license for details.
