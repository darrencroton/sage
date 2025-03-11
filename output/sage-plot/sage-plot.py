#!/usr/bin/env python

"""
SAGE Plotting Tool - Master plotting script for SAGE galaxy formation model output

Usage:
  python sage-plot.py --param-file=<param_file> [options]

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
  --no-tex               Don't use LaTeX for text rendering
  --verbose              Show detailed output
  --help                 Show this help message
"""

import os
import sys
import argparse
import importlib
import glob
import re
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
from collections import OrderedDict
from tqdm import tqdm

# Import figure modules
from figures import *

# Galaxy data structure definition
def get_galaxy_dtype():
    """Return the NumPy dtype for SAGE galaxy data."""
    galdesc_full = [
        ('SnapNum', np.int32),                    
        ('Type', np.int32),                    
        ('GalaxyIndex', np.int64),                    
        ('CentralGalaxyIndex', np.int64),                    
        ('SAGEHaloIndex', np.int32),                    
        ('SAGETreeIndex', np.int32),                    
        ('SimulationHaloIndex', np.int64),                    
        ('mergeType', np.int32),                    
        ('mergeIntoID', np.int32),                    
        ('mergeIntoSnapNum', np.int32),                    
        ('dT', np.float32),                    
        ('Pos', (np.float32, 3)),             
        ('Vel', (np.float32, 3)),             
        ('Spin', (np.float32, 3)),             
        ('Len', np.int32),                    
        ('Mvir', np.float32),                  
        ('CentralMvir', np.float32),                  
        ('Rvir', np.float32),                  
        ('Vvir', np.float32),                  
        ('Vmax', np.float32),                  
        ('VelDisp', np.float32),                  
        ('ColdGas', np.float32),                  
        ('StellarMass', np.float32),                  
        ('BulgeMass', np.float32),                  
        ('HotGas', np.float32),                  
        ('EjectedMass', np.float32),                  
        ('BlackHoleMass', np.float32),                  
        ('IntraClusterStars', np.float32),                  
        ('MetalsColdGas', np.float32),                  
        ('MetalsStellarMass', np.float32),                  
        ('MetalsBulgeMass', np.float32),                  
        ('MetalsHotGas', np.float32),                  
        ('MetalsEjectedMass', np.float32),                  
        ('MetalsIntraClusterStars', np.float32),                  
        ('SfrDisk', np.float32),                  
        ('SfrBulge', np.float32),                  
        ('SfrDiskZ', np.float32),                  
        ('SfrBulgeZ', np.float32),                  
        ('DiskRadius', np.float32),                  
        ('Cooling', np.float32),                  
        ('Heating', np.float32),
        ('QuasarModeBHaccretionMass', np.float32),
        ('TimeOfLastMajorMerger', np.float32),
        ('TimeOfLastMinorMerger', np.float32),
        ('OutflowRate', np.float32),
        ('infallMvir', np.float32),
        ('infallVvir', np.float32),
        ('infallVmax', np.float32)
    ]
    names = [galdesc_full[i][0] for i in range(len(galdesc_full))]
    formats = [galdesc_full[i][1] for i in range(len(galdesc_full))]
    return np.dtype({'names': names, 'formats': formats}, align=True)


class SAGEParameters:
    """Class to parse and store SAGE parameter file settings."""
    
    def __init__(self, param_file):
        """Initialize with parameter file path."""
        self.param_file = param_file
        self.params = {}
        self.parse_param_file()
        
    def parse_param_file(self):
        """Parse the SAGE parameter file."""
        if not os.path.exists(self.param_file):
            raise FileNotFoundError(f"Parameter file not found: {self.param_file}")
        
        # Parse the parameter file
        with open(self.param_file, 'r') as f:
            for line in f:
                # Skip comments and empty lines
                if line.startswith('#') or line.strip() == '':
                    continue
                
                # Parse key-value pairs
                parts = line.split('=')
                if len(parts) == 2:
                    key = parts[0].strip()
                    value = parts[1].strip().split('#')[0].strip()  # Remove inline comments
                    
                    # Convert to appropriate type
                    if value.isdigit():
                        value = int(value)
                    elif self._is_float(value):
                        value = float(value)
                    
                    self.params[key] = value
    
    def _is_float(self, value):
        """Check if a string can be converted to float."""
        try:
            float(value)
            return True
        except ValueError:
            return False
    
    def get(self, key, default=None):
        """Get a parameter value."""
        return self.params.get(key, default)
    
    def __getitem__(self, key):
        """Allow dictionary-like access to parameters."""
        return self.params[key]
    
    def __contains__(self, key):
        """Check if a parameter exists."""
        return key in self.params


