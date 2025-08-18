#!/usr/bin/env python

"""
SAGE Plotting Tool - Master plotting script for SAGE galaxy formation model output

Usage:
  python sage-plot-fixed.py --param-file=<param_file> [options]

Options:
  --param-file=<file>    SAGE parameter file (required)
  --first-file=<num>     First file to read [default: 0]
  --last-file=<num>      Last file to read [default: use MaxFileNum from param file]
  --snapshot=<num>       Process only this snapshot number
  --all-snapshots        Process all available snapshots
  --evolution            Generate evolution plots
  --snapshot-plots       Generate snapshot plots [default]
  --output-dir=<dir>     Output directory for plots [default: ./plots]
  --format=<format>      Output format (.png, .pdf) [default: .png]
  --plots=<list>         Comma-separated list of plots to generate
                         [default: all available plots]
  --use-tex              Use LaTeX for text rendering (not recommended)
  --verbose              Show detailed output
  --help                 Show this help message
"""

import argparse
import glob
import importlib
import os
import random
import re  # Required for regular expressions in file pattern matching
import sys
import time
from collections import OrderedDict

import matplotlib
import matplotlib.pyplot as plt
import numpy as np
from tqdm import tqdm

random.seed(42)  # For reproducibility with sample data

# Import figure modules
from figures import *

# Import the SnapshotRedshiftMapper
from snapshot_redshift_mapper import SnapshotRedshiftMapper


# Galaxy data structure definition
def get_galaxy_dtype():
    """Return the NumPy dtype for SAGE galaxy data."""
    galdesc_full = [
        ("SnapNum", np.int32),
        ("Type", np.int32),
        ("GalaxyIndex", np.int64),
        ("CentralGalaxyIndex", np.int64),
        ("SAGEHaloIndex", np.int32),
        ("SAGETreeIndex", np.int32),
        ("SimulationHaloIndex", np.int64),
        ("mergeType", np.int32),
        ("mergeIntoID", np.int32),
        ("mergeIntoSnapNum", np.int32),
        ("dT", np.float32),
        ("Pos", (np.float32, 3)),
        ("Vel", (np.float32, 3)),
        ("Spin", (np.float32, 3)),
        ("Len", np.int32),
        ("Mvir", np.float32),
        ("CentralMvir", np.float32),
        ("Rvir", np.float32),
        ("Vvir", np.float32),
        ("Vmax", np.float32),
        ("VelDisp", np.float32),
        ("ColdGas", np.float32),
        ("StellarMass", np.float32),
        ("BulgeMass", np.float32),
        ("HotGas", np.float32),
        ("EjectedMass", np.float32),
        ("BlackHoleMass", np.float32),
        ("IntraClusterStars", np.float32),
        ("MetalsColdGas", np.float32),
        ("MetalsStellarMass", np.float32),
        ("MetalsBulgeMass", np.float32),
        ("MetalsHotGas", np.float32),
        ("MetalsEjectedMass", np.float32),
        ("MetalsIntraClusterStars", np.float32),
        ("SfrDisk", np.float32),
        ("SfrBulge", np.float32),
        ("SfrDiskZ", np.float32),
        ("SfrBulgeZ", np.float32),
        ("DiskRadius", np.float32),
        ("Cooling", np.float32),
        ("Heating", np.float32),
        ("QuasarModeBHaccretionMass", np.float32),
        ("TimeOfLastMajorMerger", np.float32),
        ("TimeOfLastMinorMerger", np.float32),
        ("OutflowRate", np.float32),
        ("infallMvir", np.float32),
        ("infallVvir", np.float32),
        ("infallVmax", np.float32),
    ]
    names = [galdesc_full[i][0] for i in range(len(galdesc_full))]
    formats = [galdesc_full[i][1] for i in range(len(galdesc_full))]
    return np.dtype({"names": names, "formats": formats}, align=True)


