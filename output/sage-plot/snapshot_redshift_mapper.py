#!/usr/bin/env python

"""
SAGE Snapshot to Redshift Mapper

This module provides a class to handle mapping between snapshot numbers and redshifts
for SAGE galaxy evolution plots.
"""

import glob
import os
import re
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
        self.output_dir = output_dir
        self.file_name_galaxies = (
            params.get("FileNameGalaxies", "model") if params else "model"
        )
        self.snapshots = []  # Snapshot indices (0 to n-1)
        self.redshifts = []  # Corresponding redshift values
        self.redshift_strs = []  # Formatted strings for filenames (e.g., "_z0.000")
        self.redshift_file_patterns = []  # Complete filename patterns
        self.mapping_source = None  # Record where mapping came from for debugging
        self.load_mapping()

    def load_mapping(self):
        """
        Load mapping between snapshots and redshifts using various methods.

        Returns:
            True if mapping was successfully loaded, False otherwise
        """
        # Strategy 1: Try to read from .a_list file specified in parameter file
        if self._load_from_a_list():
            self.mapping_source = "a_list_file"
            return True

        # Strategy 2: Try to discover redshifts from existing model files
        if self._load_from_model_files():
            self.mapping_source = "model_files"
            return True

        # Strategy 3: Fall back to hardcoded redshift list from original code
        if self._load_hardcoded_values():
            self.mapping_source = "hardcoded_values"
            return True

        # Strategy 4: Use approximate formula as last resort
        self._generate_approximate_mapping()
        self.mapping_source = "approximate_formula"
        return True

    def _load_from_a_list(self):
        """
        Load expansion factors from .a_list file and convert to redshifts.

        Returns:
            True if successful, False otherwise
        """
        # Get the .a_list file path from parameters
        if not self.params:
            return False

        a_list_file = self.params.get("FileWithSnapList")
        if not a_list_file:
            return False

        # Ensure path is absolute
        if self.output_dir and not os.path.isabs(a_list_file):
            a_list_file = os.path.join(self.output_dir, a_list_file)

        # Check if the file exists
        if not os.path.exists(a_list_file):
            print(f"Warning: FileWithSnapList '{a_list_file}' not found")
            return False

        try:
            # Read expansion factors from the file
            expansion_factors = []
            with open(a_list_file, "r") as f:
                for line in f:
                    # Skip comments and empty lines
                    line = line.strip()
                    if not line or line.startswith("#"):
                        continue

                    # Try to parse as float
                    try:
                        a = float(line)
                        expansion_factors.append(a)
                    except ValueError:
                        continue

            # If we didn't find any valid expansion factors, return False
            if not expansion_factors:
                print(f"Warning: No valid expansion factors found in {a_list_file}")
                return False

            # Convert expansion factors to redshifts
            # Formula: z = 1/a - 1
            redshifts = [1.0 / a - 1.0 for a in expansion_factors]

            # Create the mapping
            self.snapshots = list(range(len(redshifts)))
            self.redshifts = redshifts

            # Create formatted redshift strings for filenames
            self.redshift_strs = [f"_z{z:.3f}" for z in self.redshifts]

            # Create file patterns
            self.redshift_file_patterns = [
                f"{self.file_name_galaxies}{self.redshift_strs[i]}"
                for i in range(len(self.snapshots))
            ]

            print(f"Loaded {len(self.redshifts)} redshift values from {a_list_file}")
            return True

        except Exception as e:
            print(f"Error loading from a_list file: {e}")
            return False

    def _load_from_model_files(self):
        """
        Discover redshifts from existing model files in output directory.

        Returns:
            True if successful, False otherwise
        """
        if not self.output_dir:
            return False

        try:
            # Search for model files with redshift in the name
            pattern = f"{self.file_name_galaxies}_z*.?_*"
            model_files = glob.glob(os.path.join(self.output_dir, pattern))

            # If no files, try more generic pattern
            if not model_files:
                pattern = f"{self.file_name_galaxies}_z*"
                model_files = glob.glob(os.path.join(self.output_dir, pattern))

            # If still no files, return False
            if not model_files:
                print(f"Warning: No model files found matching pattern '{pattern}'")
                return False

            # Extract redshifts from filenames using regex
            z_pattern = re.compile(f"{self.file_name_galaxies}_z(\\d+\\.\\d+)")
            redshift_map = {}

            for model_file in model_files:
                match = z_pattern.search(os.path.basename(model_file))
                if match:
                    redshift = float(match.group(1))
                    # We don't know the snapshot number yet, but we'll sort by redshift
                    redshift_map[redshift] = match.group(0)

            # If we didn't find any valid redshifts, return False
            if not redshift_map:
                print(f"Warning: No valid redshifts extracted from model files")
                return False

            # Sort redshifts in descending order (highest redshift = first snapshot)
            sorted_redshifts = sorted(redshift_map.keys(), reverse=True)

            # Create the mapping
            self.snapshots = list(range(len(sorted_redshifts)))
            self.redshifts = sorted_redshifts

            # Create formatted redshift strings for filenames
            self.redshift_strs = [f"_z{z:.3f}" for z in self.redshifts]

            # Create file patterns
            self.redshift_file_patterns = [
                f"{self.file_name_galaxies}{self.redshift_strs[i]}"
                for i in range(len(self.snapshots))
            ]

            print(f"Discovered {len(self.redshifts)} redshift values from model files")
            return True

        except Exception as e:
            print(f"Error discovering redshifts from model files: {e}")
            return False

    def _load_hardcoded_values(self):
        """
        Load hardcoded redshift values from the original script.

        Returns:
            True if successful, False otherwise
        """
        try:
            # These values are taken from the original history.py script
            hardcoded_redshifts = [
                127.000,
                79.998,
                50.000,
                30.000,
                19.916,
                18.244,
                16.725,
                15.343,
                14.086,
                12.941,
                11.897,
                10.944,
                10.073,
                9.278,
                8.550,
                7.883,
                7.272,
                6.712,
                6.197,
                5.724,
                5.289,
                4.888,
                4.520,
                4.179,
                3.866,
                3.576,
                3.308,
                3.060,
                2.831,
                2.619,
                2.422,
                2.239,
                2.070,
                1.913,
                1.766,
                1.630,
                1.504,
                1.386,
                1.276,
                1.173,
                1.078,
                0.989,
                0.905,
                0.828,
                0.755,
                0.687,
                0.624,
                0.564,
                0.509,
                0.457,
                0.408,
                0.362,
                0.320,
                0.280,
                0.242,
                0.208,
                0.175,
                0.144,
                0.116,
                0.089,
                0.064,
                0.041,
                0.020,
                0.000,
            ]

            # Create the mapping
            self.snapshots = list(range(len(hardcoded_redshifts)))
            self.redshifts = hardcoded_redshifts

            # Create formatted redshift strings for filenames
            self.redshift_strs = [f"_z{z:.3f}" for z in self.redshifts]

            # Create file patterns
            self.redshift_file_patterns = [
                f"{self.file_name_galaxies}{self.redshift_strs[i]}"
                for i in range(len(self.snapshots))
            ]

            print(f"Using {len(self.redshifts)} hardcoded redshift values")
            return True

        except Exception as e:
            print(f"Error loading hardcoded redshift values: {e}")
            return False

    def _generate_approximate_mapping(self):
        """
        Generate approximate mapping using simple formula.

        Returns:
            True if successful, False otherwise
        """
        try:
            # Get parameters for the approximate mapping
            last_snapshot = self.params.get("LastSnapShotNr", 63) if self.params else 63
            first_snapshot = self.params.get("FirstSnapShotNr", 0) if self.params else 0

            # Create snapshot list
            self.snapshots = list(range(first_snapshot, last_snapshot + 1))

            # Generate redshifts using approximate formula
            # This formula is derived from the original code
            self.redshifts = []
            for snap in self.snapshots:
                if snap == last_snapshot:
                    z = 0.0
                else:
                    z = 0.5 * (last_snapshot - snap)
                self.redshifts.append(z)

            # Create formatted redshift strings for filenames
            self.redshift_strs = [f"_z{z:.3f}" for z in self.redshifts]

            # Create file patterns
            self.redshift_file_patterns = [
                f"{self.file_name_galaxies}{self.redshift_strs[i]}"
                for i in range(len(self.snapshots))
            ]

            print(f"Generated {len(self.redshifts)} approximate redshift values")
            return True

        except Exception as e:
            print(f"Error generating approximate redshift mapping: {e}")
            return False

    def get_redshift(self, snapshot):
        """
        Get redshift value for a given snapshot index.

        Args:
            snapshot: Snapshot index

        Returns:
            Redshift value, or 0.0 if not found
        """
        try:
            idx = self.snapshots.index(snapshot)
            return self.redshifts[idx]
        except (ValueError, IndexError):
            # If snapshot not found or out of range, use approximate calculation
            last_snapshot = max(self.snapshots) if self.snapshots else 63
            if snapshot == last_snapshot:
                return 0.0
            else:
                return 0.5 * (last_snapshot - snapshot)

    def get_redshift_str(self, snapshot):
        """
        Get formatted redshift string for filenames.

        Args:
            snapshot: Snapshot index

        Returns:
            Formatted string (e.g., "_z0.000"), or empty string if not found
        """
        try:
            idx = self.snapshots.index(snapshot)
            return self.redshift_strs[idx]
        except (ValueError, IndexError):
            # If snapshot not found or out of range, generate on the fly
            redshift = self.get_redshift(snapshot)
            return f"_z{redshift:.3f}"

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
        Get the standard set of snapshots for evolution plots.

        Returns:
            List of snapshot indices corresponding to the standard set
        """
        # If we have a significant number of snapshots, select a diverse range
        if len(self.snapshots) >= 6:
            # Sort snapshots by redshift
            sorted_pairs = sorted(
                zip(self.snapshots, self.redshifts), key=lambda p: p[1]
            )

            # Get a diverse selection of snapshots
            if len(sorted_pairs) >= 8:
                # Pick z=0 and high-z, plus 6 spaced out in between
                z0_pair = sorted_pairs[0]  # Lowest redshift (closest to z=0)
                high_z_pair = sorted_pairs[-1]  # Highest redshift

                # Add these two definitely
                selected = [z0_pair]

                # For the remaining 6, space them out
                remaining = sorted_pairs[1:-1]
                if len(remaining) <= 6:
                    # Use all remaining if 6 or fewer
                    selected.extend(remaining)
                else:
                    # Otherwise pick 6 evenly spaced
                    step = len(remaining) // 6
                    for i in range(0, len(remaining), step):
                        if len(selected) < 7:  # Only pick 6 more
                            selected.append(remaining[i])

                # Add the high redshift one
                selected.append(high_z_pair)

                # Extract just the snapshot numbers
                return [snap for snap, _ in selected]
            else:
                # Use all snapshots if fewer than 8
                return self.snapshots

        # If we only have a few snapshots, use all of them
        if len(self.snapshots) > 0:
            return self.snapshots

        # If no snapshots available, use the standard list and try to find closest matches
        # These values are equivalent to SMFsnaps in history.py
        smf_snaps = [63, 37, 32, 27, 23, 20, 18, 16]
        return smf_snaps  # Just return the standard ones as a fallback

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
