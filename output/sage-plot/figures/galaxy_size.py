#!/usr/bin/env python

"""
SAGE Galaxy Size Relations Plot

This module generates a plot showing the relationship between galaxy size and stellar mass.
"""

import os
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import MultipleLocator
import random
from figures import setup_plot_fonts, setup_legend, get_stellar_mass_label, AXIS_LABEL_SIZE, LEGEND_FONT_SIZE, IN_FIGURE_TEXT_SIZE

def plot(galaxies, volume, metadata, params, output_dir="plots", output_format=".png"):
    """
    Create a galaxy size vs. stellar mass plot.
    
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
    # Set random seed for reproducibility when sampling points
    random.seed(2222)
    
    # Set up the figure
    fig, ax = plt.subplots(figsize=(8, 6))
    
    # Apply consistent font settings
    setup_plot_fonts(ax)
    
    # Extract necessary metadata
    hubble_h = metadata['hubble_h']
    
    # Maximum number of points to plot (for better performance and readability)
    dilute = 7500
    
    # Filter for galaxies with non-zero DiskRadius and StellarMass
    # Divide by types: disk-dominated and bulge-dominated
    disk_dominated = np.where((galaxies.DiskRadius > 0.0) & 
                             (galaxies.StellarMass > 0.0) &
                             (galaxies.BulgeMass / galaxies.StellarMass < 0.5))[0]
    
    bulge_dominated = np.where((galaxies.DiskRadius > 0.0) & 
                              (galaxies.StellarMass > 0.0) &
                              (galaxies.BulgeMass / galaxies.StellarMass >= 0.5))[0]
    
    # Check if we have any galaxies to plot
    if len(disk_dominated) == 0 and len(bulge_dominated) == 0:
        print("No galaxies found with DiskRadius > 0 and StellarMass > 0")
        # Create an empty plot with a message
        ax.text(0.5, 0.5, "No galaxies found with DiskRadius > 0 and StellarMass > 0", 
                horizontalalignment='center', verticalalignment='center',
                transform=ax.transAxes, fontsize=IN_FIGURE_TEXT_SIZE)
        
        # Save the figure
        os.makedirs(output_dir, exist_ok=True)
        output_path = os.path.join(output_dir, f"GalaxySize{output_format}")
        plt.savefig(output_path)
        plt.close()
        return output_path
    
    # If we have too many galaxies, randomly sample a subset
    disk_indices = disk_dominated
    if len(disk_dominated) > dilute:
        disk_indices = random.sample(list(disk_dominated), dilute)
    
    bulge_indices = bulge_dominated
    if len(bulge_dominated) > dilute:
        bulge_indices = random.sample(list(bulge_dominated), dilute)
    
    # Calculate stellar mass and disk radius
    # Convert to physical units
    disk_mass = np.log10(galaxies.StellarMass[disk_indices] * 1.0e10 / hubble_h)
    disk_radius = galaxies.DiskRadius[disk_indices] / hubble_h  # in kpc
    
    bulge_mass = np.log10(galaxies.StellarMass[bulge_indices] * 1.0e10 / hubble_h)
    bulge_radius = galaxies.DiskRadius[bulge_indices] / hubble_h  # in kpc
    
    # Print some debug information
    print(f"Galaxy Size Relations plot debug:")
    print(f"  Number of disk-dominated galaxies: {len(disk_indices)}")
    print(f"  Number of bulge-dominated galaxies: {len(bulge_indices)}")
    print(f"  Disk-dominated mass range: {min(disk_mass):.2f} to {max(disk_mass):.2f}")
    print(f"  Disk-dominated radius range: {min(disk_radius):.2f} to {max(disk_radius):.2f}")
    
    # Plot the galaxy data
    ax.scatter(disk_mass, disk_radius, marker='o', s=1, c='b', alpha=0.5, label='Disk-dominated')
    if len(bulge_indices) > 0:
        ax.scatter(bulge_mass, bulge_radius, marker='o', s=1, c='r', alpha=0.5, label='Bulge-dominated')
    
    # Add a reference power law relation: R ∝ M^0.3
    # This is a simplified approximation of the galaxy size-mass relation
    # Based on observations that suggest R ∝ M^(0.2-0.3) for disk galaxies
    mass_range = np.logspace(8, 12, 100) / 1.0e10  # In SAGE units
    radius_ref = 3.0 * (mass_range * 1.0e10 / 1.0e10)**0.3  # Normalized to ~3 kpc at 10^10 Msun
    
    ax.plot(np.log10(mass_range * 1.0e10), radius_ref, 'k--', lw=1.5, label=r'$R \propto M_*^{0.3}$')
    
    # Set log scale for y-axis
    ax.set_yscale('log')
    
    # Customize the plot
    ax.set_xlabel(get_stellar_mass_label(), fontsize=AXIS_LABEL_SIZE)
    ax.set_ylabel(r'Disk Radius (kpc)', fontsize=AXIS_LABEL_SIZE)
    
    # Set the x and y axis minor ticks
    ax.xaxis.set_minor_locator(MultipleLocator(0.5))
    
    # Set axis limits
    x_min = max(8.0, min(min(disk_mass) if len(disk_mass) > 0 else 10.0, 
                          min(bulge_mass) if len(bulge_mass) > 0 else 10.0) - 0.5)
    x_max = min(12.0, max(max(disk_mass) if len(disk_mass) > 0 else 10.0, 
                           max(bulge_mass) if len(bulge_mass) > 0 else 10.0) + 0.5)
    
    y_min = max(0.1, min(min(disk_radius) if len(disk_radius) > 0 else 1.0, 
                          min(bulge_radius) if len(bulge_radius) > 0 else 1.0) * 0.5)
    y_max = min(100.0, max(max(disk_radius) if len(disk_radius) > 0 else 10.0, 
                            max(bulge_radius) if len(bulge_radius) > 0 else 10.0) * 2.0)
    
    ax.set_xlim(x_min, x_max)
    ax.set_ylim(y_min, y_max)
    
    # Add consistently styled legend
    setup_legend(ax, loc='upper left')
    
    # Save the figure, ensuring the output directory exists
    try:
        os.makedirs(output_dir, exist_ok=True)
    except Exception as e:
        print(f"Warning: Could not create output directory {output_dir}: {e}")
        # Try to use a subdirectory of the current directory as fallback
        output_dir = './plots'
        os.makedirs(output_dir, exist_ok=True)
        
    output_path = os.path.join(output_dir, f"GalaxySize{output_format}")
    print(f"Saving Galaxy Size Relations plot to: {output_path}")
    plt.savefig(output_path)
    plt.close()
    
    return output_path