class SAGEParameters:
    """Class to parse and store SAGE parameter file settings."""

    def __init__(self, param_file):
        """Initialize with parameter file path."""
        self.param_file = os.path.abspath(param_file)
        self.param_dir = os.path.dirname(self.param_file)
        self.params = {}
        self.parse_param_file()

    def parse_param_file(self):
        """Parse the SAGE parameter file."""
        if not os.path.exists(self.param_file):
            raise FileNotFoundError(f"Parameter file not found: {self.param_file}")

        # Check for snapshot output list line
        output_snapshots = []

        # Parse the parameter file
        with open(self.param_file, "r") as f:
            for line in f:
                # Skip empty lines and full comment lines
                if (
                    line.strip() == ""
                    or line.strip().startswith("#")
                    or line.strip().startswith("%")
                ):
                    continue

                # Check for arrow notation for snapshots (e.g., "-> 63 37 32 27 23 20 18 16")
                if "->" in line:
                    snapshot_list = line.split("->")[1].strip().split()
                    output_snapshots = [int(snap) for snap in snapshot_list]
                    self.params["OutputSnapshots"] = output_snapshots
                    continue

                # Parse key-value pairs
                if "=" in line:
                    # Standard equals-separated key-value
                    parts = line.split("=")
                    key = parts[0].strip()
                    value_part = parts[1].strip()
                else:
                    # Handle space-separated key-value pairs (common in parameter files)
                    parts = line.split(None, 1)  # Split on whitespace, max 1 split
                    if len(parts) >= 2:
                        key = parts[0].strip()
                        value_part = parts[1].strip()
                    else:
                        continue  # Skip lines that don't match our format

                # Handle inline comments
                if ";" in value_part:
                    value = value_part.split(";")[0].strip()
                elif "#" in value_part:
                    value = value_part.split("#")[0].strip()
                else:
                    value = value_part

                # Clean the value - especially important for paths
                value = value.strip()

                # Convert to appropriate type
                if value.isdigit():
                    value = int(value)
                elif self._is_float(value):
                    value = float(value)
                elif key in ["OutputDir", "SimulationDir"]:
                    # Resolve path relative to parameter file directory
                    value = self._resolve_path(value)
                    # Make sure directory paths have a trailing slash
                    if value and not value.endswith("/"):
                        value = value + "/"
                elif key in ["FileWithSnapList"]:
                    # Resolve path relative to parameter file directory
                    value = self._resolve_path(value)
                    # Don't add trailing slash to file paths

                self.params[key] = value

    def _is_float(self, value):
        """Check if a string can be converted to float."""
        try:
            float(value)
            return True
        except ValueError:
            return False
    
    def _resolve_path(self, path):
        """
        Resolve a path from the parameter file relative to the SAGE root directory.
        
        For SAGE parameter files, relative paths are typically relative to the
        directory containing the SAGE executable and source code (the root project directory),
        not relative to the parameter file itself.
        
        Args:
            path: Path string from parameter file
            
        Returns:
            Absolute path
        """
        if not path:
            return path
            
        # Clean the path first
        path = path.strip().strip('"').strip("'")
        
        # If it's already absolute, return as-is
        if os.path.isabs(path):
            return path
            
        # For relative paths, we need to find the SAGE root directory
        # Look for common indicators (SAGE executable, Makefile, code/ directory)
        current_dir = self.param_dir
        sage_root = None
        
        # Search up from the parameter file directory to find SAGE root
        while current_dir != "/" and current_dir != os.path.dirname(current_dir):
            if (os.path.exists(os.path.join(current_dir, "Makefile")) and 
                os.path.exists(os.path.join(current_dir, "code")) and
                (os.path.exists(os.path.join(current_dir, "sage")) or 
                 os.path.exists(os.path.join(current_dir, "first_run.sh")))):
                sage_root = current_dir
                break
            current_dir = os.path.dirname(current_dir)
        
        # If we couldn't find SAGE root, fall back to parameter file directory
        if sage_root is None:
            sage_root = self.param_dir
            
        # Make the path relative to SAGE root
        return os.path.abspath(os.path.join(sage_root, path))

    def get(self, key, default=None):
        """Get a parameter value."""
        return self.params.get(key, default)

    def __getitem__(self, key):
        """Allow dictionary-like access to parameters."""
        return self.params[key]

    def __contains__(self, key):
        """Check if a parameter exists."""
        return key in self.params


