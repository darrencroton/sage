#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>

#include "core_allvars.h"
#include "core_proto.h"


// keep a static file handle to remove the need to do constant seeking.
FILE* save_fd[ABSOLUTEMAXSNAPS] = { 0 };


void save_galaxies(int filenr, int tree)
{
  char buf[1000];
  int i, n;
  struct GALAXY_OUTPUT galaxy_output;
  int OutputGalCount[MAXSNAPS], *OutputGalOrder;

  OutputGalOrder = (int*)malloc( NumGals*sizeof(int) );
  assert( OutputGalOrder );

  // reset the output galaxy count and order
  for(i = 0; i < MAXSNAPS; i++)
    OutputGalCount[i] = 0;
  for(i = 0; i < NumGals; i++)
    OutputGalOrder[i] = -1;

  // first update mergeIntoID to point to the correct galaxy in the output
  for(n = 0; n < NOUT; n++)
  {
    for(i = 0; i < NumGals; i++)
    {
      if(HaloGal[i].SnapNum == ListOutputSnaps[n])
      {
        OutputGalOrder[i] = OutputGalCount[n];
        OutputGalCount[n]++;
      }
    }
  }
    
  for(i = 0; i < NumGals; i++)
    if(HaloGal[i].mergeIntoID > -1)
      HaloGal[i].mergeIntoID = OutputGalOrder[HaloGal[i].mergeIntoID];    
  
  // now prepare and write galaxies
  for(n = 0; n < NOUT; n++)
  {
    // only open the file if it is not already open.
    if( !save_fd[n] )
    {
      sprintf(buf, "%s/%s_z%1.3f_%d", OutputDir, FileNameGalaxies, ZZ[ListOutputSnaps[n]], filenr);

      if(!(save_fd[n] = fopen(buf, "r+")))
      {
				printf("can't open file `%s'\n", buf);
				ABORT(0);
      }

			// write out placeholders for the header data.
			size_t size = (Ntrees + 2)*sizeof(int);
			int* tmp_buf = (int*)malloc( size );
			memset( tmp_buf, 0, size );
			fwrite( tmp_buf, sizeof(int), Ntrees + 2, save_fd[n] );
			free( tmp_buf );
		}

    for(i = 0; i < NumGals; i++)
    {
      if(HaloGal[i].SnapNum == ListOutputSnaps[n])
      {        
        prepare_galaxy_for_output(filenr, tree, &HaloGal[i], &galaxy_output);
        myfwrite(&galaxy_output, sizeof(struct GALAXY_OUTPUT), 1, save_fd[n]);

        TotGalaxies[n]++;
        TreeNgals[n][tree]++;
      }
    }

  }

  // don't forget to free the workspace.
  free( OutputGalOrder );

}