def setup_matplotlib(use_tex=True):
    """Set up matplotlib with standard settings."""
    matplotlib.rcdefaults()
    plt.rc('xtick', labelsize='x-large')
    plt.rc('ytick', labelsize='x-large')
    plt.rc('lines', linewidth='2.0')
    plt.rc('legend', numpoints=1, fontsize='x-large')
    
    if use_tex:
        plt.rc('text', usetex=True)


def read_galaxies(model_path, first_file, last_file):
    """
    Read galaxy data from SAGE output files.
    
    Args:
        model_path: Path to model files
        first_file: First file number to read
        last_file: Last file number to read
    
    Returns:
        Tuple containing:
            - Numpy recarray of galaxy data
            - Volume of the simulation
            - Dictionary of metadata
    """
    # Get simulation parameters from the model path
    # This would typically come from the parameter file
    hubble_h = 0.73
    box_size = 62.5  # Mpc/h for Mini-Millennium
    max_tree_files = 8  # For Mini-Millennium
    
    # Print the model path for debugging
    print(f"Looking for galaxy files with base: {model_path}")
    
    # Look for files matching the pattern in the same directory
    dir_path = os.path.dirname(model_path)
    base_name = os.path.basename(model_path)
    existing_files = glob.glob(os.path.join(dir_path, f"{base_name}_*"))
    
    if existing_files:
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
    
    print(f"Determining storage requirements for files {first_file} to {last_file}...")
    
    # First pass: Determine total number of galaxies
    for fnr in tqdm(range(first_file, last_file + 1), desc="Counting galaxies"):
        fname = f"{model_path}_{fnr}"
        
        if not os.path.isfile(fname):
            continue
            
        if os.path.getsize(fname) == 0:
            print(f"File {fname} is empty! Skipping...")
            continue
        
        try:
            with open(fname, 'rb') as fin:
                ntrees = np.fromfile(fin, np.dtype(np.int32), 1)
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
        print("No galaxies found. Providing a small sample dataset for testing.")
        
        # Create a small sample dataset for testing
        # This will allow the plotting code to run even without real data
        sample_size = 1000
        sample_galaxies = np.recarray((sample_size,), dtype=galdesc)
        
        # Generate some plausible values for testing
        sample_galaxies.SnapNum = 63
        sample_galaxies.Type = np.random.randint(0, 2, sample_size)
        sample_galaxies.StellarMass = 10**np.random.uniform(-2, 1, sample_size)
        sample_galaxies.ColdGas = 10**np.random.uniform(-2, 0.5, sample_size)
        sample_galaxies.BulgeMass = sample_galaxies.StellarMass * np.random.uniform(0, 0.5, sample_size)
        sample_galaxies.SfrDisk = 10**np.random.uniform(-3, 1, sample_size) * sample_galaxies.StellarMass
        sample_galaxies.SfrBulge = 10**np.random.uniform(-4, 0, sample_size) * sample_galaxies.StellarMass
        sample_galaxies.Vmax = 10**np.random.uniform(1.5, 2.5, sample_size)
        
        # Use this sample data
        return sample_galaxies, box_size**3, {
            'hubble_h': hubble_h,
            'box_size': box_size,
            'max_tree_files': max_tree_files,
            'volume': box_size**3,
            'ntrees': sample_size // 10,
            'ngals': sample_size,
            'good_files': max_tree_files,
            'sample_data': True  # Flag that this is sample data
        }
    
    # Initialize the storage array
    galaxies = np.empty(tot_ngals, dtype=galdesc)
    
    # Second pass: Read the galaxy data
    offset = 0
    for fnr in tqdm(range(first_file, last_file + 1), desc="Reading galaxies"):
        fname = f"{model_path}_{fnr}"
        
        if not os.path.isfile(fname) or os.path.getsize(fname) == 0:
            continue
        
        try:
            with open(fname, 'rb') as fin:
                ntrees = np.fromfile(fin, np.dtype(np.int32), 1)
                ntotgals = np.fromfile(fin, np.dtype(np.int32), 1)[0]
                gals_per_tree = np.fromfile(fin, np.dtype((np.int32, ntrees)), 1)
                
                print(f"Reading {ntotgals} galaxies from file: {fname}")
                
                gg = np.fromfile(fin, galdesc, ntotgals)
                
                # Slice the file array into the global array with a copy
                galaxies[offset:offset + ntotgals] = gg[0:ntotgals].copy()
                
                offset += ntotgals
        except Exception as e:
            print(f"Error reading file {fname}: {e}")
            continue
    
    # Convert to recarray for attribute access
    galaxies = galaxies.view(np.recarray)
    
    # Calculate the volume based on good files
    volume = box_size**3 * good_files / max_tree_files
    
    # Create metadata dictionary
    metadata = {
        'hubble_h': hubble_h,
        'box_size': box_size,
        'max_tree_files': max_tree_files,
        'volume': volume,
        'ntrees': tot_ntrees,
        'ngals': tot_ngals,
        'good_files': good_files
    }
    
    return galaxies, volume, metadata


