#!/usr/bin/env python

"""
SAGE Baryonic Mass Function Plot

This module generates a baryonic mass function plot from SAGE galaxy data.
"""

import os
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import MultipleLocator

def plot(galaxies, volume, metadata, params, output_dir="plots", output_format=".png"):
    """
    Create a baryonic mass function plot.
    
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
    # Extract necessary metadata
    hubble_h = metadata['hubble_h']
    
    # Determine IMF type from params
    whichimf = params.get('IMF_Type', 1)  # Default to Chabrier IMF
    
    # Set up the figure
    fig, ax = plt.subplots(figsize=(8, 6))
    
    # Set up binning
    binwidth = 0.1  # mass function histogram bin width
    
    # Prepare data
    w = np.where((galaxies.StellarMass + galaxies.ColdGas) > 0.0)[0]
    
    # Check if we have any galaxies to plot
    if len(w) == 0:
        print("No galaxies found with baryonic mass > 0.0")
        # Create an empty plot with a message
        ax.text(0.5, 0.5, "No galaxies found with baryonic mass > 0.0", 
                horizontalalignment='center', verticalalignment='center',
                transform=ax.transAxes)
        
        # Save the figure
        os.makedirs(output_dir, exist_ok=True)
        output_path = os.path.join(output_dir, f"BaryonicMassFunction{output_format}")
        plt.savefig(output_path)
        plt.close()
        return output_path
        
    mass = np.log10((galaxies.StellarMass[w] + galaxies.ColdGas[w]) * 1.0e10 / hubble_h)
    
    # Set up histogram bins
    mi = np.floor(min(mass)) - 2
    ma = np.floor(max(mass)) + 2
    nbins = int((ma - mi) / binwidth)
    
    # Calculate histogram for all galaxies
    counts, binedges = np.histogram(mass, range=(mi, ma), bins=nbins)
    xaxis = binedges[:-1] + 0.5 * binwidth
    
    # Plot the main histogram
    ax.plot(
        xaxis, 
        counts / volume * hubble_h**3 / binwidth, 
        'k-', 
        label='Model'
    )
    
    # Bell et al. 2003 BMF (h=1.0 converted to h=0.73)
    M = np.arange(7.0, 13.0, 0.01)
    Mstar = np.log10(5.3*1.0e10 / hubble_h / hubble_h)
    alpha = -1.21
    phistar = 0.0108 * hubble_h**3
    xval = 10.0 ** (M - Mstar)
    yval = np.log(10.) * phistar * xval ** (alpha+1) * np.exp(-xval)
    
    if whichimf == 0:
        # converted diet Salpeter IMF to Salpeter IMF
        ax.plot(np.log10(10.0**M / 0.7), yval, 'b-', lw=2.0, label='Bell et al. 2003')
    elif whichimf == 1:
        # converted diet Salpeter IMF to Salpeter IMF, then to Chabrier IMF
        ax.plot(np.log10(10.0**M / 0.7 / 1.8), yval, 'g--', lw=1.5, label='Bell et al. 2003')
    
    # Customize the plot
    ax.set_yscale('log')
    ax.set_xlim(8.0, 12.5)
    ax.set_ylim(1.0e-6, 1.0e-1)
    ax.xaxis.set_minor_locator(MultipleLocator(0.1))
    
    ax.set_ylabel(r'$\phi\ (\mathrm{Mpc}^{-3}\ \mathrm{dex}^{-1})$')
    ax.set_xlabel(r'$\log_{10}\ M_{\mathrm{bar}}\ (M_{\odot})$')
    
    # Add legend
    leg = ax.legend(loc='lower left', numpoints=1, labelspacing=0.1)
    leg.draw_frame(False)  # Don't want a box frame
    for t in leg.get_texts():  # Reduce the size of the text
        t.set_fontsize('medium')
    
    # Save the figure
    os.makedirs(output_dir, exist_ok=True)
    output_path = os.path.join(output_dir, f"BaryonicMassFunction{output_format}")
    plt.savefig(output_path)
    plt.close()
    
    return output_path
