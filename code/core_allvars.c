#include "core_allvars.h"


// galaxy data 
struct GALAXY			
  *Gal, *HaloGal;

struct halo_data *Halo;

// auxiliary halo data 
struct halo_aux_data		
  *HaloAux;


// misc 
int FirstFile;
int LastFile;
int MaxGals;
int FoF_MaxGals;
int Ntrees;			   // number of trees in current file 
int NumGals;			 // Total number of galaxies stored for current tree 

int GalaxyCounter; // unique galaxy ID for main progenitor line in tree

char OutputDir[512];
char FileNameGalaxies[512];
char SimulationDir[512];
char FileWithOutputSnaps[512];
char FileWithSnapList[512];

int TotHalos;
int TotGalaxies[NOUT];
int *TreeNgals[NOUT];

int FilesPerSnapshot;
int LastSnapShotNr;

int *FirstHaloInSnap;
int *TreeNHalos;
int *TreeFirstHalo;

int ThisTask, NTask, nodeNameLen;
char *ThisNode;

double Omega;
double OmegaLambda;
double Hubble_h;
double PartMass;
double EnergySNcode, EnergySN;
double EtaSNcode, EtaSN;


// recipe flags 
int PopSynthModelOn;
int ReionizationOn;
int SupernovaRecipeOn;
int DiskInstabilityOn;
int AGNrecipeOn;
int SFprescription;


// recipe parameters 
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
double ClumpingFactor;


// more misc 
double UnitLength_in_cm,
  UnitTime_in_s,
  UnitVelocity_in_cm_per_s,
  UnitMass_in_g,
  RhoCrit,
  UnitPressure_in_cgs,
  UnitDensity_in_cgs, UnitCoolingRate_in_cgs, UnitEnergy_in_cgs, UnitTime_in_Megayears, G, Hubble, a0, ar;

int ListOutputSnaps[NOUT];

double ZZ[MAXSNAPS];
double AA[MAXSNAPS];
double Age[MAXSNAPS];

int Snaplistlen;

gsl_rng *random_generator;

int TreeID;
int FileNum;