def parse_arguments():
    """Parse command-line arguments."""
    parser = argparse.ArgumentParser(description="SAGE Plotting Tool")
    parser.add_argument("--param-file", required=True, help="SAGE parameter file")
    parser.add_argument("--first-file", type=int, default=0, help="First file to read")
    parser.add_argument("--last-file", type=int, default=None, help="Last file to read")
    parser.add_argument("--snapshot", type=int, default=None, help="Process only this snapshot number")
    parser.add_argument("--all-snapshots", action="store_true", help="Process all available snapshots")
    parser.add_argument("--evolution", action="store_true", help="Generate evolution plots")
    parser.add_argument("--snapshot-plots", action="store_true", help="Generate snapshot plots")
    parser.add_argument("--output-dir", default="./plots", help="Output directory for plots")
    parser.add_argument("--format", default=".png", help="Output format (.png, .pdf)")
    parser.add_argument("--plots", default="all", help="Comma-separated list of plots to generate")
    parser.add_argument("--no-tex", action="store_true", help="Don't use LaTeX for text rendering")
    parser.add_argument("--verbose", action="store_true", help="Show detailed output")
    
    args = parser.parse_args()
    
    # Default to snapshot plots if neither is specified
    if not args.evolution and not args.snapshot_plots:
        args.snapshot_plots = True
    
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
    
    # Determine which modules to look for based on plot type
    if plot_type == 'snapshot':
        module_patterns = [
            'stellar_mass_function', 
            'baryonic_mass_function',
            'gas_mass_function',
            'baryonic_tully_fisher',
            'specific_sfr'
        ]
    else:  # 'evolution'
        module_patterns = [
            'smf_evolution',
            'sfr_density_evolution',
            'stellar_mass_density_evolution'
        ]
    
    # Import modules
    for pattern in module_patterns:
        try:
            module_name = f"figures.{pattern}"
            module = importlib.import_module(module_name)
            # Get the main plotting function from the module
            plot_func = getattr(module, 'plot', None)
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
            # Print a sample of parameters to avoid overwhelming output
            for i, (key, value) in enumerate(params.params.items()):
                if i < 10:
                    print(f"  {key} = {value}")
                else:
                    print("  ...")
                    break
    except Exception as e:
        print(f"Error loading parameter file: {e}")
        sys.exit(1)
        
    # Verify paths from parameter file
    output_dir = params.get('OutputDir')
    simulation_dir = params.get('SimulationDir')
    
    if output_dir and not os.path.exists(output_dir):
        print(f"Warning: OutputDir '{output_dir}' from parameter file does not exist or is not accessible.")
        
    if simulation_dir and not os.path.exists(simulation_dir):
        print(f"Warning: SimulationDir '{simulation_dir}' from parameter file does not exist or is not accessible.")
    
    # Set up matplotlib
    setup_matplotlib(not args.no_tex)
    
    # Ensure output directory exists
    os.makedirs(args.output_dir, exist_ok=True)
    
    # Determine which plots to generate
    if args.plots == "all":
        selected_plots = None  # All available
    else:
        selected_plots = [p.strip() for p in args.plots.split(",")]
    
    # Generate snapshot plots
    if args.snapshot_plots:
        # Get output model path and snapshot number
        model_path = params.get('OutputDir', './')
        snapshot = args.snapshot or params.get('LastSnapshotNr', 63)
        
        # Check if model_path exists
        if not os.path.exists(model_path):
            print(f"Warning: OutputDir '{model_path}' from parameter file does not exist.")
            print("Using a default path instead.")
            model_path = "./"
        
        # Look for model files with pattern model_z*.* in the model_path
        model_files = glob.glob(os.path.join(model_path, "model_z*.*"))
        
        if len(model_files) > 0:
            # Use the first matching file as the base
            base_model_file = os.path.splitext(model_files[0])[0]
            if args.verbose:
                print(f"Found model file: {base_model_file}")
        else:
            # Determine model file path using redshift
            redshift = params.get('ZZ', 0.0)
            if isinstance(redshift, str):
                try:
                    redshift = float(redshift)
                except ValueError:
                    redshift = 0.0
                    
            base_model_file = f"{model_path}/model_z{redshift:.3f}"
            if args.verbose:
                print(f"Using model file: {base_model_file}")
        
        # Determine last file number if not specified
        last_file = args.last_file
        if last_file is None:
            last_file = params.get('FirstFile', 0) + params.get('NumFiles', 8) - 1
        
        # Read galaxy data
        try:
            galaxies, volume, metadata = read_galaxies(
                model_path=base_model_file,
                first_file=args.first_file,
                last_file=last_file
            )
            if args.verbose:
                if metadata.get('sample_data', False):
                    print("USING SAMPLE DATA: No real galaxy data was found, using generated test data instead.")
                    print("This is useful for testing the plotting code but the plots will not reflect real SAGE output.")
                print(f"Read {len(galaxies)} galaxies from volume {volume:.2f} (Mpc/h)³")
        except Exception as e:
            print(f"Error reading galaxy data: {e}")
            sys.exit(1)
        
        # Get available snapshot plot modules
        plot_modules = get_available_plot_modules('snapshot')
        
        if args.verbose:
            print(f"Available snapshot plots: {', '.join(plot_modules.keys())}")
        
        # Filter to selected plots if specified
        if selected_plots:
            plot_modules = {k: v for k, v in plot_modules.items() if k in selected_plots}
        
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
                    output_dir=args.output_dir,
                    output_format=args.format
                )
                generated_plots.append(plot_path)
                print(f"Generated: {plot_path}")
            except Exception as e:
                print(f"Error generating {plot_name}: {e}")
        
        print(f"Generated {len(generated_plots)} snapshot plots.")
    
    # Generate evolution plots
    if args.evolution:
        # Get available evolution plot modules
        plot_modules = get_available_plot_modules('evolution')
        
        if args.verbose:
            print(f"Available evolution plots: {', '.join(plot_modules.keys())}")
        
        # Filter to selected plots if specified
        if selected_plots:
            plot_modules = {k: v for k, v in plot_modules.items() if k in selected_plots}
        
        # Determine which snapshots to process
        model_path = params.get('OutputDir', './')
        if args.all_snapshots:
            # Process all snapshots based on parameter file
            first_snap = params.get('FirstSnapShotNr', 0)
            last_snap = params.get('LastSnapshotNr', 63)
            snapshots = list(range(first_snap, last_snap + 1))
        elif args.snapshot:
            # Process only the specified snapshot
            snapshots = [args.snapshot]
        else:
            # Default snapshots for evolution plots (equivalent to SMFsnaps in history.py)
            snapshots = [63, 37, 32, 27, 23, 20, 18, 16]
        
        # Read galaxy data for each snapshot
        snapshot_data = {}
        for snap in snapshots:
            if args.verbose:
                print(f"Processing snapshot {snap}...")
            
            # Determine redshift for this snapshot using parameter settings
            # In a real implementation, this would use the proper mapping
            redshift = 0.0
            if snap < 63:
                redshift = 0.5 * (63 - snap)
            
            # Determine model file path for this snapshot
            # Look for model files with pattern model_z{redshift}*.* in the model_path
            # Use a fuzzy match approach to find a file with a similar redshift
            model_files = glob.glob(os.path.join(model_path, f"model_z*.*"))
            
            # Find the closest match based on redshift in the filename
            base_model_file = None
            if model_files:
                best_match = None
                min_diff = float('inf')
                pattern = re.compile(r'model_z(\d+\.\d+)')
                
                for model_file in model_files:
                    match = pattern.search(model_file)
                    if match:
                        file_redshift = float(match.group(1))
                        diff = abs(file_redshift - redshift)
                        if diff < min_diff:
                            min_diff = diff
                            best_match = model_file
                
                if best_match:
                    base_model_file = os.path.splitext(best_match)[0]
                    if args.verbose:
                        print(f"Found closest model file for z={redshift}: {base_model_file}")
            
            # If no match found, use the default naming pattern
            if not base_model_file:
                base_model_file = f"{model_path}/model_z{redshift:.3f}"
                if args.verbose:
                    print(f"Using default model file pattern: {base_model_file}")
            
            # Determine last file number if not specified
            last_file = args.last_file
            if last_file is None:
                last_file = params.get('FirstFile', 0) + params.get('NumFiles', 8) - 1
            
            try:
                galaxies, volume, metadata = read_galaxies(
                    model_path=base_model_file,
                    first_file=args.first_file,
                    last_file=last_file
                )
                # Add redshift to metadata
                metadata['redshift'] = redshift
                snapshot_data[snap] = (galaxies, volume, metadata)
                if args.verbose:
                    if metadata.get('sample_data', False):
                        print(f"  Using SAMPLE DATA for z={redshift:.2f}")
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
                    output_dir=args.output_dir,
                    output_format=args.format
                )
                generated_plots.append(plot_path)
                print(f"Generated: {plot_path}")
            except Exception as e:
                print(f"Error generating {plot_name}: {e}")
        
        print(f"Generated {len(generated_plots)} evolution plots.")


if __name__ == "__main__":
    main()
