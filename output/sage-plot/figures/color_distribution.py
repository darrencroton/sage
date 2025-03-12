#!/usr/bin/env python

"""
SAGE Galaxy Color Distribution Plot

This module generates a color-magnitude diagram for SAGE galaxy data.
"""

import os
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import MultipleLocator
import random
from figures import setup_plot_fonts, setup_legend, AXIS_LABEL_SIZE, LEGEND_FONT_SIZE, IN_FIGURE_TEXT_SIZE

def plot(galaxies, volume, metadata, params, output_dir="plots", output_format=".png"):
    """
    Create a galaxy color distribution plot (color-magnitude diagram).
    
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
    
    # Filter for galaxies with non-zero stellar mass
    w = np.where(galaxies.StellarMass > 0.01)[0]
    
    # Check if we have any galaxies to plot
    if len(w) == 0:
        print("No galaxies found with stellar mass > 0.01")
        # Create an empty plot with a message
        ax.text(0.5, 0.5, "No galaxies found with stellar mass > 0.01", 
                horizontalalignment='center', verticalalignment='center',
                transform=ax.transAxes, fontsize=IN_FIGURE_TEXT_SIZE)
        
        # Save the figure
        os.makedirs(output_dir, exist_ok=True)
        output_path = os.path.join(output_dir, f"ColorDistribution{output_format}")
        plt.savefig(output_path)
        plt.close()
        return output_path
    
    # If we have too many galaxies, randomly sample a subset
    if len(w) > dilute:
        w = random.sample(list(w), dilute)
    
    # SAGE doesn't directly output galaxy colors, so we'll estimate them
    # using a simple approach based on the sSFR (a proxy for color)
    # More sophisticated color models would require stellar population synthesis
    
    # Calculate stellar mass (magnitude proxy) and sSFR (color proxy)
    stellar_mass = np.log10(galaxies.StellarMass[w] * 1.0e10 / hubble_h)
    
    # Convert to an absolute magnitude proxy (higher mass = brighter = more negative mag)
    # This is just a rough approximation: M = -2.5 * log10(mass) + constant
    abs_mag = -2.5 * stellar_mass + 25.0
    
    # Calculate specific SFR as a proxy for color
    sfr = galaxies.SfrDisk[w] + galaxies.SfrBulge[w]
    ssfr = sfr / (galaxies.StellarMass[w] * 1.0e10 / hubble_h)
    
    # Convert sSFR to a color index (lower sSFR = redder color)
    # We'll use log10(sSFR) as our color proxy, with a cutoff for very low values
    log_ssfr = np.log10(np.maximum(ssfr, 1e-15))
    
    # Define the dividing line between red and blue galaxies
    ssfr_cut = -11.0  # Same as used in other plots
    
    # Create separate arrays for red and blue galaxies
    is_red = log_ssfr < ssfr_cut
    
    # Print some debug information
    print(f"Color Distribution plot debug:")
    print(f"  Number of galaxies plotted: {len(w)}")
    print(f"  Red sequence galaxies: {np.sum(is_red)}")
    print(f"  Blue cloud galaxies: {np.sum(~is_red)}")
    print(f"  Absolute magnitude range: {min(abs_mag):.2f} to {max(abs_mag):.2f}")
    print(f"  log10(sSFR) range: {min(log_ssfr):.2f} to {max(log_ssfr):.2f}")
    
    # Plot the color-magnitude diagram as a scatter plot
    # Red sequence galaxies
    ax.scatter(abs_mag[is_red], log_ssfr[is_red], marker='o', s=2, c='r', alpha=0.5, label='Red sequence')
    
    # Blue cloud galaxies
    ax.scatter(abs_mag[~is_red], log_ssfr[~is_red], marker='o', s=2, c='b', alpha=0.5, label='Blue cloud')
    
    # Mark the division between red and blue galaxies
    x_range = np.array([min(abs_mag) - 1, max(abs_mag) + 1])
    ax.plot(x_range, [ssfr_cut, ssfr_cut], 'k--', lw=1.0, alpha=0.7, label='Dividing line')
    
    # Customize the plot
    ax.set_xlabel(r'M$_r$ (pseudo-magnitude)', fontsize=AXIS_LABEL_SIZE)
    ax.set_ylabel(r'log$_{10}$ sSFR (yr$^{-1}$)', fontsize=AXIS_LABEL_SIZE)
    
    # Set the x and y axis minor ticks
    ax.xaxis.set_minor_locator(MultipleLocator(0.5))
    ax.yaxis.set_minor_locator(MultipleLocator(0.5))
    
    # Set axis limits 
    ax.set_xlim(min(abs_mag) - 0.5, max(abs_mag) + 0.5)
    ax.set_ylim(-16.0, -8.0)
    
    # Invert x-axis (standard for magnitude plots: brighter to the right)
    ax.invert_xaxis()
    
    # Add consistently styled legend
    setup_legend(ax, loc='upper right')
    
    # Add annotation explaining the color proxy
    ax.text(0.05, 0.05, 
            "Note: Color proxy based on specific star formation rate",
            transform=ax.transAxes, fontsize=10, alpha=0.7)
    
    # Save the figure, ensuring the output directory exists
    try:
        os.makedirs(output_dir, exist_ok=True)
    except Exception as e:
        print(f"Warning: Could not create output directory {output_dir}: {e}")
        # Try to use a subdirectory of the current directory as fallback
        output_dir = './plots'
        os.makedirs(output_dir, exist_ok=True)
        
    output_path = os.path.join(output_dir, f"ColorDistribution{output_format}")
    print(f"Saving Galaxy Color Distribution plot to: {output_path}")
    plt.savefig(output_path)
    plt.close()
    
    return output_path
