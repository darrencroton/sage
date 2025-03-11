#!/usr/bin/env python

"""
SAGE Baryonic Tully-Fisher Relationship Plot

This module generates a baryonic Tully-Fisher plot from SAGE galaxy data.
"""

import os
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import MultipleLocator
from random import sample, seed

def plot(galaxies, volume, metadata, params, output_dir="plots", output_format=".png", dilute=7500):
    """
    Create a baryonic Tully-Fisher plot.
    
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
    
    # Select Sb/c galaxies (Type=0 and bulge/total ratio between 0.1 and 0.5)
    w = np.where((galaxies.Type == 0) & 
                 (galaxies.StellarMass + galaxies.ColdGas > 0.0) & 
                 (galaxies.BulgeMass / galaxies.StellarMass > 0.1) & 
                 (galaxies.BulgeMass / galaxies.StellarMass < 0.5))[0]
    
    # Check if we have any galaxies to plot
    if len(w) == 0:
        print("No suitable galaxies found for Tully-Fisher plot")
        # Create an empty plot with a message
        ax.text(0.5, 0.5, "No suitable galaxies found for Tully-Fisher plot", 
                horizontalalignment='center', verticalalignment='center',
                transform=ax.transAxes)
        
        # Save the figure
        os.makedirs(output_dir, exist_ok=True)
        output_path = os.path.join(output_dir, f"BaryonicTullyFisher{output_format}")
        plt.savefig(output_path)
        plt.close()
        return output_path
    
    # Dilute the sample if needed
    if len(w) > dilute:
        w = sample(list(w), dilute)
    
    # Calculate baryonic mass and max velocity
    mass = np.log10((galaxies.StellarMass[w] + galaxies.ColdGas[w]) * 1.0e10 / hubble_h)
    vel = np.log10(galaxies.Vmax[w])
    
    # Plot the model galaxies
    ax.scatter(vel, mass, marker='o', s=1, c='k', alpha=0.5, label='Model Sb/c galaxies')
    
    # Plot Stark, McGaugh & Swatters 2009 relation
    w_obs = np.arange(0.5, 10.0, 0.5)
    TF = 3.94 * w_obs + 1.79
    ax.plot(w_obs, TF, 'b-', lw=2.0, label='Stark, McGaugh \\& Swatters 2009')
    
    # Customize the plot
    ax.set_ylabel(r'$\log_{10}\ M_{\mathrm{bar}}\ (M_{\odot})$')
    ax.set_xlabel(r'$\log_{10}V_{max}\ (km/s)$')
    
    # Set the axis limits and minor ticks
    ax.set_xlim(1.4, 2.6)
    ax.set_ylim(8.0, 12.0)
    ax.xaxis.set_minor_locator(MultipleLocator(0.05))
    ax.yaxis.set_minor_locator(MultipleLocator(0.25))
    
    # Add legend
    leg = ax.legend(loc='lower right')
    leg.draw_frame(False)  # Don't want a box frame
    for t in leg.get_texts():  # Reduce the size of the text
        t.set_fontsize('medium')
    
    # Save the figure
    os.makedirs(output_dir, exist_ok=True)
    output_path = os.path.join(output_dir, f"BaryonicTullyFisher{output_format}")
    plt.savefig(output_path)
    plt.close()
    
    return output_path
