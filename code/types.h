#ifndef TYPES_H
#define TYPES_H

#include "constants.h"
#include "core_simulation.h"

/* Enum for tree types */
enum Valid_TreeTypes
{
  genesis_lhalo_hdf5 = 0,
  lhalo_binary = 1,
  num_tree_types
};

/* This structure contains the properties that are output */
struct GALAXY_OUTPUT  
{
  int   SnapNum;
  int   Type;

  long long   GalaxyIndex;
  long long   CentralGalaxyIndex;
  int   SAGEHaloIndex;
  int   SAGETreeIndex;
  long long   SimulationHaloIndex;
  
  int   mergeType;  /* 0=none; 1=minor merger; 2=major merger; 3=disk instability; 4=disrupt to ICS */
  int   mergeIntoID;
  int   mergeIntoSnapNum;
  float dT;

  /* (sub)halo properties */
  float Pos[3];
  float Vel[3];
  float Spin[3];
  int   Len;   
  float Mvir;
  float CentralMvir;
  float Rvir;
  float Vvir;
  float Vmax;
  float VelDisp;

  /* baryonic reservoirs */
  float ColdGas;
  float StellarMass;
  float BulgeMass;
  float HotGas;
  float EjectedMass;
  float BlackHoleMass;
  float ICS;

  /* metals */
  float MetalsColdGas;
  float MetalsStellarMass;
  float MetalsBulgeMass;
  float MetalsHotGas;
  float MetalsEjectedMass;
  float MetalsICS;

  /* to calculate magnitudes */
  float SfrDisk;
  float SfrBulge;
  float SfrDiskZ;
  float SfrBulgeZ;
  
  /* misc */
  float DiskScaleRadius;
  float Cooling;
  float Heating;
  float QuasarModeBHaccretionMass;
  float TimeOfLastMajorMerger;
  float TimeOfLastMinorMerger;
  float OutflowRate;

  /* infall properties */
  float infallMvir;
  float infallVvir;
  float infallVmax;
};

/* This structure contains the properties used within the code */
struct GALAXY
{
  int   SnapNum;
  int   Type;

  int   GalaxyNr;
  int   CentralGal;
  int   HaloNr;
  long long MostBoundID;

  int   mergeType;  /* 0=none; 1=minor merger; 2=major merger; 3=disk instability; 4=disrupt to ICS */
  int   mergeIntoID;
  int   mergeIntoSnapNum;
  float dT;

  /* (sub)halo properties */
  float Pos[3];
  float Vel[3];
  int   Len;   
  float Mvir;
  float deltaMvir;
  float CentralMvir;
  float Rvir;
  float Vvir;
  float Vmax;

  /* baryonic reservoirs */
  float ColdGas;
  float StellarMass;
  float BulgeMass;
  float HotGas;
  float EjectedMass;
  float BlackHoleMass;
  float ICS;

  /* metals */
  float MetalsColdGas;
  float MetalsStellarMass;
  float MetalsBulgeMass;
  float MetalsHotGas;
  float MetalsEjectedMass;
  float MetalsICS;

  /* to calculate magnitudes */
  float SfrDisk[STEPS];
  float SfrBulge[STEPS];
  float SfrDiskColdGas[STEPS];
  float SfrDiskColdGasMetals[STEPS];
  float SfrBulgeColdGas[STEPS];
  float SfrBulgeColdGasMetals[STEPS];

  /* misc */
  float DiskScaleRadius;
  float MergTime;
  double Cooling;
  double Heating;
  float r_heat;
  float QuasarModeBHaccretionMass;
  float TimeOfLastMajorMerger;
  float TimeOfLastMinorMerger;
  float OutflowRate;
  float TotalSatelliteBaryons;

  /* infall properties */
  float infallMvir;
  float infallVvir;
  float infallVmax;
};

/* auxiliary halo data */
struct halo_aux_data   
{
  int DoneFlag;
  int HaloFlag;
  int NGalaxies;
  int FirstGalaxy;
};

#endif /* #ifndef TYPES_H */
