# Dusty SAGE

Dusty SAGE is a semi-analytic model for galaxy formation that include a detailed prescription for dust evolution. This model is a modification of Semi-Analytic Galaxy Evolution [(SAGE)](https://github.com/darrencroton/sage) described [Croton et al. (2016)](http://arxiv.org/abs/1601.04709). The description of Dusty SAGE is given in [Triani et al. (2020)](https://arxiv.org/abs/2002.05343).

Dusty SAGE runs on any N-body simulation whose trees are organised in a supported format and contain a minimum set of basic halo properties. For testing purposes, treefiles for the [mini-Millennium Simulation](http://arxiv.org/abs/astro-ph/0504097) are available [here](https://data-portal.hpc.swin.edu.au/dataset/mini-millennium-simulation). 

## Installation 
```SAGE``` requires only GSL and should compile mostly out of the box.
### Downloading
```
$ git clone github.com/dptriani/dusty-sage
```
### Building
```
$ cd dusty-sage
$ make
```
### Running
To run Dusty SAGE, you need to specify the parameters needed by the model in the parameter file (.par). An example of a parameter file is given [here](https://github.com/dptriani/dusty-sage/blob/master/src/auxdata/trees/mini-millennium/mini-millennium.par). To run the model:
```
$ ./sage <path to .par file>
```

## Citation
If you use Dusty SAGE in a publication, please cite the following items:

[Triani et al. (2020)](https://arxiv.org/abs/2002.05343)

If you use SAGE in a publication, please cite the following items:

[Croton et al. (2016)](http://arxiv.org/abs/1601.04709)


## Maintainer 

Questions and comments can be sent to Dian Triani: dtriani@swin.edu.au.
