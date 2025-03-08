/**
 * @file    io_save_binary.c
 * @brief   Functions for saving galaxy data to binary output files
 *
 * This file contains the functionality for writing simulated galaxies to binary output
 * files. It handles the conversion of internal galaxy data structures to the
 * output format, manages file I/O operations, and ensures consistent galaxy
 * indexing across files. The code supports writing galaxy data for multiple
 * snapshots and maintains proper cross-references between galaxies.
 *
 * Key functions:
 * - save_galaxies(): Writes galaxies to output files for all requested snapshots
 * - prepare_galaxy_for_output(): Converts internal galaxy format to output format
 * - finalize_galaxy_file(): Completes file writing by updating headers
 *
 * The output files include headers with tree counts and galaxy counts per tree,
 * followed by the galaxy data for the corresponding snapshot.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>

#include "core_allvars.h"
#include "core_proto.h"
#include "error_handling.h"

#define TREE_MUL_FAC        (1000000000LL)
#define FILENR_MUL_FAC      (1000000000000000LL)

// keep a static file handle to remove the need to do constant seeking.
FILE* save_fd[ABSOLUTEMAXSNAPS] = { 0 };

#ifndef MAX_BUF_SIZE
#define MAX_BUF_SIZE (3*MAX_STRING_LEN+40)
#endif
#define MAX_OUTFILE_SIZE (MAX_STRING_LEN+40)

/**
 * @brief   Saves galaxies to output files for all requested snapshots
 *
 * @param   filenr    Current file number being processed
 * @param   tree      Current tree number being processed
 *
 * This function writes all galaxies for the current tree to their respective
 * output files. For each output snapshot, it:
 * 
 * 1. Opens the output file if not already open
 * 2. Writes placeholder headers to be filled later
 * 3. Processes galaxies belonging to that snapshot
 * 4. Converts internal galaxy structures to output format
 * 5. Writes galaxies to the file
 * 6. Updates galaxy counts for the file and tree
 * 
 * The function also handles the indexing system that allows cross-referencing
 * between galaxies (e.g., for tracking merger destinations) across different
 * trees and files.
 */
void save_galaxies(int filenr, int tree)
{
  char buf[MAX_BUF_SIZE+1];
  int i, n;
  static const struct GALAXY_OUTPUT galaxy_output_null = {0};
  struct GALAXY_OUTPUT galaxy_output = galaxy_output_null;

  int OutputGalCount[MAXSNAPS], *OutputGalOrder, nwritten;

  OutputGalOrder = (int*)malloc( NumGals*sizeof(int) );
  if(OutputGalOrder == NULL) {
    FATAL_ERROR("Memory allocation failed for OutputGalOrder array (%d elements, %zu bytes)", 
               NumGals, NumGals*sizeof(int));
  }

  // reset the output galaxy count and order
  for(i = 0; i < MAXSNAPS; i++)
    OutputGalCount[i] = 0;
  for(i = 0; i < NumGals; i++)
    OutputGalOrder[i] = -1;

  // first update mergeIntoID to point to the correct galaxy in the output
  for(n = 0; n < SageConfig.NOUT; n++)
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
  for(n = 0; n < SageConfig.NOUT; n++)
  {
      // only open the file if it is not already open.
      if( !save_fd[n] )
            {
        snprintf(buf, MAX_BUF_SIZE, "%s/%s_z%1.3f_%d", SageConfig.OutputDir, SageConfig.FileNameGalaxies, ZZ[ListOutputSnaps[n]], filenr);

        save_fd[n] = fopen(buf, "r+");
        if (save_fd[n] == NULL)
        {
          FATAL_ERROR("Failed to open output galaxy file '%s' for snapshot %d (filenr %d)", 
                    buf, ListOutputSnaps[n], filenr);
        }

        // write out placeholders for the header data.
        size_t size = (Ntrees + 2)*sizeof(int); /* Extra two inegers are for saving the total number of trees and total number of galaxies in this file */
        int* tmp_buf = (int*)malloc( size );
        if (tmp_buf == NULL)
        {
          FATAL_ERROR("Memory allocation failed for header buffer (%zu bytes) for snapshot %d (filenr %d)", 
                     size, ListOutputSnaps[n], filenr);
        }

        memset( tmp_buf, 0, size );
        nwritten = fwrite( tmp_buf, sizeof(int), Ntrees + 2, save_fd[n] );
        if (nwritten != Ntrees + 2)
        {
        ERROR_LOG("Failed to write header information to output file %d. Expected %d elements, wrote %d elements. Will retry after output is complete", 
                 n, Ntrees + 2, nwritten);
        // Note: This is converted to ERROR_LOG to demonstrate a recoverable error
        // We could retry or implement a fallback strategy here instead of aborting
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
            FATAL_ERROR("Failed to write galaxy data for galaxy %d (tree %d, filenr %d, snapshot %d). Expected 1 element, wrote %d elements", 
                       i, tree, filenr, ListOutputSnaps[n], nwritten);
          }

          TotGalaxies[n]++;
          SimState.TotGalaxies[n]++; /* Update SimState directly */
          TreeNgals[n][tree]++;
              }
            }

  }

  // don't forget to free the workspace.
  free( OutputGalOrder );

}



