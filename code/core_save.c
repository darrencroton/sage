#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "core_allvars.h"
#include "core_proto.h"



void save_galaxies(int filenr, int tree)
{
#ifndef MINIMIZE_IO
  char buf[1000];
#endif
  FILE *fd;
  int i, n;
  struct GALAXY_OUTPUT galaxy_output;

  for(n = 0; n < NOUT; n++)
  {
#ifdef MINIMIZE_IO
    fd = (FILE *) (size_t)(10 + n);
    offset_galsnapdata[n] = 0;
#else
    sprintf(buf, "%s/%s_z%1.3f_%d", OutputDir, FileNameGalaxies, ZZ[ListOutputSnaps[n]], filenr);

    if(!(fd = fopen(buf, "r+")))
    {
      printf("can't open file `%s'\n", buf);
      ABORT(1);
    }
#endif

    myfseek(fd, (2 + Ntrees) * sizeof(int), SEEK_CUR);
    myfseek(fd, TotGalaxies[n] * sizeof(struct GALAXY_OUTPUT), SEEK_CUR);

    for(i = 0; i < NumGals; i++)
    {
      if(HaloGal[i].SnapNum == ListOutputSnaps[n])
      {
        prepare_galaxy_for_output(n, filenr, tree, &HaloGal[i], &galaxy_output);
        myfwrite(&galaxy_output, sizeof(struct GALAXY_OUTPUT), 1, fd);

        TotGalaxies[n]++;
        TreeNgals[n][tree]++;
      }
    }

#ifndef MINIMIZE_IO
    fclose(fd);
#endif

  }

}



void prepare_galaxy_for_output(int n, int filenr, int tree, struct GALAXY *g, struct GALAXY_OUTPUT *o)
{
  int j;

  o->Type = g->Type;
  o->GalaxyIndex = g->GalaxyNr + 1e6 * tree + 1e12 * filenr;
  o->HaloIndex = g->HaloNr;
  o->FOFHaloIndex = Halo[g->HaloNr].FirstHaloInFOFgroup;
  o->TreeIndex = tree;
  o->SnapNum = g->SnapNum;

  o->CentralGal = g->CentralGal;
  o->CentralMvir = get_virial_mass(Halo[g->HaloNr].FirstHaloInFOFgroup);

  for(j = 0; j < 3; j++)
  {
    o->Pos[j] = g->Pos[j];
    o->Vel[j] = g->Vel[j];
    o->Spin[j] = Halo[g->HaloNr].Spin[j];
  }

  o->Len = g->Len;
  o->Mvir = g->Mvir;
  o->Rvir = get_virial_radius(g->HaloNr);  //output the actual Rvir, not the maximum Rvir
  o->Vvir = get_virial_velocity(g->HaloNr);  //output the actual Vvir, not the maximum Vvir
  o->Vmax = g->Vmax;
  o->VelDisp = Halo[g->HaloNr].VelDisp;

  o->ColdGas = g->ColdGas;
  o->StellarMass = g->StellarMass;
  o->BulgeMass = g->BulgeMass;
  o->HotGas = g->HotGas;
  o->EjectedMass = g->EjectedMass;
  o->BlackHoleMass = g->BlackHoleMass;
  o->ICS = g->ICS;

  o->MetalsColdGas = g->MetalsColdGas;
  o->MetalsStellarMass = g->MetalsStellarMass;
  o->MetalsBulgeMass = g->MetalsBulgeMass;
  o->MetalsHotGas = g->MetalsHotGas;
  o->MetalsEjectedMass = g->MetalsEjectedMass;
  o->MetalsICS = g->MetalsICS;

  // NOTE: in Msun/yr 
  o->Sfr = g->Sfr[n] * UnitMass_in_g / UnitTime_in_s * SEC_PER_YEAR / SOLAR_MASS;
  o->SfrBulge = g->SfrBulge[n] * UnitMass_in_g / UnitTime_in_s * SEC_PER_YEAR / SOLAR_MASS;
  o->SfrICS = g->SfrICS[n] * UnitMass_in_g / UnitTime_in_s * SEC_PER_YEAR / SOLAR_MASS;

  o->DiskScaleRadius = g->DiskScaleRadius;

  if (g->Cooling > 0.0)
    o->Cooling = log10(g->Cooling * UnitEnergy_in_cgs / UnitTime_in_s);
  else
    o->Cooling = 0.0;
  if (g->Heating > 0.0)
    o->Heating = log10(g->Heating * UnitEnergy_in_cgs / UnitTime_in_s);
  else
    o->Heating = 0.0;

  o->LastMajorMerger = g->LastMajorMerger * UnitTime_in_Megayears;
  o->OutflowRate = g->OutflowRate * UnitMass_in_g / UnitTime_in_s * SEC_PER_YEAR / SOLAR_MASS;

}



void finalize_galaxy_file(int filenr)
{
#ifndef MINIMIZE_IO
  char buf[1000];
#endif
  FILE *fd;
  int n;

  for(n = 0; n < NOUT; n++)
  {
#ifdef MINIMIZE_IO
    fd = (FILE *) (size_t)(10 + n);
    offset_galsnapdata[n] = 0;
#else
    sprintf(buf, "%s/%s_z%1.3f_%d", OutputDir, FileNameGalaxies, ZZ[ListOutputSnaps[n]], filenr);
    if(!(fd = fopen(buf, "r+")))
    {
      printf("can't open file `%s'\n", buf);
      ABORT(1);
    }
#endif

    myfwrite(&Ntrees, sizeof(int), 1, fd);
    myfwrite(&TotGalaxies[n], sizeof(int), 1, fd);
    myfwrite(TreeNgals[n], sizeof(int), Ntrees, fd);

#ifndef MINIMIZE_IO
    fclose(fd);
#else
    write_galaxy_data_snap(n, filenr);
#endif

  }
  
}


