#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <limits.h>

#include "core_allvars.h"
#include "core_proto.h"

#define TREE_MUL_FAC        (1000000000LL)
#define FILENR_MUL_FAC      (1000000000000000LL)

// keep a static file handle to remove the need to do constant seeking.
FILE* save_fd[ABSOLUTEMAXSNAPS] = { 0 };

void save_galaxies(const int filenr, const int tree, const int ntrees, const int numgals, struct halo_data *halos,
                   struct halo_aux_data *haloaux, struct GALAXY *halogal, int **treengals, int *totgalaxies)
{

    char buf[MAX_STRING_LEN];
    struct GALAXY_OUTPUT galaxy_output;
    memset(&galaxy_output, 0, sizeof(struct GALAXY_OUTPUT));
    int OutputGalCount[MAXSNAPS], *OutputGalOrder, nwritten;

    OutputGalOrder = (int*)malloc( numgals*sizeof(int) );
    if(OutputGalOrder == NULL) {
        fprintf(stderr,"Error: Could not allocate memory for %d int elements in array `OutputGalOrder`\n", numgals);
        ABORT(10);
    }

    // reset the output galaxy count and order
    for(int i = 0; i < MAXSNAPS; i++) {
        OutputGalCount[i] = 0;
    }

    for(int i = 0; i < numgals; i++) {
        OutputGalOrder[i] = -1;
    }
  
    // first update mergeIntoID to point to the correct galaxy in the output
    for(int n = 0; n < NOUT; n++) {
        for(int i = 0; i < numgals; i++) {
            if(halogal[i].SnapNum == ListOutputSnaps[n]) {
                OutputGalOrder[i] = OutputGalCount[n];
                OutputGalCount[n]++;
            }
        }
    }
  
    for(int i = 0; i < numgals; i++) {
        if(halogal[i].mergeIntoID > -1) {
            halogal[i].mergeIntoID = OutputGalOrder[halogal[i].mergeIntoID];
        }
    }
  
    // now prepare and write galaxies
    for(int n = 0; n < NOUT; n++) {
        // only open the file if it is not already open.
        if( !save_fd[n] ) {
            snprintf(buf, MAX_STRING_LEN - 1, "%s/%s_z%1.3f_%d", OutputDir, FileNameGalaxies, ZZ[ListOutputSnaps[n]], filenr);
       
            save_fd[n] = fopen(buf, "r+");
            if (save_fd[n] == NULL) {
                fprintf(stderr, "Error: Can't open file `%s'\n", buf);
                ABORT(0);
            }
        
            // write out placeholders for the header data.
            size_t size = (ntrees + 2)*sizeof(int); /* Extra two inegers are for saving the total number of trees and total number of galaxies in this file */
            int* tmp_buf = (int*)malloc( size );
            if (tmp_buf == NULL) {
                fprintf(stderr, "Error: Could not allocate memory for header information for file %d\n", n);
                ABORT(10);
            }

            memset( tmp_buf, 0, size );
            nwritten = fwrite( tmp_buf, sizeof(int), ntrees + 2, save_fd[n] );
            if (nwritten != ntrees + 2) {
                fprintf(stderr, "Error: Failed to write out %d elements for header information for file %d. "
                        "Only wrote %d elements.\n", ntrees + 2, n, nwritten);
            }
            free( tmp_buf );
        }
      
        for(int i = 0; i < numgals; i++) {
            if(halogal[i].SnapNum == ListOutputSnaps[n]) {
                prepare_galaxy_for_output(filenr, tree, &halogal[i], &galaxy_output, halos, haloaux, halogal);
   
                nwritten = myfwrite(&galaxy_output, sizeof(struct GALAXY_OUTPUT), 1, save_fd[n]);
                if (nwritten != 1) {
                    fprintf(stderr, "Error: Failed to write out the galaxy struct for galaxy %d within file %d. "
                            " Meant to write 1 element but only wrote %d elements.\n", i, n, nwritten); 
                }
          
                totgalaxies[n]++;
                treengals[n][tree]++;	      
            }
        }
    }