def setup_matplotlib(use_tex=False):
    """Set up matplotlib with standard settings."""
    matplotlib.rcdefaults()
    plt.rc("xtick", labelsize="x-large")
    plt.rc("ytick", labelsize="x-large")
    plt.rc("lines", linewidth="2.0")
    plt.rc("legend", numpoints=1, fontsize="x-large")

    # Only use LaTeX if explicitly requested
    if use_tex:
        try:
            plt.rc("text", usetex=True)
            print("LaTeX rendering enabled for text")
        except Exception as e:
            print(f"Warning: Could not enable LaTeX: {e}")
            # Fall back to regular text rendering
            plt.rc("text", usetex=False)
    else:
        # Explicitly disable LaTeX
        plt.rc("text", usetex=False)

    # Set up nice math rendering even without LaTeX
    plt.rcParams["mathtext.fontset"] = "dejavusans"
    plt.rcParams["mathtext.default"] = "regular"


def read_galaxies(model_path, first_file, last_file, params=None):
    """
    Read galaxy data from SAGE output files.

    Args:
        model_path: Path to model files
        first_file: First file number to read
        last_file: Last file number to read
        params: Dictionary with SAGE parameters

    Returns:
        Tuple containing:
            - Numpy recarray of galaxy data
            - Volume of the simulation
            - Dictionary of metadata
    """
    # This is important information, so show regardless of verbose flag
    print(f"Reading galaxy data from {model_path}")
    """
    Read galaxy data from SAGE output files.
    
    Args:
        model_path: Path to model files
        first_file: First file number to read
        last_file: Last file number to read
        params: Dictionary with SAGE parameters
    
    Returns:
        Tuple containing:
            - Numpy recarray of galaxy data
            - Volume of the simulation
            - Dictionary of metadata
    """
    # Get required parameters from the parameter file
    if not params:
        print("Error: Parameter dictionary is required.")
        sys.exit(1)
        
    # Ensure required parameters exist
    required_params = ["Hubble_h", "BoxSize"]
    missing_params = [p for p in required_params if p not in params]
    if missing_params:
        print(f"Error: Required parameters missing from parameter file: {', '.join(missing_params)}")
        sys.exit(1)
        
    hubble_h = params["Hubble_h"]
    box_size = params["BoxSize"]
    
    # For volume calculation, we'll use the number of good files read
    # No need for MaxTreeFiles parameter - we'll calculate based on actual files read

    # Print the model path for debugging
    if args.verbose:
        print(f"Looking for galaxy files with base: {model_path}")

    # Look for files matching the pattern in the same directory
    dir_path = os.path.dirname(model_path)
    base_name = os.path.basename(model_path)

    # First try exact file number pattern (model_z0.000_0, model_z0.000_1, etc.)
    pattern1 = f"{model_path}_{first_file}"

    # Then try generic pattern (model_z0.000_*)
    pattern2 = os.path.join(dir_path, f"{base_name}_*")

    # Log the patterns we're trying
    if args.verbose:
        print(f"  Trying exact pattern: {pattern1}")
        print(f"  Trying generic pattern: {pattern2}")

    # Try the exact pattern first
    exact_files = glob.glob(pattern1)
    if exact_files:
        existing_files = exact_files
        if args.verbose:
            print(f"  Found file with exact pattern")
    else:
        # Fall back to the generic pattern
        existing_files = glob.glob(pattern2)

    if existing_files:
        if args.verbose:
            print(f"Found {len(existing_files)} files matching the pattern.")
            for f in existing_files[:5]:  # Show first 5 files
                print(f"  {f}")
            if len(existing_files) > 5:
                print(f"  ... and {len(existing_files) - 5} more")
    else:
        print(f"No files found matching the pattern {base_name}_*")

    # Get the galaxy data dtype
    galdesc = get_galaxy_dtype()

    # Initialize variables
    tot_ntrees = 0
    tot_ngals = 0
    good_files = 0

    if args.verbose:
        print(
            f"Determining storage requirements for files {first_file} to {last_file}..."
        )

    # First pass: Determine total number of galaxies
    # Only show progress bar when verbose is enabled
    file_iterator = (
        tqdm(range(first_file, last_file + 1), desc="Counting galaxies")
        if args.verbose
        else range(first_file, last_file + 1)
    )
    for fnr in file_iterator:
        fname = f"{model_path}_{fnr}"

        if not os.path.isfile(fname):
            continue

        if os.path.getsize(fname) == 0:
            print(f"File {fname} is empty! Skipping...")
            continue

        try:
            with open(fname, "rb") as fin:
                ntrees = np.fromfile(fin, np.dtype(np.int32), 1)[
                    0
                ]  # Extract scalar value
                ntotgals = np.fromfile(fin, np.dtype(np.int32), 1)[0]
                tot_ntrees += ntrees
                tot_ngals += ntotgals
                good_files += 1
        except Exception as e:
            print(f"Error reading file {fname}: {e}")
            continue

    print(f"Input files contain: {tot_ntrees} trees, {tot_ngals} galaxies.")

    # Check if we found any galaxies
    if tot_ngals == 0:
        print("Error: No galaxies found in the model files.")
        print(f"Please check that the model files exist and are not empty.")
        sys.exit(1)

    # Initialize the storage array
    galaxies = np.empty(tot_ngals, dtype=galdesc)

    # Second pass: Read the galaxy data
    offset = 0
    # Only show progress bar when verbose is enabled
    file_iterator = (
        tqdm(range(first_file, last_file + 1), desc="Reading galaxies")
        if args.verbose
        else range(first_file, last_file + 1)
    )
    for fnr in file_iterator:
        fname = f"{model_path}_{fnr}"

        if not os.path.isfile(fname) or os.path.getsize(fname) == 0:
            continue

        try:
            with open(fname, "rb") as fin:
                ntrees = np.fromfile(fin, np.dtype(np.int32), 1)[
                    0
                ]  # Extract scalar value
                ntotgals = np.fromfile(fin, np.dtype(np.int32), 1)[0]
                gals_per_tree = np.fromfile(fin, np.dtype((np.int32, ntrees)), 1)

                print(f"Reading {ntotgals} galaxies from file: {fname}")

                gg = np.fromfile(fin, galdesc, ntotgals)

                # Slice the file array into the global array with a copy
                galaxies[offset : offset + ntotgals] = gg[0:ntotgals].copy()

                offset += ntotgals
        except Exception as e:
            print(f"Error reading file {fname}: {e}")
            continue

    # Convert to recarray for attribute access
    galaxies = galaxies.view(np.recarray)

    # Calculate the volume based on the box size and the number of good files read
    # Volume is the box size cubed, scaled by the fraction of files actually read
    # This assumes files are distributed uniformly across the simulation volume
    volume = box_size**3.0
    
    # If we have information about first/last file and good files, adjust volume
    if "FirstFile" in params and "LastFile" in params:
        total_files = params["NumSimulationTreeFiles"]
        if total_files > 0 and good_files > 0:
            volume = volume * good_files / total_files
            if args.verbose:
                print(f"  Volume fraction: {good_files}/{total_files} = {good_files/total_files:.4f}")
                print(f"  Adjusted volume: {volume:.2f} (Mpc/h)³")

    # Create metadata dictionary
    metadata = {
        "hubble_h": hubble_h,
        "box_size": box_size,
        "volume": volume,
        "ntrees": tot_ntrees,
        "ngals": tot_ngals,
        "good_files": good_files,
    }

    return galaxies, volume, metadata


