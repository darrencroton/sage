/**
 * @file    core_allvars.c
 * @brief   Defines global variables used throughout the SAGE model
 *
 * This file contains the definitions of all global variables used by the
 * SAGE model. These variables fall into several categories:
 *
 * 1. Core data structures (galaxies, halos, and auxiliary data)
 * 2. Configuration parameters and derived values
 * 3. Simulation state variables (counts, indices, etc.)
 * 4. Physical constants and units
 * 5. File and output control variables
 *
 * Many of these global variables are being gradually migrated to the
 * SageConfig and SimState structures to improve encapsulation and make
 * the code more maintainable, but they are kept for backward compatibility
 * with existing code. New code should preferentially use the structured
 * approach rather than accessing these globals directly.
 *
 * Note: This file contains only variable definitions - the declarations
 * are in globals.h and other header files.
 */

#include "config.h"
#include "globals.h"
#include "types.h"

/*  Global configuration structure */
struct SageConfig SageConfig;

/*  Global simulation state structure */
struct SimulationState SimState;

/*  galaxy data  */
struct GALAXY *Gal, *HaloGal;

struct halo_data *Halo;

/*  auxiliary halo data  */
struct halo_aux_data *HaloAux;

/*  misc  */

int HDF5Output;
#ifdef HDF5
char *core_output_file;
size_t HDF5_dst_size;
size_t *HDF5_dst_offsets;
size_t *HDF5_dst_sizes;
const char **HDF5_field_names;
hid_t *HDF5_field_types;
int HDF5_n_props;
#endif

int MaxGals;
int FoF_MaxGals;
int Ntrees;  /*  number of trees in current file  */
int NumGals; /*  Total number of galaxies stored for current tree  */

int GalaxyCounter; /*  unique galaxy ID for main progenitor line in tree */

int TotHalos;
int TotGalaxies[ABSOLUTEMAXSNAPS];
int *TreeNgals[ABSOLUTEMAXSNAPS];

int LastSnapShotNr;
double BoxSize;

int *FirstHaloInSnap;
int *TreeNHalos;
int *TreeFirstHalo;

#ifdef MPI
int ThisTask, NTask, nodeNameLen;
char *ThisNode;
#endif

/*  recipe parameters  */
int NParam;
char ParamTag[MAXTAGS][50];
int ParamID[MAXTAGS];
void *ParamAddr[MAXTAGS];

/*  derived values from parameters */
double EnergySNcode;
double EtaSNcode;

/*  more misc - kept for backward compatibility */
double UnitLength_in_cm, UnitTime_in_s, UnitVelocity_in_cm_per_s, UnitMass_in_g,
    RhoCrit, UnitPressure_in_cgs, UnitDensity_in_cgs, UnitCoolingRate_in_cgs,
    UnitEnergy_in_cgs, UnitTime_in_Megayears, G, Hubble, a0, ar;

int ListOutputSnaps[ABSOLUTEMAXSNAPS];
double ZZ[ABSOLUTEMAXSNAPS];
double AA[ABSOLUTEMAXSNAPS];
double *Age;

int MAXSNAPS;
int NOUT;
int Snaplistlen;

/* derived values from parameters */
double EnergySNcode;
double EtaSNcode;

/* Random number generator removed - not used in computation */

int TreeID;
int FileNum;

enum Valid_TreeTypes TreeType;
