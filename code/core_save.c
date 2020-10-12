#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>

#include "core_allvars.h"
#include "core_proto.h"

#define TREE_MUL_FAC        (1000000000LL)
#define FILENR_MUL_FAC      (1000000000000000LL)

// keep a static file handle to remove the need to do constant seeking.
FILE* save_fd[ABSOLUTEMAXSNAPS] = { 0 };

#ifndef MAX_BUF_SIZE
#define MAX_BUF_SIZE (3*MAX_STRING_LEN+40)
#endif
#define MAX_OUTFILE_SIZE (MAX_STRING_LEN+40)

void save_galaxies(int filenr, int tree)
{
  char buf[MAX_BUF_SIZE+1];
  int i, n;
  static const struct GALAXY_OUTPUT galaxy_output_null = {0};
  struct GALAXY_OUTPUT galaxy_output = galaxy_output_null;

  int OutputGalCount[MAXSNAPS], *OutputGalOrder, nwritten;

  OutputGalOrder = (int*)malloc( NumGals*sizeof(int) );
  if(OutputGalOrder == NULL) {
    fprintf(stderr,"Error: Could not allocate memory for %d int elements in array `OutputGalOrder`\n", NumGals);
    ABORT(10);
  }

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
        snprintf(buf, MAX_BUF_SIZE, "%s/%s_z%1.3f_%d", OutputDir, FileNameGalaxies, ZZ[ListOutputSnaps[n]], filenr);


        save_fd[n] = fopen(buf, "r+");
        if (save_fd[n] == NULL)
        {
          fprintf(stderr, "can't open file `%s'\n", buf);
          ABORT(0);
        }

        // write out placeholders for the header data.
        size_t size = (Ntrees + 2)*sizeof(int); /* Extra two inegers are for saving the total number of trees and total number of galaxies in this file */
        int* tmp_buf = (int*)malloc( size );
        if (tmp_buf == NULL)
        {
          fprintf(stderr, "Error: Could not allocate memory for header information for file %d\n", n);
          ABORT(10);
        }

        memset( tmp_buf, 0, size );
        nwritten = fwrite( tmp_buf, sizeof(int), Ntrees + 2, save_fd[n] );
        if (nwritten != Ntrees + 2)
        {
          fprintf(stderr, "Error: Failed to write out %d elements for header information for file %d.  Only wrote %d elements.\n", Ntrees + 2, n, nwritten);
        }
        free( tmp_buf );
            }

      for(i = 0; i < NumGals; i++)
            {
        if(HaloGal[i].SnapNum == ListOutputSnaps[n])
              {
          prepare_galaxy_for_output(filenr, tree, &HaloGal[i], &galaxy_output);

          nwritten = myfwrite(&galaxy_output, sizeof(struct GALAXY_OUTPUT), 1, save_fd[n]);
          if (nwritten != 1)
          {
            fprintf(stderr, "Error: Failed to write out the galaxy struct for galaxy %d within file %d.  Meant to write 1 element but only wrote %d elements.\n", i, n, nwritten);
          }

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

  // assume that because there are so many files, the trees per file will be less than 100000
  // required for limits of long long
  if(LastFile>=10000)
  {
      assert( g->GalaxyNr < TREE_MUL_FAC ); // breaking tree size assumption
      assert(tree < (FILENR_MUL_FAC/10)/TREE_MUL_FAC);
      o->GalaxyIndex = g->GalaxyNr + TREE_MUL_FAC * tree + (FILENR_MUL_FAC/10) * filenr;
      assert( (o->GalaxyIndex - g->GalaxyNr - TREE_MUL_FAC*tree)/(FILENR_MUL_FAC/10) == filenr );
      assert( (o->GalaxyIndex - g->GalaxyNr -(FILENR_MUL_FAC/10)*filenr) / TREE_MUL_FAC == tree );
      assert( o->GalaxyIndex - TREE_MUL_FAC*tree - (FILENR_MUL_FAC/10)*filenr == g->GalaxyNr );
      o->CentralGalaxyIndex = HaloGal[HaloAux[Halo[g->HaloNr].FirstHaloInFOFgroup].FirstGalaxy].GalaxyNr + TREE_MUL_FAC * tree + (FILENR_MUL_FAC/10) * filenr;
  }
  else
  {
      assert( g->GalaxyNr < TREE_MUL_FAC ); // breaking tree size assumption
      assert(tree < FILENR_MUL_FAC/TREE_MUL_FAC);
      o->GalaxyIndex = g->GalaxyNr + TREE_MUL_FAC * tree + FILENR_MUL_FAC * filenr;
      assert( (o->GalaxyIndex - g->GalaxyNr - TREE_MUL_FAC*tree)/FILENR_MUL_FAC == filenr );
      assert( (o->GalaxyIndex - g->GalaxyNr -FILENR_MUL_FAC*filenr) / TREE_MUL_FAC == tree );
      assert( o->GalaxyIndex - TREE_MUL_FAC*tree - FILENR_MUL_FAC*filenr == g->GalaxyNr );
      o->CentralGalaxyIndex = HaloGal[HaloAux[Halo[g->HaloNr].FirstHaloInFOFgroup].FirstGalaxy].GalaxyNr + TREE_MUL_FAC * tree + FILENR_MUL_FAC * filenr;
  }

  o->SAGEHaloIndex = g->HaloNr;
  o->SAGETreeIndex = tree;
  o->SimulationHaloIndex = Halo[g->HaloNr].MostBoundID;

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

  o->TimeOfLastMajorMerger = g->TimeOfLastMajorMerger * UnitTime_in_Megayears;
  o->TimeOfLastMinorMerger = g->TimeOfLastMinorMerger * UnitTime_in_Megayears;

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
  int n, nwritten;

  for(n = 0; n < NOUT; n++)
  {
    // file must already be open.
    assert( save_fd[n] );

    // seek to the beginning.
    fseek( save_fd[n], 0, SEEK_SET );

    nwritten = myfwrite(&Ntrees, sizeof(int), 1, save_fd[n]);
    if (nwritten != 1)
    {
      fprintf(stderr, "Error: Failed to write out 1 element for the number of trees for the header of file %d.  Only wrote %d elements.\n", n, nwritten);
    }

    nwritten = myfwrite(&TotGalaxies[n], sizeof(int), 1, save_fd[n]);
    if (nwritten != 1)
    {
      fprintf(stderr, "Error: Failed to write out 1 element for the number of galaxies for the header of file %d.  Only wrote %d elements.\n", n, nwritten);
    }

    nwritten = myfwrite(TreeNgals[n], sizeof(int), Ntrees, save_fd[n]);
    if (nwritten != Ntrees)
    {
      fprintf(stderr, "Error: Failed to write out %d elements for the number of galaxies per tree for the header of file %d.  Only wrote %d elements.\n", Ntrees, n, nwritten);
    }


    // close the file and clear handle after everything has been written
    fclose( save_fd[n] );
    save_fd[n] = NULL;
  }

}

#undef TREE_MUL_FAC
#undef FILENR_MUL_FAC