def parse_arguments():
    """Parse command-line arguments."""
    parser = argparse.ArgumentParser(description="SAGE Plotting Tool")
    parser.add_argument("--param-file", required=True, help="SAGE parameter file (required)")
    parser.add_argument("--first-file", type=int, help="First file to read (overrides parameter file)")
    parser.add_argument("--last-file", type=int, help="Last file to read (overrides parameter file)")
    parser.add_argument(
        "--snapshot", type=int, help="Process only this snapshot number (overrides parameter file)"
    )
    parser.add_argument(
        "--all-snapshots", action="store_true", help="Process all available snapshots"
    )
    parser.add_argument(
        "--evolution", action="store_true", help="Generate evolution plots"
    )
    parser.add_argument(
        "--snapshot-plots", action="store_true", help="Generate snapshot plots"
    )
    parser.add_argument(
        "--output-dir",
        help="DEPRECATED - Output directory is always <OutputDir>/plots",
    )
    parser.add_argument("--format", default=".png", help="Output format (.png, .pdf)")
    parser.add_argument(
        "--plots", help="Comma-separated list of plots to generate (default: all available plots)"
    )
    parser.add_argument(
        "--use-tex",
        action="store_true",
        help="Use LaTeX for text rendering (not recommended)",
    )
    parser.add_argument("--verbose", action="store_true", help="Show detailed output")

    args = parser.parse_args()

    # Default to snapshot plots if neither is specified
    if not args.evolution and not args.snapshot_plots:
        args.snapshot_plots = True
        
    # Default to all plots if not specified
    if args.plots is None:
        args.plots = "all"

    return args


