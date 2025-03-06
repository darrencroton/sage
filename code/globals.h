#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdio.h>
#include <gsl/gsl_rng.h>
#include "constants.h"
#include "types.h"

#ifdef MPI
extern int ThisTask, NTask, nodeNameLen;
extern char *ThisNode;
#endif

/* galaxy data pointers */
extern struct GALAXY *Gal, *HaloGal;
extern struct halo_data *Halo;
extern struct halo_aux_data *HaloAux;

/* file information */
extern int FirstFile;    /* first and last file for processing */
extern int LastFile;
extern int Ntrees;      /* number of trees in current file  */
extern int NumGals;     /* Total number of galaxies stored for current tree */
extern int MaxGals;     /* Maximum number of galaxies allowed for current tree */  
extern int FoF_MaxGals;
extern int GalaxyCounter;     /* unique galaxy ID for main progenitor line in tree */
extern int LastSnapShotNr;
extern double BoxSize;

/* paths */
extern char OutputDir[MAX_STRING_LEN];
extern char FileNameGalaxies[MAX_STRING_LEN];
extern char TreeName[MAX_STRING_LEN];
extern char TreeExtension[MAX_STRING_LEN]; // If the trees are in HDF5, they will have a .hdf5 extension. Otherwise they have no extension.
extern char SimulationDir[MAX_STRING_LEN];
extern char FileWithSnapList[MAX_STRING_LEN];

/* halo information */
extern int TotHalos;
extern int TotGalaxies[ABSOLUTEMAXSNAPS];
extern int *TreeNgals[ABSOLUTEMAXSNAPS];
extern int *FirstHaloInSnap;
extern int *TreeNHalos;
extern int *TreeFirstHalo;

/* cosmological parameters */
extern double Omega;
extern double OmegaLambda;
extern double PartMass;
extern double Hubble_h;
extern double EnergySNcode, EnergySN;
extern double EtaSNcode, EtaSN;

/* recipe parameters */
extern int NParam;
extern char ParamTag[MAXTAGS][50];
extern int ParamID[MAXTAGS];
extern void *ParamAddr[MAXTAGS];
extern double RecycleFraction;
extern double Yield;
extern double FracZleaveDisk;
extern double ReIncorporationFactor;
extern double ThreshMajorMerger;
extern double BaryonFrac;
extern double SfrEfficiency;
extern double FeedbackReheatingEpsilon;
extern double FeedbackEjectionEfficiency;
extern double RadioModeEfficiency;
extern double QuasarModeEfficiency;
extern double BlackHoleGrowthRate;
extern double Reionization_z0;
extern double Reionization_zr;
extern double ThresholdSatDisruption;

/* units */
extern double UnitLength_in_cm,
  UnitTime_in_s,
  UnitVelocity_in_cm_per_s,
  UnitMass_in_g,
  RhoCrit,
  UnitPressure_in_cgs,
  UnitDensity_in_cgs,
  UnitCoolingRate_in_cgs,
  UnitEnergy_in_cgs,
  UnitTime_in_Megayears, 
  G,
  Hubble,
  a0, ar;

/* output snapshots */
extern int ListOutputSnaps[ABSOLUTEMAXSNAPS];
extern double ZZ[ABSOLUTEMAXSNAPS];
extern double AA[ABSOLUTEMAXSNAPS];
extern double *Age;
extern int MAXSNAPS;
extern int NOUT;
extern int Snaplistlen;

/* random number generator */
extern gsl_rng *random_generator;

/* tree and file information */
extern int TreeID;
extern int FileNum;

/* HDF5 specific globals */
#ifdef HDF5
extern int HDF5Output;
extern char *core_output_file;
extern size_t HDF5_dst_size;
extern size_t *HDF5_dst_offsets;
extern size_t *HDF5_dst_sizes;
extern const char **HDF5_field_names;
extern hid_t *HDF5_field_types;
extern int HDF5_n_props;
#endif

#endif /* #ifndef GLOBALS_H */
