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

## Quick Start

For new users who want to get SAGE up and running immediately:

```bash
# Clone the repository
git clone https://github.com/darrencroton/sage.git
cd sage

# Run the automated setup script
./first_run.sh

# Compile SAGE
make

# Run SAGE with the mini-Millennium simulation
./sage input/millennium.par

# Generate plots (using virtual environment)
source sage_venv/bin/activate
cd output/sage-plot
python sage-plot.py --param-file=../../input/millennium.par
deactivate
```

The `first_run.sh` script will automatically:
- Create necessary directories (`input/data/millennium`, `output/results/millennium`)
- Download the mini-Millennium simulation trees
- Set up a Python virtual environment (`sage_venv`) with plotting dependencies
- Configure the parameter file with correct paths

## Installation

### Requirements

- C compiler (gcc or compatible)
- GNU Make
- Python 3.x (for plotting)
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

### Manual Setup (Alternative to first_run.sh)

If you prefer to set up SAGE manually or the automated script doesn't work for your system:

#### 1. Create Directory Structure
```bash
mkdir -p input/data/millennium
mkdir -p output/results/millennium
```

#### 2. Download Simulation Data
Download the mini-Millennium simulation trees:
```bash
cd input/data/millennium

# Using wget
wget "https://www.dropbox.com/s/l5ukpo7ar3rgxo4/mini-millennium-treefiles.tar?dl=0" -O mini-millennium-treefiles.tar

# Or using curl
curl -L -o mini-millennium-treefiles.tar "https://www.dropbox.com/s/l5ukpo7ar3rgxo4/mini-millennium-treefiles.tar?dl=0"

# Extract the files
tar -xf mini-millennium-treefiles.tar
rm mini-millennium-treefiles.tar

cd ../../..
```

#### 3. Set Up Python Environment
Install required Python packages for plotting using the provided `requirements.txt`. Due to modern Python environment management, you have several options:

**Option 1 (Recommended): Use a virtual environment:**
```bash
python3 -m venv sage_venv
source sage_venv/bin/activate
pip install -r requirements.txt
```

**Option 2: Use --user flag:**
```bash
pip3 install --user -r requirements.txt
```

**Option 3: Use system package manager (macOS with Homebrew):**
```bash
brew install python-numpy python-matplotlib python-tqdm
```

**Option 4: Override system protection (not recommended):**
```bash
pip3 install --break-system-packages -r requirements.txt
```

#### 4. Configure Parameter File
Update `input/millennium.par` with the correct absolute paths:
- Set `OutputDir` to your full path + `/output/results/millennium/`
- Set `SimulationDir` to your full path + `/input/data/millennium/`
- Set `FileWithSnapList` to your full path + `/input/data/millennium/millennium.a_list`

#### 5. Compile and Run
```bash
make
./sage input/millennium.par
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
%----- SAGE output file information -------
%------------------------------------------

FileNameGalaxies       model 
OutputDir              /path/to/output/directory/

FirstFile              0
LastFile               7

%------------------------------------------
%----- Snapshot output list ---------------
%------------------------------------------

NumOutputs             8   

% List of output snapshots
-> 63 37 32 27 23 20 18 16

%------------------------------------------
%----- Simulation information  ------------
%------------------------------------------

TreeName               trees_063
TreeType               lhalo_binary
SimulationDir          /path/to/simulation/data/
FileWithSnapList       /path/to/snapshot/list
LastSnapShotNr         63
NumSimulationTreeFiles 8
BoxSize                62.5  

Omega                  0.25 
OmegaLambda            0.75
BaryonFrac             0.17
Hubble_h               0.73
PartMass               0.0860657

%------------------------------------------
%----- SAGE recipe options ----------------
%------------------------------------------

SFprescription         0
AGNrecipeOn            2
SupernovaRecipeOn      1
ReionizationOn         1
DiskInstabilityOn      1

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
# Activate the virtual environment first
source sage_venv/bin/activate

# Generate all plots
cd output/sage-plot
python sage-plot.py --param-file=/path/to/sage_params.par

# Generate specific plots
python sage-plot.py --param-file=/path/to/sage_params.par --plots=stellar_mass_function,gas_fraction

# Deactivate when done
deactivate
```

## Sample Data

For testing purposes, treefiles for the [mini-Millennium Simulation](http://arxiv.org/abs/astro-ph/0504097) are automatically downloaded and configured by the `first_run.sh` script. You can also manually download them from [here](https://data-portal.hpc.swin.edu.au/dataset/mini-millennium-simulation).

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
