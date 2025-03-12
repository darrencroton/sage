#!/usr/bin/env python

"""
SAGE Specific Star Formation Rate Plot

This module generates a specific star formation rate plot from SAGE galaxy data.
"""

import os
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import MultipleLocator
from random import sample, seed
from figures import get_stellar_mass_label, get_ssfr_label

def plot(galaxies, volume, metadata, params, output_dir="plots", output_format=".png", dilute=7500):
    """
    Create a specific star formation rate plot.
    
    Args:
        galaxies: Galaxy data as a numpy recarray
        volume: Simulation volume in (Mpc/h)^3
        metadata: Dictionary with additional metadata
        params: Dictionary with SAGE parameters
        output_dir: Output directory for the plot
        output_format: File format for the output
        dilute: Maximum number of points to plot (for clarity)
    
    Returns:
        Path to the saved plot file
    """
    # Set random seed for reproducibility when diluting
    seed(2222)
    
    # Extract necessary metadata
    hubble_h = metadata['hubble_h']
    
    # Set up the figure
    fig, ax = plt.subplots(figsize=(8, 6))
    
    # Select galaxies with sufficient stellar mass
    w = np.where(galaxies.StellarMass > 0.01)[0]
    
    # Check if we have any galaxies to plot
    if len(w) == 0:
        print("No galaxies found with stellar mass > 0.01")
        # Create an empty plot with a message
        ax.text(0.5, 0.5, "No galaxies found with stellar mass > 0.01", 
                horizontalalignment='center', verticalalignment='center',
                transform=ax.transAxes)
        
        # Save the figure
        os.makedirs(output_dir, exist_ok=True)
        output_path = os.path.join(output_dir, f"SpecificSFR{output_format}")
        plt.savefig(output_path)
        plt.close()
        return output_path
    
    # Dilute the sample if needed
    if len(w) > dilute:
        w = sample(list(w), dilute)
    
    # Calculate stellar mass and specific SFR
    mass = np.log10(galaxies.StellarMass[w] * 1.0e10 / hubble_h)
    sfr = galaxies.SfrDisk[w] + galaxies.SfrBulge[w]
    ssfr = np.log10(sfr / (galaxies.StellarMass[w] * 1.0e10 / hubble_h))
    
    # Plot the model galaxies
    ax.scatter(mass, ssfr, marker='o', s=1, c='k', alpha=0.5, label='Model galaxies')
    
    # Add a horizontal line at the division between star-forming and quiescent galaxies
    ssfr_cut = -11.0
    ax.axhline(y=ssfr_cut, c='r', ls='--', lw=2)
    ax.text(11, ssfr_cut + 0.1, 'Star-forming', color='b')
    ax.text(11, ssfr_cut - 0.5, 'Quiescent', color='r')
    
    # Customize the plot
    ax.set_ylabel(get_ssfr_label())
    ax.set_xlabel(get_stellar_mass_label())
    
    # Set the axis limits and minor ticks
    ax.set_xlim(8.0, 12.0)
    ax.set_ylim(-14.0, -8.0)
    ax.xaxis.set_minor_locator(MultipleLocator(0.5))
    ax.yaxis.set_minor_locator(MultipleLocator(0.5))
    
    # Add legend
    leg = ax.legend(loc='upper right')
    leg.draw_frame(False)  # Don't want a box frame
    for t in leg.get_texts():  # Reduce the size of the text
        t.set_fontsize('medium')
    
    # Save the figure, ensuring the output directory exists
    try:
        os.makedirs(output_dir, exist_ok=True)
    except Exception as e:
        print(f"Warning: Could not create output directory {output_dir}: {e}")
        # Try to use a subdirectory of the current directory as fallback
        output_dir = './plots'
        os.makedirs(output_dir, exist_ok=True)
        
    output_path = os.path.join(output_dir, f"SpecificSFR{output_format}")
    print(f"Saving Specific SFR to: {output_path}")
    plt.savefig(output_path)
    plt.close()
    
    return output_path
