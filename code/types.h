#ifndef TYPES_H
#define TYPES_H

#include "constants.h"
#include "core_simulation.h"

/* Enum for tree types */
enum Valid_TreeTypes {
  genesis_lhalo_hdf5 = 0,
  lhalo_binary = 1,
  num_tree_types
};

/* Configuration structure to hold global parameters */
struct SageConfig {
  /* file information */
  int FirstFile; /* first and last file for processing */
  int LastFile;
  int LastSnapshotNr;
  double BoxSize;

  /* paths */
  char OutputDir[MAX_STRING_LEN];
  char FileNameGalaxies[MAX_STRING_LEN];
  char TreeName[MAX_STRING_LEN];
  char TreeExtension[MAX_STRING_LEN];
  char SimulationDir[MAX_STRING_LEN];
  char FileWithSnapList[MAX_STRING_LEN];

  /* cosmological parameters */
  double Omega;
  double OmegaLambda;
  double PartMass;
  double Hubble_h;

  /* flags */
  int OverwriteOutputFiles; // Flag: 1=overwrite (default), 0=skip existing
                            // files

  /* output parameters */
  int NOUT;
  int ListOutputSnaps[ABSOLUTEMAXSNAPS];
  double ZZ[ABSOLUTEMAXSNAPS];
  double AA[ABSOLUTEMAXSNAPS];
  int MAXSNAPS;
  int Snaplistlen;

  /* units */
  double UnitLength_in_cm;
  double UnitTime_in_s;
  double UnitVelocity_in_cm_per_s;
  double UnitMass_in_g;
  double UnitTime_in_Megayears;
  double UnitPressure_in_cgs;
  double UnitDensity_in_cgs;
  double UnitCoolingRate_in_cgs;
  double UnitEnergy_in_cgs;

  /* derived parameters */
  double RhoCrit;
  double G;
  double Hubble;

  /* Tree type */
  enum Valid_TreeTypes TreeType;
};

/* This structure contains the properties that are output */
struct GALAXY_OUTPUT {
  int SnapNum;
  int Type;

  long long GalaxyIndex;
  long long CentralGalaxyIndex;
  int SAGEHaloIndex;
  int SAGETreeIndex;
  long long SimulationHaloIndex;

  int MergeStatus; /* 0=halo is active; 1=halo has merged and is no longer tracked */
  int mergeIntoID;
  int mergeIntoSnapNum;
  float dT;

  /* (sub)halo properties */
  float Pos[3];
  float Vel[3];
  float Spin[3];
  int Len;
  float Mvir;
  float CentralMvir;
  float Rvir;
  float Vvir;
  float Vmax;
  float VelDisp;

  /* infall properties */
  float infallMvir;
  float infallVvir;
  float infallVmax;
};

/* This structure contains the properties used within the code */
struct GALAXY {
  int SnapNum;
  int Type;

  int GalaxyNr;
  int CentralGal;
  int HaloNr;
  long long MostBoundID;

  int MergeStatus; /* 0=halo is active; 1=halo has merged and is no longer tracked */
  int mergeIntoID;
  int mergeIntoSnapNum;
  float dT;

  /* (sub)halo properties */
  float Pos[3];
  float Vel[3];
  int Len;
  float Mvir;
  float deltaMvir;
  float CentralMvir;
  float Rvir;
  float Vvir;
  float Vmax;

  /* misc */
  float MergTime;

  /* infall properties */
  float infallMvir;
  float infallVvir;
  float infallVmax;
};

/* auxiliary halo data */
struct halo_aux_data {
  int DoneFlag;
  int HaloFlag;
  int NGalaxies;
  int FirstGalaxy;
};

/* Structure to hold runtime simulation state */
struct SimulationState {
  /* Tree and galaxy counts */
  int Ntrees;        /* number of trees in current file */
  int NumGals;       /* Total number of galaxies stored for current tree */
  int MaxGals;       /* Maximum number of galaxies allowed for current tree */
  int FoF_MaxGals;   /* Maximum number of galaxies for FoF groups */
  int GalaxyCounter; /* unique galaxy ID for main progenitor line in tree */
  int TotHalos;      /* Total number of halos */
  int TotGalaxies[ABSOLUTEMAXSNAPS]; /* Galaxy count per snapshot */

  /* File and tree identifiers */
  int FileNum; /* Current file number being processed */
  int TreeID;  /* Current tree ID being processed */

  /* Snapshot information */
  int MAXSNAPS;                          /* Maximum number of snapshots */
  int Snaplistlen;                       /* Length of snapshot list */
  int NOUT;                              /* Number of outputs */
  int ListOutputSnaps[ABSOLUTEMAXSNAPS]; /* List of output snapshot numbers */

  /* Tree structure pointers */
  int *
      TreeNgals[ABSOLUTEMAXSNAPS]; /* Array of galaxies per tree per snapshot */
  int *TreeNHalos;                 /* Array of halos per tree */
  int *TreeFirstHalo;              /* Array of first halo in each tree */
  int *FirstHaloInSnap;            /* Array of first halo in each snapshot */
};

#endif /* #ifndef TYPES_H */