    // don't forget to free the workspace.
    free( OutputGalOrder );

}



void prepare_galaxy_for_output(int filenr, int tree, struct GALAXY *g, struct GALAXY_OUTPUT *o,
                               struct halo_data *halos,
                               struct halo_aux_data *haloaux, struct GALAXY *halogal)
{
    o->SnapNum = g->SnapNum;
    assert(g->Type >= SHRT_MIN && g->Type <= SHRT_MAX && "Converting galaxy type while saving from integer to short will result in data corruption");
    o->Type = g->Type;

    // assume that because there are so many files, the trees per file will be less than 100000
    // required for limits of long long
    if(LastFile>=10000) {
        assert( g->GalaxyNr < TREE_MUL_FAC ); // breaking tree size assumption
        assert(tree < (FILENR_MUL_FAC/10)/TREE_MUL_FAC);
        o->GalaxyIndex = g->GalaxyNr + TREE_MUL_FAC * tree + (FILENR_MUL_FAC/10) * filenr;
        assert( (o->GalaxyIndex - g->GalaxyNr - TREE_MUL_FAC*tree)/(FILENR_MUL_FAC/10) == filenr );
        assert( (o->GalaxyIndex - g->GalaxyNr -(FILENR_MUL_FAC/10)*filenr) / TREE_MUL_FAC == tree );
        assert( o->GalaxyIndex - TREE_MUL_FAC*tree - (FILENR_MUL_FAC/10)*filenr == g->GalaxyNr );
        o->CentralGalaxyIndex = halogal[haloaux[halos[g->HaloNr].FirstHaloInFOFgroup].FirstGalaxy].GalaxyNr + TREE_MUL_FAC * tree + (FILENR_MUL_FAC/10) * filenr;
    } else {
        assert( g->GalaxyNr < TREE_MUL_FAC ); // breaking tree size assumption
        assert(tree < FILENR_MUL_FAC/TREE_MUL_FAC);
        o->GalaxyIndex = g->GalaxyNr + TREE_MUL_FAC * tree + FILENR_MUL_FAC * filenr;
        assert( (o->GalaxyIndex - g->GalaxyNr - TREE_MUL_FAC*tree)/FILENR_MUL_FAC == filenr );
        assert( (o->GalaxyIndex - g->GalaxyNr -FILENR_MUL_FAC*filenr) / TREE_MUL_FAC == tree );
        assert( o->GalaxyIndex - TREE_MUL_FAC*tree - FILENR_MUL_FAC*filenr == g->GalaxyNr );
        o->CentralGalaxyIndex = halogal[haloaux[halos[g->HaloNr].FirstHaloInFOFgroup].FirstGalaxy].GalaxyNr + TREE_MUL_FAC * tree + FILENR_MUL_FAC * filenr;
    }

#undef TREE_MUL_FAC
#undef FILENR_MUL_FAC

    o->SAGEHaloIndex = g->HaloNr;
    o->SAGETreeIndex = tree;
    o->SimulationHaloIndex = llabs(halos[g->HaloNr].MostBoundID);
#if 0
    o->isFlyby = halos[g->HaloNr].MostBoundID < 0 ? 1:0;
#endif  

    o->mergeType = g->mergeType;
    o->mergeIntoID = g->mergeIntoID;
    o->mergeIntoSnapNum = g->mergeIntoSnapNum;
    o->dT = g->dT * UnitTime_in_s / SEC_PER_MEGAYEAR;

    for(int j = 0; j < 3; j++) {
        o->Pos[j] = g->Pos[j];
        o->Vel[j] = g->Vel[j];
        o->Spin[j] = halos[g->HaloNr].Spin[j];
    }
    