void prepare_galaxy_for_output(int filenr, int tree, struct GALAXY *g, struct GALAXY_OUTPUT *o)
{
  int j, step;

  o->SnapNum = g->SnapNum;
  o->Type = g->Type;

  assert( g->GalaxyNr < 1e9 ); // breaking tree size assumption
  o->GalaxyIndex = g->GalaxyNr + 1e9 * tree + 1e12 * filenr;
  assert( (o->GalaxyIndex - g->GalaxyNr - 1e9*tree)/1e12 == filenr );
  assert( (o->GalaxyIndex - g->GalaxyNr - 1e12*filenr)/1e9 == tree );
  assert( o->GalaxyIndex - 1e9*tree - 1e12*filenr == g->GalaxyNr );

  o->CentralGalaxyIndex = HaloGal[HaloAux[Halo[g->HaloNr].FirstHaloInFOFgroup].FirstGalaxy].GalaxyNr + 1e9 * tree + 1e12 * filenr;

  o->SAGEHaloIndex = g->HaloNr;
  o->SAGETreeIndex = tree;
  o->SimulationFOFHaloIndex = Halo[g->HaloNr].SubhaloIndex;

  o->mergeType = g->mergeType;
  o->mergeIntoID = g->mergeIntoID;
  o->mergeIntoSnapNum = g->mergeIntoSnapNum;
  o->dT = g->dT * UnitTime_in_s / SEC_PER_MEGAYEAR;

  for(j = 0; j < 3; j++)
  {
    o->Pos[j] = g->Pos[j];
    o->Vel[j] = g->Vel[j];
    o->Spin[j] = Halo[g->HaloNr].Spin[j];
  }

  o->Len = g->Len;
  o->Mvir = g->Mvir;
  o->CentralMvir = get_virial_mass(Halo[g->HaloNr].FirstHaloInFOFgroup);
  o->Rvir = get_virial_radius(g->HaloNr);  // output the actual Rvir, not the maximum Rvir
  o->Vvir = get_virial_velocity(g->HaloNr);  // output the actual Vvir, not the maximum Vvir
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
  
  o->SfrDisk = 0.0;
  o->SfrBulge = 0.0;
  o->SfrDiskZ = 0.0;
  o->SfrBulgeZ = 0.0;
  
  // NOTE: in Msun/yr 
  for(step = 0; step < STEPS; step++)
  {
    o->SfrDisk += g->SfrDisk[step] * UnitMass_in_g / UnitTime_in_s * SEC_PER_YEAR / SOLAR_MASS / STEPS;
    o->SfrBulge += g->SfrBulge[step] * UnitMass_in_g / UnitTime_in_s * SEC_PER_YEAR / SOLAR_MASS / STEPS;
    
    if(g->SfrDiskColdGas[step] > 0.0)
      o->SfrDiskZ += g->SfrDiskColdGasMetals[step] / g->SfrDiskColdGas[step] / STEPS;

    if(g->SfrBulgeColdGas[step] > 0.0)
      o->SfrBulgeZ += g->SfrBulgeColdGasMetals[step] / g->SfrBulgeColdGas[step] / STEPS;
  }

  o->DiskScaleRadius = g->DiskScaleRadius;

  if (g->Cooling > 0.0)
    o->Cooling = log10(g->Cooling * UnitEnergy_in_cgs / UnitTime_in_s);
  else
    o->Cooling = 0.0;
  if (g->Heating > 0.0)
    o->Heating = log10(g->Heating * UnitEnergy_in_cgs / UnitTime_in_s);
  else
    o->Heating = 0.0;

  o->QuasarModeBHaccretionMass = g->QuasarModeBHaccretionMass;

  o->TimeSinceMajorMerger = g->TimeSinceMajorMerger * UnitTime_in_Megayears;
  o->TimeSinceMinorMerger = g->TimeSinceMinorMerger * UnitTime_in_Megayears;
	
  o->OutflowRate = g->OutflowRate * UnitMass_in_g / UnitTime_in_s * SEC_PER_YEAR / SOLAR_MASS;

  //infall properties
  if(g->Type != 0)
  {
    o->infallMvir = g->infallMvir;
    o->infallVvir = g->infallVvir;
    o->infallVmax = g->infallVmax;
  }
  else
  {
    o->infallMvir = 0.0;
    o->infallVvir = 0.0;
    o->infallVmax = 0.0;
  }

}



void finalize_galaxy_file(int filenr)
{
  int n;

  for(n = 0; n < NOUT; n++)
  {
    // file must already be open.
    assert( save_fd[n] );

    // seek to the beginning.
    fseek( save_fd[n], 0, SEEK_SET );

    myfwrite(&Ntrees, sizeof(int), 1, save_fd[n]);
    myfwrite(&TotGalaxies[n], sizeof(int), 1, save_fd[n]);
    myfwrite(TreeNgals[n], sizeof(int), Ntrees, save_fd[n]);

    // close the file and clear handle after everything has been written
    fclose( save_fd[n] );
    save_fd[n] = NULL;
  }
  
}


