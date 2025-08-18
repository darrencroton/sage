#!/usr/bin/env python

"""
SAGE Snapshot to Redshift Mapper

This module provides a class to handle mapping between snapshot numbers and redshifts
for SAGE galaxy evolution plots.
"""

import os
import sys
from collections import OrderedDict

import numpy as np


class SnapshotRedshiftMapper:
    """Maps between snapshot numbers and redshifts using various data sources."""

    def __init__(self, param_file=None, params=None, output_dir=None):
        """
        Initialize with parameter file and output directory paths.

        Args:
            param_file: Path to SAGE parameter file
            params: Dictionary with already parsed parameters (optional)
            output_dir: Path to SAGE output directory
        """
        self.param_file = param_file
        self.params = params  # Can pass already parsed params
        
        # Check that we have valid parameters
        if not params:
            print("Error: Parameter dictionary is required for SnapshotRedshiftMapper")
            sys.exit(1)
        
        # Check for critical parameters
        required_params = ["FileNameGalaxies"]
        missing_params = [p for p in required_params if p not in params]
        if missing_params:
            print(f"Error: Required parameters missing from parameter file: {', '.join(missing_params)}")
            sys.exit(1)
            
        # Set up the mapper
        self.output_dir = output_dir
        self.file_name_galaxies = params["FileNameGalaxies"]
        self.snapshots = []  # Snapshot indices (0 to n-1)
        self.redshifts = []  # Corresponding redshift values
        self.redshift_strs = []  # Formatted strings for filenames (e.g., "_z0.000")
        self.redshift_file_patterns = []  # Complete filename patterns
        self.mapping_source = None  # Record where mapping came from for debugging
        
        # Load the mapping between snapshots and redshifts
        self.load_mapping()

    def load_mapping(self):
        """
        Load mapping between snapshots and redshifts from parameter file.

        Returns:
            True if mapping was successfully loaded, False otherwise
        """
        # Only use the a_list file specified in parameter file - no fallbacks
        if not self._load_from_a_list():
            print("Error: Could not load redshift data from FileWithSnapList")
            print("Please ensure FileWithSnapList parameter points to a valid a_list file")
            sys.exit(1)
            
        self.mapping_source = "a_list_file"
        return True

    def _load_from_a_list(self):
        """
        Load expansion factors from .a_list file and convert to redshifts.

        Returns:
            True if successful, False otherwise
        """
        # Get the .a_list file path from parameters - required parameter
        if not self.params:
            print("Error: Parameter dictionary is required")
            return False

        # Check if required parameters exist
        required_params = ["FileWithSnapList", "LastSnapShotNr"]
        missing_params = [p for p in required_params if p not in self.params]
        if missing_params:
            print(f"Error: Required parameters missing from parameter file: {', '.join(missing_params)}")
            return False
            
        a_list_file = self.params["FileWithSnapList"]
        last_snapshot_nr = self.params["LastSnapShotNr"]

        # The a_list_file path is already resolved by SAGEParameters class
        # No need for additional path manipulation

        if self.params.get("verbose", False):
            print(f"Using a_list file: {a_list_file}")

        # Check if the file exists
        if not os.path.exists(a_list_file):
            print(f"Error: FileWithSnapList '{a_list_file}' not found")
            print(f"Please check that the file exists and the path is correct")
            return False

        # Try to read the a_list file 
        try:
            # Read expansion factors from the file
            expansion_factors = []
            line_number = 0
            with open(a_list_file, "r") as f:
                for line in f:
                    line_number += 1
                    # Skip comments and empty lines
                    line = line.strip()
                    if not line or line.startswith("#"):
                        continue

                    # Try to parse as float
                    try:
                        a = float(line)
                        # Check for valid expansion factor (must be positive)
                        if a <= 0:
                            print(f"Warning: Invalid expansion factor {a} at line {line_number} (must be positive)")
                            continue
                        expansion_factors.append(a)
                    except ValueError:
                        print(f"Warning: Non-numeric value '{line}' at line {line_number}")
                        continue

            # If we didn't find any valid expansion factors, return False
            if not expansion_factors:
                print(f"Error: No valid expansion factors found in {a_list_file}")
                print("File should contain one expansion factor per line (e.g., 0.5, 0.6, 0.7, ...)")
                return False

            # Expansion factors should be in descending order (earliest time first, latest last)
            # Check if they need to be reversed
            if len(expansion_factors) > 1 and expansion_factors[0] > expansion_factors[-1]:
                expansion_factors.reverse()
                print("Note: Expansion factors were in descending order, reversing to ascending order")

            # Convert expansion factors to redshifts
            # Formula: z = 1/a - 1
            try:
                redshifts = [1.0 / a - 1.0 for a in expansion_factors]
            except ZeroDivisionError:
                print("Error: Encountered division by zero when converting expansion factors to redshifts")
                print("Ensure all expansion factors are positive non-zero values")
                return False

            # Create the mapping - need to match snapshot numbers to redshifts
            # Typically, the last expansion factor (largest) corresponds to the LastSnapShotNr
            # The first expansion factor (smallest) corresponds to snapshot 0
            num_snapshots = len(expansion_factors)
            
            # Calculate snapshot numbers
            if num_snapshots <= 1:
                self.snapshots = [last_snapshot_nr]
            else:
                # Handle case where we have multiple snapshots
                # In typical format, snapshots run from 0 to LastSnapShotNr
                snapshot_step = last_snapshot_nr / (num_snapshots - 1)
                self.snapshots = [round(i * snapshot_step) for i in range(num_snapshots)]
                
                # Ensure last snapshot is exactly LastSnapShotNr
                if self.snapshots[-1] != last_snapshot_nr:
                    self.snapshots[-1] = last_snapshot_nr
                    
            # Ensure we have the same number of snapshots and redshifts
            if len(self.snapshots) != len(redshifts):
                print(f"Error: Mismatch between number of snapshots ({len(self.snapshots)}) and redshifts ({len(redshifts)})")
                return False
                
            self.redshifts = redshifts

            # Create formatted redshift strings for filenames
            self.redshift_strs = [f"_z{z:.3f}" for z in self.redshifts]

            # Create file patterns
            self.redshift_file_patterns = [
                f"{self.file_name_galaxies}{self.redshift_strs[i]}"
                for i in range(len(self.snapshots))
            ]

            print(f"Loaded {len(self.redshifts)} redshift values from {a_list_file}")
            if self.params.get("verbose", False):
                # Print first few snapshot-redshift pairs
                print("Snapshot-Redshift mapping (showing first 5):")
                for i in range(min(5, len(self.snapshots))):
                    print(f"  Snapshot {self.snapshots[i]}: z = {self.redshifts[i]:.3f}")
                if len(self.snapshots) > 5:
                    print(f"  ... and {len(self.snapshots) - 5} more snapshots")
                    
            return True

        except Exception as e:
            print(f"Error loading from a_list file: {e}")
            return False

    # Removed fallback methods:
    # - _load_from_model_files
    # - _load_hardcoded_values
    # - _generate_approximate_mapping

    def get_redshift(self, snapshot):
        """
        Get redshift value for a given snapshot index.

        Args:
            snapshot: Snapshot index

        Returns:
            Redshift value
        """
        try:
            idx = self.snapshots.index(snapshot)
            return self.redshifts[idx]
        except (ValueError, IndexError):
            print(f"Error: Snapshot {snapshot} not found in redshift mapping")
            print(f"Available snapshots: {self.snapshots}")
            sys.exit(1)

    def get_redshift_str(self, snapshot):
        """
        Get formatted redshift string for filenames.

        Args:
            snapshot: Snapshot index

        Returns:
            Formatted string (e.g., "_z0.000")
        """
        try:
            idx = self.snapshots.index(snapshot)
            return self.redshift_strs[idx]
        except (ValueError, IndexError):
            print(f"Error: Snapshot {snapshot} not found in redshift mapping")
            print(f"Available snapshots: {self.snapshots}")
            sys.exit(1)

    def get_model_file_path(self, snapshot, file_num):
        """
        Get the full path to a model file for a given snapshot and file number.

        Args:
            snapshot: Snapshot index
            file_num: File number

        Returns:
            Full path to the model file
        """
        redshift_str = self.get_redshift_str(snapshot)

        # Check if we have an output directory
        if self.output_dir:
            return os.path.join(
                self.output_dir, f"{self.file_name_galaxies}{redshift_str}_{file_num}"
            )
        else:
            return f"{self.file_name_galaxies}{redshift_str}_{file_num}"

    def select_snapshots_for_evolution(self, num_snapshots=8, redshift_max=8.0):
        """
        Select appropriate snapshots for evolution plots.

        Args:
            num_snapshots: Number of snapshots to select
            redshift_max: Maximum redshift to consider

        Returns:
            List of snapshot indices
        """
        # Create a list of (snapshot, redshift) tuples
        snapshot_redshift_pairs = list(zip(self.snapshots, self.redshifts))

        # Filter out snapshots with redshift > redshift_max
        filtered_pairs = [
            (snap, z) for snap, z in snapshot_redshift_pairs if z <= redshift_max
        ]

        # If we have no snapshots left, return an empty list
        if not filtered_pairs:
            return []

        # If we have fewer snapshots than requested, return all of them
        if len(filtered_pairs) <= num_snapshots:
            return [snap for snap, _ in filtered_pairs]

        # Otherwise, select snapshots evenly spaced in redshift
        # Sort by redshift
        filtered_pairs.sort(key=lambda x: x[1])

        # Determine step size
        step = max(1, len(filtered_pairs) // num_snapshots)

        # Select snapshots
        selected_pairs = filtered_pairs[::step][:num_snapshots]

        # If we have fewer than requested, add some more
        if len(selected_pairs) < num_snapshots:
            # Find snapshots that weren't selected
            unselected = [p for p in filtered_pairs if p not in selected_pairs]

            # Sort by redshift
            unselected.sort(key=lambda x: x[1])

            # Add as many as needed
            selected_pairs.extend(unselected[: num_snapshots - len(selected_pairs)])

            # Resort by redshift
            selected_pairs.sort(key=lambda x: x[1])

        # Extract snapshot indices
        selected_snapshots = [snap for snap, _ in selected_pairs]

        return selected_snapshots

    def get_all_snapshots(self):
        """
        Get all available snapshots.

        Returns:
            List of all snapshot indices
        """
        return self.snapshots.copy()

    def get_evolution_snapshots(self):
        """
        Get snapshots for evolution plots based on parameter file.
        
        Priority:
        1. OutputSnapshots list from parameter file (-> arrow notation in param file)
        2. If no OutputSnapshots or invalid, use a diverse selection of available snapshots
        
        Returns:
            List of snapshot indices for evolution plots
        """
        # First, ensure we have snapshots available
        if not self.snapshots:
            print("Error: No snapshots available in redshift mapping")
            sys.exit(1)
            
        # Check for the OutputSnapshots parameter which is set by the arrow notation in parameter file
        if "OutputSnapshots" in self.params:
            output_snapshots = self.params["OutputSnapshots"]
            
            if not output_snapshots:
                print("Warning: OutputSnapshots parameter is empty")
            else:
                # Check which snapshots from OutputSnapshots are valid
                valid_snapshots = []
                invalid_snapshots = []
                
                for snap in output_snapshots:
                    if snap in self.snapshots:
                        valid_snapshots.append(snap)
                    else:
                        invalid_snapshots.append(snap)
                
                # Report on validity
                if invalid_snapshots:
                    print(f"Warning: Some snapshots from OutputSnapshots are not in redshift mapping: {invalid_snapshots}")
                
                if valid_snapshots:
                    # Sort by redshift (from low to high)
                    # This ensures we get a proper evolution sequence
                    snap_z_pairs = [(snap, self.get_redshift(snap)) for snap in valid_snapshots]
                    sorted_pairs = sorted(snap_z_pairs, key=lambda x: x[1])
                    
                    print(f"Using {len(sorted_pairs)} snapshots from parameter file for evolution plots")
                    
                    # Return just the snapshot numbers, sorted by redshift
                    return [snap for snap, _ in sorted_pairs]
                else:
                    print("Warning: No valid snapshots found in OutputSnapshots list")
        
        # If we don't have OutputSnapshots or none are valid, select a diverse range
        print("Note: Using a diverse selection of snapshots for evolution plots")
        
        # Sort all available snapshots by redshift (low to high)
        sorted_pairs = sorted(
            zip(self.snapshots, self.redshifts), key=lambda p: p[1]
        )
        
        # Get a diverse selection of snapshots
        if len(sorted_pairs) >= 8:
            # For 8 or more available snapshots, pick a diverse set:
            # - Lowest redshift (closest to z=0)
            # - Highest redshift
            # - 6 evenly spaced in between
            z0_pair = sorted_pairs[0]
            high_z_pair = sorted_pairs[-1]
            
            # Start with lowest redshift
            selected = [z0_pair]
            
            # Add evenly spaced intermediate redshifts
            remaining = sorted_pairs[1:-1]
            if remaining:
                if len(remaining) <= 6:
                    # If 6 or fewer remaining, use all of them
                    selected.extend(remaining)
                else:
                    # Pick 6 evenly spaced
                    step = len(remaining) // 6
                    for i in range(0, len(remaining), step):
                        if len(selected) < 7:  # We want 7 total (plus high-z = 8)
                            selected.append(remaining[i])
            
            # Add highest redshift
            selected.append(high_z_pair)
        else:
            # For fewer than 8 snapshots, use all available
            selected = sorted_pairs
        
        # Extract and return just the snapshot numbers
        return [snap for snap, _ in selected]

    def debug_info(self):
        """
        Return debug information about the current mapping.

        Returns:
            Multi-line string with debug information
        """
        info = [
            f"SnapshotRedshiftMapper Debug Info:",
            f"  Mapping source: {self.mapping_source}",
            f"  Number of snapshots: {len(self.snapshots)}",
            f"  File name base: {self.file_name_galaxies}",
            f"  Output directory: {self.output_dir}",
            f"\nSnapshot-Redshift Mapping (showing first 10):",
        ]

        # Show first 10 snapshot-redshift pairs
        for i, (snap, z) in enumerate(zip(self.snapshots, self.redshifts)):
            if i >= 10:
                info.append(f"  ...and {len(self.snapshots) - 10} more")
                break
            info.append(
                f"  Snapshot {snap}: z={z:.3f}, file pattern={self.redshift_file_patterns[i]}"
            )

        # Show evolution snapshots
        try:
            evo_snaps = self.get_evolution_snapshots()
            info.append(f"\nStandard Evolution Snapshots:")
            for snap in evo_snaps:
                info.append(f"  Snapshot {snap}: z={self.get_redshift(snap):.3f}")
        except Exception as e:
            info.append(f"\nError getting evolution snapshots: {e}")

        return "\n".join(info)
