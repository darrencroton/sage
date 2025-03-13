#!/usr/bin/env python

"""
Setup script for the SAGE Plotting Tool package.

This file defines the package metadata, dependencies, and entry points
for the SAGE Plotting Tool, which provides a centralized system for
generating plots from SAGE galaxy formation model outputs.
"""

from setuptools import find_packages, setup

setup(
    name="sage-plot",
    version="0.1.0",
    description="Centralized plotting tool for the SAGE galaxy formation model",
    author="SAGE Team",
    packages=find_packages(),
    entry_points={
        "console_scripts": [
            "sage-plot=sage_plot:main",
        ],
    },
    install_requires=[
        "numpy",
        "matplotlib",
        "tqdm",
    ],
    python_requires=">=3.6",
)
