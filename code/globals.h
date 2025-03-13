#ifndef GLOBALS_H
#define GLOBALS_H

#include "constants.h"
#include "types.h"
#include <stdio.h>

/* Global configuration structure */
extern struct SageConfig SageConfig;

/* Global simulation state structure */
extern struct SimulationState SimState;

#ifdef MPI
extern int ThisTask, NTask, nodeNameLen;
extern char *ThisNode;
#endif

/* galaxy data pointers */
extern struct GALAXY *Gal, *HaloGal;
extern struct halo_data *Halo;
extern struct halo_aux_data *HaloAux;

/* runtime file information */
extern int Ntrees;  /* number of trees in current file  */
extern int NumGals; /* Total number of galaxies stored for current tree */
extern int MaxGals; /* Maximum number of galaxies allowed for current tree */
extern int FoF_MaxGals;
extern int
    GalaxyCounter; /* unique galaxy ID for main progenitor line in tree */

/* halo information */
extern int TotHalos;
extern int TotGalaxies[ABSOLUTEMAXSNAPS];
extern int *TreeNgals[ABSOLUTEMAXSNAPS];
extern int *FirstHaloInSnap;
extern int *TreeNHalos;
extern int *TreeFirstHalo;

/* parameter handling globals */
extern int NParam;
extern char ParamTag[MAXTAGS][50];
extern int ParamID[MAXTAGS];
extern void *ParamAddr[MAXTAGS];

/* derived values from parameters */
extern double EnergySNcode, EtaSNcode;

/* units */
extern double UnitLength_in_cm, UnitTime_in_s, UnitVelocity_in_cm_per_s,
    UnitMass_in_g, RhoCrit, UnitPressure_in_cgs, UnitDensity_in_cgs,
    UnitCoolingRate_in_cgs, UnitEnergy_in_cgs, UnitTime_in_Megayears, G, Hubble,
    a0, ar;

/* output snapshots - kept for backward compatibility */
extern int ListOutputSnaps[ABSOLUTEMAXSNAPS];
extern double ZZ[ABSOLUTEMAXSNAPS];
extern double AA[ABSOLUTEMAXSNAPS];
extern double *Age;
extern int MAXSNAPS;
extern int NOUT;
extern int Snaplistlen;

/* Random number generator removed - not used in computation */

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
