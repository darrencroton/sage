##Semi-Analytic Galaxy Evolution (SAGE)

[![DOI](https://zenodo.org/badge/13542/darrencroton/sage.svg)](https://zenodo.org/badge/latestdoi/13542/darrencroton/sage)

SAGE is a publicly available codebase for modelling galaxy formation in a cosmological context, described [here](http://arxiv.org/abs/1601.04709). It is a significant update to that used in [Croton et al. (2006)](http://arxiv.org/abs/astro-ph/0508046) and has been rebuilt to be modular and customisable. SAGE will run on any N-body simulation whose trees are organised in a supported format and contain a minimum set of basic halo properties. 

SAGE is written in C and should compile on most systems out of the box, its only dependency being [GSL](http://www.gnu.org/software/gsl/). For testing purposes, treefiles for the mini-[Millennium Simulation](http://arxiv.org/abs/astro-ph/0504097) are available [here](http://supercomputing.swin.edu.au/data-sharing-cluster/mini-millennium-simulation/).

A description of the model and its default calibration results can be found in [Croton et al. (2016)](http://arxiv.org/abs/1601.04709). These calibration results can also be explored in an iPython notebook showcasing the key figures [here](https://github.com/darrencroton/sage/blob/master/output/SAGE_MM.ipynb). Galaxy formation models built using SAGE on the Millennium, Bolshoi and GiggleZ simulations can be downloaded at the [Theoretical Astrophysical Observatory (TAO)](https://tao.asvo.org.au/). You can also find SAGE on [ascl.net](http://ascl.net/code/v/1298).

Questions and comments can be sent to Darren Croton: dcroton@astro.swin.edu.au.
