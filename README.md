# Semi-Analytic Galaxy Evolution (SAGE)

[![DOI](https://zenodo.org/badge/13542/darrencroton/sage.svg)](https://zenodo.org/badge/latestdoi/13542/darrencroton/sage)

SAGE is a publicly available code-base for modelling galaxy formation in a cosmological context. A description of the model and its default calibration results can be found in [Croton et al. (2016)](http://arxiv.org/abs/1601.04709). These calibration results can also be explored in an iPython notebook showcasing the key figures [here](https://github.com/darrencroton/sage/blob/master/output/SAGE_MM.ipynb). SAGE is a significant update to that previously used in [Croton et al. (2006)](http://arxiv.org/abs/astro-ph/0508046).

SAGE is written in C and was built to be modular and customisable. It will run on any N-body simulation whose trees are organised in a supported format and contain a minimum set of basic halo properties. For testing purposes, treefiles for the [mini-Millennium Simulation](http://arxiv.org/abs/astro-ph/0504097) are available [here](https://data-portal.hpc.swin.edu.au/dataset/mini-millennium-simulation). SAGE should compile on most systems out of the box, its only dependency being [GSL](http://www.gnu.org/software/gsl/).

Galaxy formation models built using SAGE on the Millennium, Bolshoi and GiggleZ simulations can be downloaded at the [Theoretical Astrophysical Observatory (TAO)](https://tao.asvo.org.au/). You can also find SAGE on [ascl.net](http://ascl.net/1601.006).

Questions and comments can be sent to Darren Croton: dcroton@astro.swin.edu.au.