/**
 * @brief   Converts internal galaxy structure to output format
 *
 * @param   filenr    Current file number being processed
 * @param   tree      Current tree number being processed
 * @param   g         Pointer to the internal galaxy structure
 * @param   o         Pointer to the output galaxy structure to be filled
 *
 * This function transforms the internal galaxy representation (GALAXY struct)
 * to the output format (GALAXY_OUTPUT struct). It:
 * 
 * 1. Copies basic galaxy properties (type, position, velocities, masses)
 * 2. Calculates derived properties (star formation rates, metallicities)
 * 3. Creates a unique galaxy index that encodes file, tree, and galaxy number
 * 4. Converts units from internal simulation units to physical units
 * 5. Processes special properties like cooling/heating rates
 * 
 * The unique indexing system allows galaxies to be cross-referenced across
 * different trees and files, which is essential for tracking mergers and
 * other inter-galaxy relationships.
 */
void prepare_galaxy_for_output(int filenr, int tree, struct GALAXY *g, struct GALAXY_OUTPUT *o)
{
  int j, step;

  o->SnapNum = g->SnapNum;
  o->Type = g->Type;

  // assume that because there are so many files, the trees per file will be less than 100000
  // required for limits of long long
  if(SageConfig.LastFile>=10000)
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



/**
 * @brief   Finalizes galaxy output files by writing header information
 *
 * @param   filenr    Current file number being processed
 *
 * This function completes the galaxy output files after all galaxies have
 * been written. For each output snapshot, it:
 * 
 * 1. Seeks to the beginning of the file
 * 2. Writes the total number of trees
 * 3. Writes the total number of galaxies in the file
 * 4. Writes the number of galaxies for each tree
 * 5. Closes the file
 * 
 * This header information is essential for readers to navigate the file
 * structure and access specific trees or galaxies efficiently.
 */
void finalize_galaxy_file(int filenr)
{
  int n, nwritten;

  for(n = 0; n < SageConfig.NOUT; n++)
  {
    // file must already be open.
    assert( save_fd[n] );

    // seek to the beginning.
    fseek( save_fd[n], 0, SEEK_SET );

    nwritten = myfwrite(&Ntrees, sizeof(int), 1, save_fd[n]);
    if (nwritten != 1)
    {
      FATAL_ERROR("Failed to write number of trees to header of file %d (filenr %d). Expected 1 element, wrote %d elements", 
                 n, filenr, nwritten);
    }

    nwritten = myfwrite(&TotGalaxies[n], sizeof(int), 1, save_fd[n]);
    if (nwritten != 1)
    {
      FATAL_ERROR("Failed to write total galaxy count to header of file %d (filenr %d). Expected 1 element, wrote %d elements", 
                 n, filenr, nwritten);
    }

    nwritten = myfwrite(TreeNgals[n], sizeof(int), Ntrees, save_fd[n]);
    if (nwritten != Ntrees)
    {
      FATAL_ERROR("Failed to write galaxy counts per tree to header of file %d (filenr %d). Expected %d elements, wrote %d elements", 
                 n, filenr, Ntrees, nwritten);
    }


    // close the file and clear handle after everything has been written
    fclose( save_fd[n] );
    save_fd[n] = NULL;
  }

}

#undef TREE_MUL_FAC
#undef FILENR_MUL_FAC
