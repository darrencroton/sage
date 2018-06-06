#include "core_allvars.h"


/*  Parameters */
int HDF5Output;
#ifdef HDF5
char          *core_output_file;
size_t         HDF5_dst_size;
size_t        *HDF5_dst_offsets;
size_t        *HDF5_dst_sizes;
const char   **HDF5_field_names;
hid_t         *HDF5_field_types;
int            HDF5_n_props;
#endif

int FirstFile;
int LastFile;
char OutputDir[MAX_STRING_LEN];
char FileNameGalaxies[MAX_STRING_LEN];
char TreeName[MAX_STRING_LEN];
char TreeExtension[MAX_STRING_LEN] = {'\0'}; /* If the FileType is HDF5 they will have .hdf5 extension, otherwise nothing. */
char SimulationDir[MAX_STRING_LEN];
char FileWithSnapList[MAX_STRING_LEN];

#ifdef OLD_VERSION
int Ntrees;			   /*  number of trees in current file  */
int *TreeNHalos;
int *TreeFirstHalo;
/*  galaxy data  */
struct GALAXY *Gal, *HaloGal;

struct halo_data *Halo;

/*  auxiliary halo data  */
struct halo_aux_data  *HaloAux;
int MaxGals;
int FoF_MaxGals;
int NumGals;			 /*  Total number of galaxies stored for current tree  */

int GalaxyCounter; /*  unique galaxy ID for main progenitor line in tree */
int TotGalaxies[ABSOLUTEMAXSNAPS];
int *TreeNgals[ABSOLUTEMAXSNAPS];

#endif



#ifdef MPI
int ThisTask, NTask, nodeNameLen;
char *ThisNode;
#endif

double Omega;
double OmegaLambda;
double Hubble_h;
double PartMass;
double EnergySNcode, EnergySN;
double EtaSNcode, EtaSN;
double BoxSize;

/*  recipe flags  */
int ReionizationOn;
int SupernovaRecipeOn;
int DiskInstabilityOn;
int AGNrecipeOn;
int SFprescription;


double RecycleFraction;
double Yield;
double FracZleaveDisk;
double ReIncorporationFactor;
double ThreshMajorMerger;
double BaryonFrac;
double SfrEfficiency;
double FeedbackReheatingEpsilon;
double FeedbackEjectionEfficiency;
double RadioModeEfficiency;
double QuasarModeEfficiency;
double BlackHoleGrowthRate;
double Reionization_z0;
double Reionization_zr;
double ThresholdSatDisruption;


/*  more misc  */
double UnitLength_in_cm,
  UnitTime_in_s,
  UnitVelocity_in_cm_per_s,
  UnitMass_in_g,
  RhoCrit,
  UnitPressure_in_cgs,
  UnitDensity_in_cgs, UnitCoolingRate_in_cgs, UnitEnergy_in_cgs, UnitTime_in_Megayears, G, Hubble, a0, ar;

int ListOutputSnaps[ABSOLUTEMAXSNAPS];

double ZZ[ABSOLUTEMAXSNAPS];
double AA[ABSOLUTEMAXSNAPS];
double *Age;

int MAXSNAPS;
int NOUT;
int Snaplistlen;
int LastSnapShotNr;

gsl_rng *random_generator;

int TreeID;
int FileNum;
