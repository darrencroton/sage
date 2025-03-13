# Semi-Analytic Galaxy Evolution (SAGE)

[![DOI](https://zenodo.org/badge/13542/darrencroton/sage.svg)](https://zenodo.org/badge/latestdoi/13542/darrencroton/sage)

SAGE is a modern, modular, and customizable semi-analytic model for simulating galaxy formation and evolution in a cosmological context. It provides a computationally efficient framework for tracking the hierarchical growth of dark matter halos, gas cooling, star formation, supernova feedback, black hole growth, active galactic nuclei feedback, and other key astrophysical processes.

## Features

- **Comprehensive Physics**: Implements state-of-the-art models for galaxy formation processes
- **Computational Efficiency**: Processes large cosmological simulations on modest hardware
- **Flexible Input**: Works with multiple N-body simulation formats (binary, HDF5)
- **Modular Design**: Easily extensible for implementing new physical models
- **Robust Memory Management**: Optimized for handling large merger trees
- **Consistent Error Handling**: Comprehensive logging and error reporting
- **Integrated Visualization**: Dedicated plotting system for analyzing model outputs
- **Well-Structured Code**: Organized headers and reduced global variable dependencies
- **Numerical Stability**: Enhanced handling of floating-point operations
- **Cross-Platform I/O**: Improved error checking and platform compatibility
- **Extensive Documentation**: Detailed inline documentation of algorithms and implementation

## Installation

### Requirements

- C compiler (gcc or compatible)
- GNU Make
- (Optional) HDF5 libraries for HDF5 tree format support
- (Optional) clang-format for code formatting
- (Optional) black and isort for Python code formatting

### Building SAGE

```bash
# Clone the repository
git clone https://github.com/darrencroton/sage.git
cd sage

# Basic compilation
make

# With HDF5 support (optional)
make USE-HDF5=yes
```

## Basic Usage

### Running a Simulation

```bash
# Basic execution
./sage <parameter_file>

# With command-line options
./sage --verbose <parameter_file>

# Show help
./sage --help
```

### Command-Line Options

- `--verbose`: Show debug messages
- `--quiet`: Show only warnings and errors
- `--overwrite`: Overwrite existing output files
- `--help`: Display help message

### Parameter File Example

```
%------------------------------------------
%----- SAGE parameter file -----------------
%------------------------------------------

FileNameGalaxies         SA
OutputDir                output/results

FirstFile                0
LastFile                 7

NumSimulationTreeFiles   8

%------------------------------------------
%----- Simulation input options ------------
%------------------------------------------

TreeName                 trees_063
TreeExtension            .0
SimulationDir            input/treefiles

LastSnapShot             63
FirstSnapShot            0
BoxSize                  100.0

%------------------------------------------
%----- Cosmological values -----------------
%------------------------------------------

Omega_m                  0.25 
Omega_b                  0.045
Omega_lambda             0.75
PartMass                 1.0e9
Hubble_h                 0.73

%------------------------------------------
%----- Galaxy formation model options ------
%------------------------------------------

SFprescription           0
ReionizationOn           1
SupernovaRecipeOn        1
DiskInstabilityOn        1
AGNrecipeOn              1

% ... (additional parameters)
```

Note: A full example parameter file can be found in the `input` directory.

## Code Structure

The SAGE codebase is organized around these key components:

- **Core Files**: Main model framework and execution flow
  - `main.c`: Program entry point and core execution
  - `core_build_model.c`: Galaxy construction and evolution
  - `core_init.c`: Initialization routines
  - `core_read_parameter_file.c`: Parameter handling
  
- **Model Files**: Physical processes implementation
  - `model_cooling_heating.c`: Gas cooling and heating processes
  - `model_disk_instability.c`: Disk instability model
  - `model_infall.c`: Gas infall calculations
  - `model_mergers.c`: Galaxy mergers handling
  - `model_reincorporation.c`: Gas reincorporation
  - `model_starformation_and_feedback.c`: Star formation and feedback
  
- **I/O Files**: Input/output operations
  - `io_tree.c`: Tree loading and management
  - `io_save_binary.c`: Binary output format
  - `io_save_hdf5.c`: HDF5 output format (if enabled)
  
- **Utility Files**: Helper functions
  - `util_numeric.c`: Numerical stability utilities
  - `util_error.c`: Error handling system
  - `util_memory.c`: Memory management
  - `util_parameters.c`: Parameter processing
  
- **Header Files**: Declarations and configurations
  - `constants.h`: Physical and numerical constants
  - `types.h`: Structure definitions
  - `globals.h`: Global variable declarations
  - `config.h`: Configuration parameters

## Code Formatting

SAGE includes a code formatting script to maintain consistent coding style:

```bash
# Format all code (C and Python)
./beautify.sh

# Format only C code
./beautify.sh --c-only

# Format only Python code
./beautify.sh --py-only

# See more options
./beautify.sh --help
```

## Visualization System

SAGE includes a comprehensive plotting system (`sage-plot`) for analyzing model outputs. Located in `output/sage-plot/`, this tool provides:

- 19 different plot types covering galaxy properties and evolution metrics
- Consistent styling and interfaces across all visualizations
- A single entry point for generating all plots
- Robust error handling and fallback mechanisms

For detailed usage instructions, see the [sage-plot README](output/sage-plot/README.md).

Basic usage:
```bash
# Generate all plots
cd output/sage-plot
python sage-plot.py --param-file=/path/to/sage_params.par

# Generate specific plots
python sage-plot.py --param-file=/path/to/sage_params.par --plots=stellar_mass_function,gas_fraction
```

## Sample Data

For testing purposes, treefiles for the [mini-Millennium Simulation](http://arxiv.org/abs/astro-ph/0504097) are available [here](https://data-portal.hpc.swin.edu.au/dataset/mini-millennium-simulation).

## Citations

If you use SAGE in your research, please cite:

- [Croton et al. (2016)](http://arxiv.org/abs/1601.04709) - The current SAGE model
- [Croton et al. (2006)](http://arxiv.org/abs/astro-ph/0508046) - The original model

You can also find SAGE on [ascl.net](http://ascl.net/1601.006).

## Related Resources

- [Theoretical Astrophysical Observatory (TAO)](https://tao.asvo.org.au/) - Hosts pre-computed SAGE models on various simulations
- [SAGE Calibration Notebook](https://github.com/darrencroton/sage/blob/master/output/SAGE_MM.ipynb) - Explore key calibration figures

## Contributing

Contributions to SAGE are welcome! Please consider:

1. **Bug Reports**: File issues for any bugs or unexpected behavior
2. **Feature Requests**: Suggest enhancements or new physical models
3. **Pull Requests**: Submit code improvements or documentation updates

## License

SAGE is available under an open-source license. See the LICENSE file for details.

## Contact

Questions and comments can be sent to Darren Croton: dcroton@swin.edu.au.

Visit Darren's homepage at https://darrencroton.github.io for more information on SAGE and related projects.
