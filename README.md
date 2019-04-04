# Semi-Analytic Galaxy Evolution (SAGE)

[![Build Status](https://travis-ci.org/manodeep/sage.svg?branch=lhvt)](https://travis-ci.org/manodeep/sage)
[![DOI](https://zenodo.org/badge/13542/darrencroton/sage.svg)](https://zenodo.org/badge/latestdoi/13542/darrencroton/sage)

SAGE is a publicly available code-base for modelling galaxy formation in a cosmological context. A description of the model and its default calibration results can be found in [Croton et al. (2016)](http://arxiv.org/abs/1601.04709). These calibration results can also be explored in an iPython notebook showcasing the key figures [here](https://github.com/darrencroton/sage/blob/master/output/SAGE_MM.ipynb). SAGE is a significant update to that previously used in [Croton et al. (2006)](http://arxiv.org/abs/astro-ph/0508046).

SAGE is written in C and was built to be modular and customisable. It will run on any N-body simulation whose trees are organised in a supported format and contain a minimum set of basic halo properties. For testing purposes, treefiles for the [mini-Millennium Simulation](http://arxiv.org/abs/astro-ph/0504097) are available [here](https://data-portal.hpc.swin.edu.au/dataset/mini-millennium-simulation). 

Galaxy formation models built using SAGE on the Millennium, Bolshoi and GiggleZ simulations can be downloaded at the [Theoretical Astrophysical Observatory (TAO)](https://tao.asvo.org.au/). You can also find SAGE on [ascl.net](http://ascl.net/1601.006).

## Installation 

SAGE should compile on most systems out of the box and the only required tool is a [C99  compiler](https://en.wikipedia.org/wiki/C99). [GSL](http://www.gnu.org/software/gsl/) is recommended but not necessary. Optionally, SAGE can read in trees in [HDF5](https://support.hdfgroup.org/HDF5/) format if compiled with HDF5 library support.

## Citation

If you use SAGE in a publication, please cite the following items:

```
@ARTICLE{2016ApJS..222...22C,
   author = {{Croton}, D.~J. and {Stevens}, A.~R.~H. and {Tonini}, C. and 
	{Garel}, T. and {Bernyk}, M. and {Bibiano}, A. and {Hodkinson}, L. and 
	{Mutch}, S.~J. and {Poole}, G.~B. and {Shattow}, G.~M.},
    title = "{Semi-Analytic Galaxy Evolution (SAGE): Model Calibration and Basic Results}",
  journal = {\apjs},
archivePrefix = "arXiv",
   eprint = {1601.04709},
 keywords = {galaxies: active, galaxies: evolution, galaxies: halos, methods: numerical},
     year = 2016,
    month = feb,
   volume = 222,
      eid = {22},
    pages = {22},
      doi = {10.3847/0067-0049/222/2/22},
   adsurl = {http://adsabs.harvard.edu/abs/2016ApJS..222...22C},
  adsnote = {Provided by the SAO/NASA Astrophysics Data System}
}
```


## Maintainer 

Questions and comments can be sent to Darren Croton: dcroton@astro.swin.edu.au.
