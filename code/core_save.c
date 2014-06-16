#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>

#include "core_allvars.h"
#include "core_proto.h"


/* Keep a static file handle to remove the need to
   do constant seeking. */
FILE* save_fd[NOUT] = { 0 };

/* Need access to the mergers file. */
extern FILE* mergers_fd;


void save_galaxies(int filenr, int tree)
{
#ifndef MINIMIZE_IO
  char buf[1000];
#endif
  /* FILE *fd; */
  int i, n;
  struct GALAXY_OUTPUT galaxy_output;
  int OutputGalCount[MAXSNAPS], OutputGalOrder[NumGals];
  // int counter;

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
#ifdef MINIMIZE_IO
    fd = (FILE *) (size_t)(10 + n);
    offset_galsnapdata[n] = 0;
#else

    /* Only open the file if it is not already open. */
    if( !save_fd[n] )
    {
      sprintf(buf, "%s/%s_z%1.3f_%d", OutputDir, FileNameGalaxies, ZZ[ListOutputSnaps[n]], filenr);

      if(!(save_fd[n] = fopen(buf, "r+")))
      {
	printf("can't open file `%s'\n", buf);
	ABORT(1);
      }

      /* Write out placeholders for the header data. */
      {
	size_t size = (Ntrees + 2)*sizeof(int);
	int* tmp_buf = (int*)malloc( size );
	memset( tmp_buf, 0, size );
	fwrite( tmp_buf, sizeof(int), Ntrees + 2, save_fd[n] );
	free( tmp_buf );
      }
    }

#endif

    /* No longer need to seek. */
    /* myfseek(fd, (2 + Ntrees) * sizeof(int), SEEK_CUR); */
    /* myfseek(fd, TotGalaxies[n] * sizeof(struct GALAXY_OUTPUT), SEEK_CUR); */

    // counter = 0;
    for(i = 0; i < NumGals; i++)
    {
      if(HaloGal[i].SnapNum == ListOutputSnaps[n])
      {
        
        // // if(tree == 707 && HaloGal[i].SnapNum >= 58 && HaloGal[i].SnapNum <= 60)
        // if(tree == 27 && HaloGal[i].SnapNum >= 54 && HaloGal[i].SnapNum <= 55)
        // {
        //   printf("SAVE:\t%i\t%i\t%i\t%i\t%f\t%i\t%f\t%i\t%i\t%i\t%i\n", 
        //     counter, i, HaloGal[i].GalaxyNr, HaloGal[i].SnapNum, 
        //     HaloGal[i].Mvir, HaloGal[i].Len, HaloGal[i].StellarMass, 
        //     HaloGal[i].Type, HaloGal[i].mergeType, HaloGal[i].mergeIntoID, HaloGal[i].mergeIntoSnapNum);
        // }
        // counter++;
        
        prepare_galaxy_for_output(filenr, tree, &HaloGal[i], &galaxy_output);
        myfwrite(&galaxy_output, sizeof(struct GALAXY_OUTPUT), 1, save_fd[n]);

        TotGalaxies[n]++;
        TreeNgals[n][tree]++;
      }
    }

#ifndef MINIMIZE_IO
    /* Don't close between calls. */
    /* fclose(fd); */
#endif

  }

  /* Write mergers and free memory. */
  {
    /* Count mergers. */
    merger_node_type* cur = merger_nodes;
    unsigned n_mergers = 0;
    while( cur )
    {
      ++n_mergers;
      cur = cur->next;
    }
    fwrite( &n_mergers, sizeof(unsigned), 1, mergers_fd );

    /* Dump mergers. */
    cur = merger_nodes;
    while( cur )
    {
      merger_node_type* next = cur->next;
      fwrite( &cur->central, sizeof(long long), 1, mergers_fd );
      fwrite( &cur->merged, sizeof(long long), 1, mergers_fd );
      fwrite( &cur->snapshot, sizeof(unsigned), 1, mergers_fd );
      free( cur );
      cur = next;
    }
    merger_nodes = NULL;
  }

}



void prepare_galaxy_for_output(int filenr, int tree, struct GALAXY *g, struct GALAXY_OUTPUT *o)
{
  int j, step;

  o->Type = g->Type;
  o->mergeType = g->mergeType;
  o->mergeIntoID = g->mergeIntoID;
  assert( g->GalaxyNr < 1e9 ); // breaking tree size assumption
  o->GalaxyIndex = g->GalaxyNr + 1e9 * tree + 1e12 * filenr;
  assert( (o->GalaxyIndex - g->GalaxyNr - 1e9*tree)/1e12 == filenr );
  assert( (o->GalaxyIndex - g->GalaxyNr - 1e12*filenr)/1e9 == tree );
  assert( o->GalaxyIndex - 1e9*tree - 1e12*filenr == g->GalaxyNr );
  o->HaloIndex = g->HaloNr;
  o->FOFHaloIndex = Halo[g->HaloNr].FirstHaloInFOFgroup;
  o->TreeIndex = tree;
  o->SnapNum = g->SnapNum;
  o->dt = g->dt;

  o->CentralGal = g->CentralGal;
  o->CentralMvir = get_virial_mass(Halo[g->HaloNr].FirstHaloInFOFgroup);

  o->mergeType = g->mergeType;
  o->mergeIntoID = g->mergeIntoID;
  o->mergeIntoSnapNum = g->mergeIntoSnapNum;

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

  o->LastMajorMerger = g->LastMajorMerger * UnitTime_in_Megayears;
  o->OutflowRate = g->OutflowRate * UnitMass_in_g / UnitTime_in_s * SEC_PER_YEAR / SOLAR_MASS;

  o->infallMvir = g->infallMvir;  //infall properties
  o->infallVvir = g->infallVvir;
  o->infallVmax = g->infallVmax;

}



void finalize_galaxy_file(int filenr)
{
#ifndef MINIMIZE_IO
  /* char buf[1000]; */
#endif
  FILE *fd;
  int n;

  for(n = 0; n < NOUT; n++)
  {
#ifdef MINIMIZE_IO
    fd = (FILE *) (size_t)(10 + n);
    offset_galsnapdata[n] = 0;
#else

    /* File must already be open. */
    assert( save_fd[n] );

    /* Seek to the beginning. */
    fseek( save_fd[n], 0, SEEK_SET );

    /* sprintf(buf, "%s/%s_z%1.3f_%d", OutputDir, FileNameGalaxies, ZZ[ListOutputSnaps[n]], filenr); */
    /* if(!(fd = fopen(buf, "r+"))) */
    /* { */
    /*   printf("can't open file `%s'\n", buf); */
    /*   ABORT(1); */
    /* } */
#endif

    myfwrite(&Ntrees, sizeof(int), 1, save_fd[n]);
    myfwrite(&TotGalaxies[n], sizeof(int), 1, save_fd[n]);
    myfwrite(TreeNgals[n], sizeof(int), Ntrees, save_fd[n]);

#ifndef MINIMIZE_IO

    /* Close the file and clear handle after everything has
       been written. */
    fclose( save_fd[n] );
    save_fd[n] = NULL;

#else
    write_galaxy_data_snap(n, filenr);
#endif
  }
  
}


