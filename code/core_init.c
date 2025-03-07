#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_integration.h>

#include "core_allvars.h"
#include "core_proto.h"



void init(void)
{
  int i;

  Age = mymalloc(ABSOLUTEMAXSNAPS*sizeof(*Age));

  random_generator = gsl_rng_alloc(gsl_rng_ranlxd1);
  gsl_rng_set(random_generator, 42);     // start-up seed

  set_units();
  srand((unsigned) time(NULL));

  read_snap_list();

  //Hack to fix deltaT for snapshot 0
  //This way, galsnapnum = -1 will not segfault.
  Age[0] = time_to_present(1000.0);//lookback time from z=1000
  Age++;

  for(i = 0; i < SageConfig.Snaplistlen; i++)
  {
    SageConfig.ZZ[i] = 1 / SageConfig.AA[i] - 1;
    Age[i] = time_to_present(SageConfig.ZZ[i]);
    ZZ[i] = SageConfig.ZZ[i]; // Sync with global for backward compatibility
  }

  SageConfig.a0 = 1.0 / (1.0 + SageConfig.Reionization_z0);
  SageConfig.ar = 1.0 / (1.0 + SageConfig.Reionization_zr);
  
  // Sync with globals for backward compatibility
  a0 = SageConfig.a0;
  ar = SageConfig.ar;

  read_cooling_functions();
}



void set_units(void)
{
  // Calculate derived units and store in SageConfig
  SageConfig.UnitTime_in_s = SageConfig.UnitLength_in_cm / SageConfig.UnitVelocity_in_cm_per_s;
  SageConfig.UnitTime_in_Megayears = SageConfig.UnitTime_in_s / SEC_PER_MEGAYEAR;
  SageConfig.G = GRAVITY / pow(SageConfig.UnitLength_in_cm, 3) * SageConfig.UnitMass_in_g * pow(SageConfig.UnitTime_in_s, 2);
  SageConfig.UnitDensity_in_cgs = SageConfig.UnitMass_in_g / pow(SageConfig.UnitLength_in_cm, 3);
  SageConfig.UnitPressure_in_cgs = SageConfig.UnitMass_in_g / SageConfig.UnitLength_in_cm / pow(SageConfig.UnitTime_in_s, 2);
  SageConfig.UnitCoolingRate_in_cgs = SageConfig.UnitPressure_in_cgs / SageConfig.UnitTime_in_s;
  SageConfig.UnitEnergy_in_cgs = SageConfig.UnitMass_in_g * pow(SageConfig.UnitLength_in_cm, 2) / pow(SageConfig.UnitTime_in_s, 2);

  SageConfig.EnergySNcode = SageConfig.EnergySN / SageConfig.UnitEnergy_in_cgs * SageConfig.Hubble_h;
  SageConfig.EtaSNcode = SageConfig.EtaSN * (SageConfig.UnitMass_in_g / SOLAR_MASS) / SageConfig.Hubble_h;

  // Convert some physical input parameters to internal units
  SageConfig.Hubble = HUBBLE * SageConfig.UnitTime_in_s;

  // Compute a few quantities
  SageConfig.RhoCrit = 3 * SageConfig.Hubble * SageConfig.Hubble / (8 * M_PI * SageConfig.G);
  
  // Synchronize with global variables (for backward compatibility)
  UnitLength_in_cm = SageConfig.UnitLength_in_cm;
  UnitMass_in_g = SageConfig.UnitMass_in_g;
  UnitVelocity_in_cm_per_s = SageConfig.UnitVelocity_in_cm_per_s;
  UnitTime_in_s = SageConfig.UnitTime_in_s;
  UnitTime_in_Megayears = SageConfig.UnitTime_in_Megayears;
  G = SageConfig.G;
  UnitDensity_in_cgs = SageConfig.UnitDensity_in_cgs;
  UnitPressure_in_cgs = SageConfig.UnitPressure_in_cgs;
  UnitCoolingRate_in_cgs = SageConfig.UnitCoolingRate_in_cgs;
  UnitEnergy_in_cgs = SageConfig.UnitEnergy_in_cgs;
  EnergySNcode = SageConfig.EnergySNcode;
  EtaSNcode = SageConfig.EtaSNcode;
  Hubble = SageConfig.Hubble;
  RhoCrit = SageConfig.RhoCrit;
}



void read_snap_list(void)
{
  FILE *fd;
  char fname[MAX_STRING_LEN+1];

  snprintf(fname, MAX_STRING_LEN, "%s", SageConfig.FileWithSnapList);

  if(!(fd = fopen(fname, "r")))
  {
    FATAL_ERROR("Can't read output list in file '%s'", fname);
  }

  SageConfig.Snaplistlen = 0;
  do {
    if(fscanf(fd, " %lg ", &SageConfig.AA[SageConfig.Snaplistlen]) == 1)
      SageConfig.Snaplistlen++;
    else
      break;
  } while(SageConfig.Snaplistlen < SageConfig.MAXSNAPS);

  fclose(fd);

  // Synchronize with globals for backward compatibility
  Snaplistlen = SageConfig.Snaplistlen;
  memcpy(AA, SageConfig.AA, sizeof(double) * ABSOLUTEMAXSNAPS);

#ifdef MPI
  if(ThisTask == 0)
#endif
    INFO_LOG("Found %d defined times in snaplist", SageConfig.Snaplistlen);
}



double time_to_present(double z)
{
#define WORKSIZE 1000
  gsl_function F;
  gsl_integration_workspace *workspace;
  double time, result, abserr;

  workspace = gsl_integration_workspace_alloc(WORKSIZE);
  F.function = &integrand_time_to_present;

  gsl_integration_qag(&F, 1.0 / (z + 1), 1.0, 1.0 / SageConfig.Hubble,
    1.0e-8, WORKSIZE, GSL_INTEG_GAUSS21, workspace, &result, &abserr);

  time = 1 / SageConfig.Hubble * result;

  gsl_integration_workspace_free(workspace);

  // return time to present as a function of redshift
  return time;
}



double integrand_time_to_present(double a, void *param)
{
  return 1 / sqrt(SageConfig.Omega / a + (1 - SageConfig.Omega - SageConfig.OmegaLambda) + SageConfig.OmegaLambda * a * a);
}