def get_available_plot_modules(plot_type):
    """
    Get available plot modules of a specific type.

    Args:
        plot_type: 'snapshot' or 'evolution'

    Returns:
        Dictionary mapping plot names to their modules
    """
    modules = {}

    # Get the module patterns from the figures module
    if plot_type == "snapshot":
        module_patterns = SNAPSHOT_PLOTS
    else:  # 'evolution'
        module_patterns = EVOLUTION_PLOTS

    # Import modules
    for pattern in module_patterns:
        try:
            module_name = f"figures.{pattern}"
            module = importlib.import_module(module_name)
            # Get the main plotting function from the module
            plot_func = getattr(module, "plot", None)
            if plot_func:
                modules[pattern] = plot_func
        except (ImportError, AttributeError) as e:
            if args.verbose:
                print(f"Warning: Could not import {module_name}: {e}")

    return modules


def main():
    """Main execution function."""
    global args
    args = parse_arguments()

    # Parse the parameter file
    try:
        params = SAGEParameters(args.param_file)
        if args.verbose:
            print(f"Loaded parameters from {args.param_file}")

            # Print important parameters
            important_params = [
                "OutputDir",
                "FileNameGalaxies",
                "LastSnapShotNr",
                "FirstFile",
                "LastFile",
                "Hubble_h",
                "WhichIMF",
            ]
            for key in important_params:
                if key in params.params:
                    print(f"  {key} = {params.params[key]}")

        # Print raw params for debugging
        if args.verbose:
            print("\nRaw parameter values:")
            for key, value in params.params.items():
                print(f"  {key} = {value} (type: {type(value)})")
    except Exception as e:
        print(f"Error loading parameter file: {e}")
        sys.exit(1)

    # Verify all required parameters exist
    required_params = [
        "OutputDir", 
        "FileNameGalaxies", 
        "FirstFile", 
        "LastFile",
        "BoxSize",
        "Hubble_h",
        "FileWithSnapList",
        "NumSimulationTreeFiles"
    ]
    
    missing_params = []
    for param in required_params:
        if param not in params.params:
            missing_params.append(param)
    
    if missing_params:
        print(f"Error: Required parameters missing from parameter file: {', '.join(missing_params)}")
        sys.exit(1)
    
    # Verify paths from parameter file exist
    output_dir = params["OutputDir"]
    simulation_dir = params.get("SimulationDir")  # May not be used directly
    file_name_galaxies = params["FileNameGalaxies"]
    file_with_snap_list = params["FileWithSnapList"]

    if args.verbose:
        print(f"Parameter file details:")
        print(f"  OutputDir: {output_dir}")
        print(f"  SimulationDir: {simulation_dir if simulation_dir else 'Not specified'}")
        print(f"  FileNameGalaxies: {file_name_galaxies}")
        print(f"  FirstFile: {params['FirstFile']}")
        print(f"  LastFile: {params['LastFile']}")
        print(f"  FileWithSnapList: {file_with_snap_list}")
        print(f"  BoxSize: {params['BoxSize']}")
        print(f"  Hubble_h: {params['Hubble_h']}")

    # Check if OutputDir exists
    if not os.path.exists(output_dir):
        print(f"Error: OutputDir '{output_dir}' from parameter file does not exist.")
        sys.exit(1)
        
    # Check if FileWithSnapList exists (path is already resolved by SAGEParameters)
    if not os.path.exists(file_with_snap_list):
        print(f"Error: FileWithSnapList '{file_with_snap_list}' not found.")
        print("Please verify the path is correct in the parameter file.")
        sys.exit(1)

    # Set up matplotlib
    setup_matplotlib(args.use_tex)

    # Get output directory from parameter file - required parameter
    if "OutputDir" not in params:
        print("Error: OutputDir parameter is required in the parameter file.")
        sys.exit(1)
    
    # Get the output directory path (already resolved by SAGEParameters)
    model_output_dir = params["OutputDir"]
    
    if args.verbose:
        print(f"\nOutput directory handling:")
        print(f"  model_output_dir from params: '{model_output_dir}'")
    
    # Check if output directory exists
    if not os.path.exists(model_output_dir):
        print(f"Error: OutputDir '{model_output_dir}' specified in parameter file does not exist.")
        sys.exit(1)
    
    # Check if output directory is writable
    if not os.access(model_output_dir, os.W_OK):
        print(f"Error: OutputDir '{model_output_dir}' specified in parameter file is not writable.")
        sys.exit(1)
    
    # Set the plots directory as a subdirectory of the output directory
    output_dir = os.path.join(model_output_dir, "plots")
    
    if args.verbose:
        print(f"  Using output directory: '{output_dir}'")
    
    # Create the plots directory if it doesn't exist
    try:
        os.makedirs(output_dir, exist_ok=True)
        if args.verbose:
            print(f"  Successfully created/verified output directory: {output_dir}")
    except Exception as e:
        print(f"Error: Could not create output directory {output_dir}: {e}")
        sys.exit(1)
    
    # If output_dir argument was provided, inform user we're ignoring it
    if args.output_dir:
        print(f"Note: Ignoring --output-dir argument. Using directory from parameter file: {output_dir}")

    # Determine which plots to generate
    if args.plots == "all":
        selected_plots = None  # All available
    else:
        selected_plots = [p.strip() for p in args.plots.split(",")]

        # Check if any evolution plots are specifically requested but --evolution flag is not set
        requested_evolution_plots = [p for p in selected_plots if p in EVOLUTION_PLOTS]
        if requested_evolution_plots and not args.evolution:
            print(
                f"Warning: Evolution plots requested ({', '.join(requested_evolution_plots)}) but --evolution flag not set."
            )
            print(
                f"These plots require data from multiple snapshots to work correctly."
            )
            print(f"Adding --evolution flag automatically.")
            args.evolution = True

    # Generate snapshot plots
    if args.snapshot_plots:
        # Get required parameters for finding model files
        if "OutputDir" not in params:
            print("Error: OutputDir parameter is required in the parameter file.")
            sys.exit(1)
            
        if "FileNameGalaxies" not in params:
            print("Error: FileNameGalaxies parameter is required in the parameter file.")
            sys.exit(1)
            
        # Get output model path and snapshot number (path already resolved by SAGEParameters)
        model_path = params["OutputDir"]
        snapshot = args.snapshot or params.get("LastSnapShotNr")
        
        if not snapshot:
            print("Error: LastSnapShotNr not found in parameter file and no snapshot specified.")
            sys.exit(1)
        
        # File name from parameter file
        file_name_galaxies = params["FileNameGalaxies"]
        
        if args.verbose:
            print(f"\nModel file discovery:")
            print(f"  model_path from params: '{model_path}'")
            print(f"  file_name_galaxies: '{file_name_galaxies}'")
            print(f"  Using snapshot: {snapshot}")
        
        # Check if model_path exists
        if not os.path.exists(model_path):
            print(f"Error: OutputDir '{model_path}' from parameter file does not exist.")
            sys.exit(1)

        # Get the redshift for this snapshot using the mapper
        mapper = SnapshotRedshiftMapper(args.param_file, params.params, model_path)
        redshift_str = mapper.get_redshift_str(snapshot)
        
        if args.verbose:
            print(f"  Redshift string for snapshot {snapshot}: {redshift_str}")
        
        # Construct the base model file path directly
        base_model_file = os.path.join(model_path, f"{file_name_galaxies}{redshift_str}")
        
        if args.verbose:
            print(f"  Using model file base: {base_model_file}")

        # Required parameters check
        required_params = ["FirstFile", "LastFile"]
        missing_params = [p for p in required_params if p not in params.params]
        if missing_params:
            print(f"Error: Required parameters missing from parameter file: {', '.join(missing_params)}")
            sys.exit(1)
            
        # Get first and last file numbers, prioritizing command-line arguments
        if args.first_file is not None:
            first_file = args.first_file
            if args.verbose:
                print(f"Using first_file={first_file} from command-line argument")
        else:
            first_file = params["FirstFile"]
            if args.verbose:
                print(f"Using first_file={first_file} from parameter file")
                
        if args.last_file is not None:
            last_file = args.last_file
            if args.verbose:
                print(f"Using last_file={last_file} from command-line argument")
        else:
            last_file = params["LastFile"]
            if args.verbose:
                print(f"Using last_file={last_file} from parameter file")
                
        # Validate file range
        if first_file > last_file:
            print(f"Error: FirstFile ({first_file}) is greater than LastFile ({last_file})")
            sys.exit(1)

        # Read galaxy data
        try:
            galaxies, volume, metadata = read_galaxies(
                model_path=base_model_file,
                first_file=first_file,
                last_file=last_file,
                params=params.params,
            )
            if args.verbose:
                if metadata.get("sample_data", False):
                    print(
                        "USING SAMPLE DATA: No real galaxy data was found, using generated test data instead."
                    )
                    print(
                        "This is useful for testing the plotting code but the plots will not reflect real SAGE output."
                    )
                print(
                    f"Read {len(galaxies)} galaxies from volume {volume:.2f} (Mpc/h)³"
                )
        except Exception as e:
            print(f"Error reading galaxy data: {e}")
            sys.exit(1)

        # Get available snapshot plot modules
        plot_modules = get_available_plot_modules("snapshot")

        if args.verbose:
            print(f"Available snapshot plots: {', '.join(plot_modules.keys())}")

        # Filter to selected plots if specified
        if selected_plots:
            plot_modules = {
                k: v for k, v in plot_modules.items() if k in selected_plots
            }

        # Generate each plot
        generated_plots = []
        for plot_name, plot_func in plot_modules.items():
            try:
                if args.verbose:
                    print(f"Generating {plot_name}...")
                plot_path = plot_func(
                    galaxies=galaxies,
                    volume=volume,
                    metadata=metadata,
                    params=params.params,
                    output_dir=output_dir,
                    output_format=args.format,
                    verbose=args.verbose,
                )
                generated_plots.append(plot_path)
                print(f"Generated: {plot_path}")
            except Exception as e:
                print(f"Error generating {plot_name}: {e}")

        if args.verbose:
            print(f"Generated {len(generated_plots)} snapshot plots.")

    # Generate evolution plots
    if args.evolution:
        # Get available evolution plot modules
        plot_modules = get_available_plot_modules("evolution")

        if args.verbose:
            print(f"Available evolution plots: {', '.join(plot_modules.keys())}")

        # Filter to selected plots if specified
        if selected_plots:
            plot_modules = {
                k: v for k, v in plot_modules.items() if k in selected_plots
            }

        # Create a snapshot-to-redshift mapper
        mapper = SnapshotRedshiftMapper(
            args.param_file, params.params, model_output_dir
        )
        if args.verbose:
            print(mapper.debug_info())

        # Create the mapper from parameter file
        mapper = SnapshotRedshiftMapper(args.param_file, params.params, params["OutputDir"])
        
        # Determine which snapshots to process
        if args.all_snapshots:
            # Process all available snapshots
            snapshots = mapper.get_all_snapshots()
            if args.verbose:
                print(f"Using all {len(snapshots)} available snapshots")
        elif args.snapshot:
            # Process only the specified snapshot
            # Verify this snapshot exists in our mapping
            if args.snapshot not in mapper.snapshots:
                print(f"Error: Specified snapshot {args.snapshot} not found in redshift mapping")
                print(f"Available snapshots: {mapper.snapshots}")
                sys.exit(1)
            
            snapshots = [args.snapshot]
            if args.verbose:
                print(f"Using single snapshot: {args.snapshot}")
        else:
            # Use the evolution snapshots determined by the mapper
            # This will prioritize OutputSnapshots from parameter file
            snapshots = mapper.get_evolution_snapshots()
            
            # Check that we have at least 2 snapshots for a meaningful evolution plot
            if len(snapshots) < 2:
                print("Error: At least 2 snapshots are required for evolution plots")
                print(f"Available snapshots: {snapshots}")
                sys.exit(1)
                
            # Check for diverse redshift coverage
            redshifts = [mapper.get_redshift(snap) for snap in snapshots]
            min_z = min(redshifts)
            max_z = max(redshifts)
            
            if args.verbose:
                print(f"Using {len(snapshots)} snapshots for evolution plots")
                print(f"Redshift range: z={min_z:.3f} to z={max_z:.3f}")

        if args.verbose:
            print(f"Selected snapshots for evolution plots: {snapshots}")
            print(
                f"Corresponding redshifts: {[mapper.get_redshift(snap) if snap >= 0 else 0.0 for snap in snapshots]}"
            )

        # Read galaxy data for each snapshot
        snapshot_data = {}
        # Only show progress bar when verbose is enabled
        snapshot_iterator = (
            tqdm(snapshots, desc="Loading snapshot data for evolution plots")
            if args.verbose
            else snapshots
        )
        for snap in snapshot_iterator:
            # Special case for -1 snap code that was used for sample z=0 data
            if snap == -1:
                print("Error: Sample data generation is no longer supported.")
                print("Please ensure all necessary data files exist.")
                sys.exit(1)

            # Regular case - read actual snapshot data
            # Get redshift and model file path from mapper
            redshift = mapper.get_redshift(snap)
            model_file_base = mapper.get_model_file_path(snap, 0).rsplit("_", 1)[
                0
            ]  # Remove file number

            if args.verbose:
                print(f"Processing snapshot {snap} (z={redshift:.3f})")
                print(f"Using model file pattern: {model_file_base}")

            # Required parameters check
            required_params = ["FirstFile", "LastFile", "NumSimulationTreeFiles"]
            missing_params = [p for p in required_params if p not in params.params]
            if missing_params:
                print(f"Error: Required parameters missing from parameter file: {', '.join(missing_params)}")
                sys.exit(1)
                
            # Get first and last file numbers, prioritizing command-line arguments
            if args.first_file is not None:
                first_file = args.first_file
                if args.verbose:
                    print(f"Using first_file={first_file} from command-line argument")
            else:
                first_file = params["FirstFile"]
                if args.verbose:
                    print(f"Using first_file={first_file} from parameter file")
                    
            if args.last_file is not None:
                last_file = args.last_file
                if args.verbose:
                    print(f"Using last_file={last_file} from command-line argument")
            else:
                last_file = params["LastFile"]
                if args.verbose:
                    print(f"Using last_file={last_file} from parameter file")
                    
            # Validate file range
            if first_file > last_file:
                print(f"Error: FirstFile ({first_file}) is greater than LastFile ({last_file})")
                sys.exit(1)

            try:
                galaxies, volume, metadata = read_galaxies(
                    model_path=model_file_base,
                    first_file=first_file,
                    last_file=last_file,
                    params=params.params,
                )
                # Add redshift to metadata
                metadata["redshift"] = redshift
                snapshot_data[snap] = (galaxies, volume, metadata)
                if args.verbose:
                    print(f"  Read {len(galaxies)} galaxies at z={redshift:.2f}")
            except Exception as e:
                print(f"Error reading snapshot {snap}: {e}")

        # Generate each evolution plot
        generated_plots = []
        for plot_name, plot_func in plot_modules.items():
            try:
                if args.verbose:
                    print(f"Generating {plot_name}...")
                plot_path = plot_func(
                    snapshots=snapshot_data,
                    params=params.params,
                    output_dir=output_dir,
                    output_format=args.format,
                    verbose=args.verbose,
                )
                generated_plots.append(plot_path)
                print(f"Generated: {plot_path}")
            except Exception as e:
                print(f"Error generating {plot_name}: {e}")

        if args.verbose:
            print(f"Generated {len(generated_plots)} evolution plots.")


if __name__ == "__main__":
    main()
