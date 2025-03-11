#!/usr/bin/env python

from setuptools import setup, find_packages

setup(
    name="sage-plot",
    version="0.1.0",
    description="Centralized plotting tool for the SAGE galaxy formation model",
    author="SAGE Team",
    packages=find_packages(),
    entry_points={
        'console_scripts': [
            'sage-plot=sage_plot:main',
        ],
    },
    install_requires=[
        'numpy',
        'matplotlib',
        'tqdm',
    ],
    python_requires='>=3.6',
)