    o->Len = g->Len;
    o->Mvir = g->Mvir;
    o->CentralMvir = get_virial_mass(halos[g->HaloNr].FirstHaloInFOFgroup, halos);
    o->Rvir = get_virial_radius(g->HaloNr, halos);  // output the actual Rvir, not the maximum Rvir
    o->Vvir = get_virial_velocity(g->HaloNr, halos);  // output the actual Vvir, not the maximum Vvir
    o->Vmax = g->Vmax;
    o->VelDisp = halos[g->HaloNr].VelDisp;

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
    for(int step = 0; step < STEPS; step++) {
        o->SfrDisk += g->SfrDisk[step] * UnitMass_in_g / UnitTime_in_s * SEC_PER_YEAR / SOLAR_MASS / STEPS;
        o->SfrBulge += g->SfrBulge[step] * UnitMass_in_g / UnitTime_in_s * SEC_PER_YEAR / SOLAR_MASS / STEPS;
        
        if(g->SfrDiskColdGas[step] > 0.0) {
            o->SfrDiskZ += g->SfrDiskColdGasMetals[step] / g->SfrDiskColdGas[step] / STEPS;
        }
        
        if(g->SfrBulgeColdGas[step] > 0.0) {
            o->SfrBulgeZ += g->SfrBulgeColdGasMetals[step] / g->SfrBulgeColdGas[step] / STEPS;
        }
    }

    o->DiskScaleRadius = g->DiskScaleRadius;

    if (g->Cooling > 0.0) {
        o->Cooling = log10(g->Cooling * UnitEnergy_in_cgs / UnitTime_in_s);
    } else {
        o->Cooling = 0.0;
    }

    if (g->Heating > 0.0) {
        o->Heating = log10(g->Heating * UnitEnergy_in_cgs / UnitTime_in_s);
    } else {
        o->Heating = 0.0;
    }

    o->QuasarModeBHaccretionMass = g->QuasarModeBHaccretionMass;

    o->TimeOfLastMajorMerger = g->TimeOfLastMajorMerger * UnitTime_in_Megayears;
    o->TimeOfLastMinorMerger = g->TimeOfLastMinorMerger * UnitTime_in_Megayears;
	
    o->OutflowRate = g->OutflowRate * UnitMass_in_g / UnitTime_in_s * SEC_PER_YEAR / SOLAR_MASS;

    //infall properties
    if(g->Type != 0) {
        o->infallMvir = g->infallMvir;
        o->infallVvir = g->infallVvir;
        o->infallVmax = g->infallVmax;
    } else {
        o->infallMvir = 0.0;
        o->infallVvir = 0.0;
        o->infallVmax = 0.0;
    }
}


void finalize_galaxy_file(const int ntrees, const int *totgalaxies, const int **treengals)    
{
    for(int n = 0; n < NOUT; n++) {
        // file must already be open.
        assert( save_fd[n] );

        // seek to the beginning.
        fseek( save_fd[n], 0, SEEK_SET );

        int nwritten = myfwrite(&ntrees, sizeof(int), 1, save_fd[n]);
        if (nwritten != 1) {
            fprintf(stderr, "Error: Failed to write out 1 element for the number of trees for the header of file %d.\n"
                    "Only wrote %d elements.\n", n, nwritten);
        }

        nwritten = myfwrite(&totgalaxies[n], sizeof(int), 1, save_fd[n]); 
        if (nwritten != 1) {
            fprintf(stderr, "Error: Failed to write out 1 element for the number of galaxies for the header of file %d.\n"
                    "Only wrote %d elements.\n", n, nwritten);
        }
        
        nwritten = myfwrite(treengals[n], sizeof(int), ntrees, save_fd[n]);
        if (nwritten != ntrees) {
            fprintf(stderr, "Error: Failed to write out %d elements for the number of galaxies per tree for the header of file %d.\n"
                    "Only wrote %d elements.\n", ntrees, n, nwritten);
        }


        // close the file and clear handle after everything has been written
        fclose( save_fd[n] );
        save_fd[n] = NULL;
    }
}

