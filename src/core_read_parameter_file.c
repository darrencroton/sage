#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "core_allvars.h"
#include "core_mymalloc.h"

void read_parameter_file(const int ThisTask, const char *fname)
{
    int errorFlag = 0;
    int *used_tag = 0;
    char my_treetype[MAX_STRING_LEN];
    /*  recipe parameters  */
    int NParam = 0;
    char ParamTag[MAXTAGS][50];
    int  ParamID[MAXTAGS];
    void *ParamAddr[MAXTAGS];

    NParam = 0;

    if(ThisTask == 0) {
        printf("\nreading parameter file:\n\n");
    }

    strcpy(ParamTag[NParam], "FileNameGalaxies");
    ParamAddr[NParam] = run_params.FileNameGalaxies;
    ParamID[NParam++] = STRING;

    strcpy(ParamTag[NParam], "OutputDir");
    ParamAddr[NParam] = run_params.OutputDir;
    ParamID[NParam++] = STRING;

    strcpy(ParamTag[NParam], "TreeType");
    ParamAddr[NParam] = my_treetype;
    ParamID[NParam++] = STRING;

    strcpy(ParamTag[NParam], "TreeName");
    ParamAddr[NParam] = run_params.TreeName;
    ParamID[NParam++] = STRING;

    strcpy(ParamTag[NParam], "SimulationDir");
    ParamAddr[NParam] = run_params.SimulationDir;
    ParamID[NParam++] = STRING;

    strcpy(ParamTag[NParam], "FileWithSnapList");
    ParamAddr[NParam] = run_params.FileWithSnapList;
    ParamID[NParam++] = STRING;

    strcpy(ParamTag[NParam], "LastSnapShotNr");
    ParamAddr[NParam] = &(run_params.LastSnapShotNr);
    ParamID[NParam++] = INT;

    strcpy(ParamTag[NParam], "FirstFile");
    ParamAddr[NParam] = &(run_params.FirstFile);
    ParamID[NParam++] = INT;

    strcpy(ParamTag[NParam], "LastFile");
    ParamAddr[NParam] = &(run_params.LastFile);
    ParamID[NParam++] = INT;

    strcpy(ParamTag[NParam], "ThreshMajorMerger");
    ParamAddr[NParam] = &(run_params.ThreshMajorMerger);
    ParamID[NParam++] = DOUBLE;

    strcpy(ParamTag[NParam], "RecycleFraction");
    ParamAddr[NParam] = &(run_params.RecycleFraction);
    ParamID[NParam++] = DOUBLE;

    strcpy(ParamTag[NParam], "ReIncorporationFactor");
    ParamAddr[NParam] = &(run_params.ReIncorporationFactor);
    ParamID[NParam++] = DOUBLE;

    strcpy(ParamTag[NParam], "UnitVelocity_in_cm_per_s");
    ParamAddr[NParam] = &(run_params.UnitVelocity_in_cm_per_s);
    ParamID[NParam++] = DOUBLE;

    strcpy(ParamTag[NParam], "UnitLength_in_cm");
    ParamAddr[NParam] = &(run_params.UnitLength_in_cm);
    ParamID[NParam++] = DOUBLE;

    strcpy(ParamTag[NParam], "UnitMass_in_g");
    ParamAddr[NParam] = &(run_params.UnitMass_in_g);
    ParamID[NParam++] = DOUBLE;

    strcpy(ParamTag[NParam], "Hubble_h");
    ParamAddr[NParam] = &(run_params.Hubble_h);
    ParamID[NParam++] = DOUBLE;

    strcpy(ParamTag[NParam], "ReionizationOn");
    ParamAddr[NParam] = &(run_params.ReionizationOn);
    ParamID[NParam++] = INT;

    strcpy(ParamTag[NParam], "SupernovaRecipeOn");
    ParamAddr[NParam] = &(run_params.SupernovaRecipeOn);
    ParamID[NParam++] = INT;

    strcpy(ParamTag[NParam], "DiskInstabilityOn");
    ParamAddr[NParam] = &(run_params.DiskInstabilityOn);
    ParamID[NParam++] = INT;

    strcpy(ParamTag[NParam], "SFprescription");
    ParamAddr[NParam] = &(run_params.SFprescription);
    ParamID[NParam++] = INT;

    strcpy(ParamTag[NParam], "AGNrecipeOn");
    ParamAddr[NParam] = &(run_params.AGNrecipeOn);
    ParamID[NParam++] = INT;

    strcpy(ParamTag[NParam], "BaryonFrac");
    ParamAddr[NParam] = &(run_params.BaryonFrac);
    ParamID[NParam++] = DOUBLE;

    strcpy(ParamTag[NParam], "Omega");
    ParamAddr[NParam] = &(run_params.Omega);
    ParamID[NParam++] = DOUBLE;

    strcpy(ParamTag[NParam], "OmegaLambda");
    ParamAddr[NParam] = &(run_params.OmegaLambda);
    ParamID[NParam++] = DOUBLE;

    strcpy(ParamTag[NParam], "PartMass");
    ParamAddr[NParam] = &(run_params.PartMass);
    ParamID[NParam++] = DOUBLE;

    strcpy(ParamTag[NParam], "EnergySN");
    ParamAddr[NParam] = &(run_params.EnergySN);
    ParamID[NParam++] = DOUBLE;

    strcpy(ParamTag[NParam], "EtaSN");
    ParamAddr[NParam] = &(run_params.EtaSN);
    ParamID[NParam++] = DOUBLE;

    strcpy(ParamTag[NParam], "Yield");
    ParamAddr[NParam] = &(run_params.Yield);
    ParamID[NParam++] = DOUBLE;

    strcpy(ParamTag[NParam], "FracZleaveDisk");
    ParamAddr[NParam] = &(run_params.FracZleaveDisk);
    ParamID[NParam++] = DOUBLE;
    
    strcpy(ParamTag[NParam], "SfrEfficiency");
    ParamAddr[NParam] = &(run_params.SfrEfficiency);
    ParamID[NParam++] = DOUBLE;
    
    strcpy(ParamTag[NParam], "FeedbackReheatingEpsilon");
    ParamAddr[NParam] = &(run_params.FeedbackReheatingEpsilon);
    ParamID[NParam++] = DOUBLE;
    
    strcpy(ParamTag[NParam], "FeedbackEjectionEfficiency");
    ParamAddr[NParam] = &(run_params.FeedbackEjectionEfficiency);
    ParamID[NParam++] = DOUBLE;
    
    strcpy(ParamTag[NParam], "BlackHoleGrowthRate");
    ParamAddr[NParam] = &(run_params.BlackHoleGrowthRate);
    ParamID[NParam++] = DOUBLE;

    strcpy(ParamTag[NParam], "RadioModeEfficiency");
    ParamAddr[NParam] = &(run_params.RadioModeEfficiency);
    ParamID[NParam++] = DOUBLE;

    strcpy(ParamTag[NParam], "QuasarModeEfficiency");
    ParamAddr[NParam] = &(run_params.QuasarModeEfficiency);
    ParamID[NParam++] = DOUBLE;

    strcpy(ParamTag[NParam], "Reionization_z0");
    ParamAddr[NParam] = &(run_params.Reionization_z0);
    ParamID[NParam++] = DOUBLE;
    
    strcpy(ParamTag[NParam], "Reionization_zr");
    ParamAddr[NParam] = &(run_params.Reionization_zr);
    ParamID[NParam++] = DOUBLE;

    strcpy(ParamTag[NParam], "ThresholdSatDisruption");
    ParamAddr[NParam] = &(run_params.ThresholdSatDisruption);
    ParamID[NParam++] = DOUBLE;

    strcpy(ParamTag[NParam], "NumOutputs");
    ParamAddr[NParam] = &(run_params.NOUT);
    ParamID[NParam++] = INT;

    used_tag = mymalloc(sizeof(int) * NParam);
    for(int i=0; i<NParam; i++) {
        used_tag[i]=1;
    }

    FILE *fd = fopen(fname, "r");
    if (fd == NULL) {
        printf("Parameter file %s not found.\n", fname);
        errorFlag = 1;
    }
  
    if(fd != NULL) {
        char buffer[MAX_STRING_LEN];
        while(fgets(&(buffer[0]), MAX_STRING_LEN, fd) != NULL) {
            char buf1[MAX_STRING_LEN], buf2[MAX_STRING_LEN];
            if(sscanf(buffer, "%s%s%*[^\n]", buf1, buf2) < 2) {
                continue;
            }
          
            if(buf1[0] == '%' || buf1[0] == '-') {
                continue;
            }

            int j=-1;
            for(int i = 0; i < NParam; i++) {
                if(strcmp(buf1, ParamTag[i]) == 0) {
                    j = i;
                    ParamTag[i][0] = 0;
                    used_tag[i] = 0;
                    break;
                }
            }
          
            if(j >= 0) {
                if(ThisTask == 0) {
                    printf("%35s\t%10s\n", buf1, buf2);
                }
                  
                switch (ParamID[j])
                    {
                    case DOUBLE:
                        *((double *) ParamAddr[j]) = atof(buf2);
                        break;
                    case STRING:
                        strcpy(ParamAddr[j], buf2);
                        break;
                    case INT:
                        *((int *) ParamAddr[j]) = atoi(buf2);
                        break;
                    }
            } else {
                printf("Error in file %s:   Tag '%s' not allowed or multiply defined.\n", fname, buf1);
                errorFlag = 1;
            }
        }
        fclose(fd);
      
        int i = strlen(run_params.OutputDir);
        if(i > 0) {
            if(run_params.OutputDir[i - 1] != '/')
                strcat(run_params.OutputDir, "/");
        }
    }
  
    for(int i = 0; i < NParam; i++) {
        if(used_tag[i]) {
            printf("Error. I miss a value for tag '%s' in parameter file '%s'.\n", ParamTag[i], fname);
            errorFlag = 1;
        }
    }
  
    if(errorFlag) {
        ABORT(1);
    }
    printf("\n");
  
    if( ! (run_params.LastSnapShotNr+1 > 0 && run_params.LastSnapShotNr+1 < ABSOLUTEMAXSNAPS) ) {
        fprintf(stderr,"LastSnapshotNr = %d should be in [0, %d) \n", run_params.LastSnapShotNr, ABSOLUTEMAXSNAPS);
        ABORT(1);
    }
    run_params.MAXSNAPS = run_params.LastSnapShotNr + 1;
    
    if(!(run_params.NOUT == -1 || (run_params.NOUT > 0 && run_params.NOUT <= ABSOLUTEMAXSNAPS))) {
        fprintf(stderr,"NumOutputs must be -1 or between 1 and %i\n", ABSOLUTEMAXSNAPS);
        ABORT(1);
    }
  
    // read in the output snapshot list
    if(run_params.NOUT == -1) {
        run_params.NOUT = run_params.MAXSNAPS;
        for (int i=run_params.NOUT-1; i>=0; i--) {
            run_params.ListOutputSnaps[i] = i;
        }
        printf("all %d snapshots selected for output\n", run_params.NOUT);
    } else {
        printf("%d snapshots selected for output: ", run_params.NOUT);
        // reopen the parameter file
        fd = fopen(fname, "r");
        
        int done = 0;
        while(!feof(fd) && !done) {
            char buf[MAX_STRING_LEN];
            
            /* scan down to find the line with the snapshots */
            if(fscanf(fd, "%s", buf) == 0) continue;
            if(strcmp(buf, "->") == 0) {
                // read the snapshots into ListOutputSnaps
                for(int i=0; i<run_params.NOUT; i++) {
                    if(fscanf(fd, "%d", &(run_params.ListOutputSnaps[i])) == 1) { 
                        printf("%d ", run_params.ListOutputSnaps[i]);
                    }
                }
                done = 1;
                break;
            }
        }
      
        fclose(fd);
        if(! done ) {
            fprintf(stderr,"Error: Could not properly parse output snapshots\n");
            ABORT(2);
        }
        printf("\n");
    }

    /* because in the default case of 'lhalo-binary', nothing
       gets written to "treeextension", we need to 
       null terminate tree-extension first
     */
    run_params.TreeExtension[0] = '\0';
    // Check file type is valid. 
    if (strncmp(my_treetype, "lhalo_binary", 511) != 0) { // strncmp returns 0 if the two strings are equal. Only available options are HDF5 or binary files. 
        snprintf(run_params.TreeExtension, 511, ".hdf5");
#ifndef HDF5
        fprintf(stderr, "You have specified to use a HDF5 file but have no compiled with the HDF5 option enabled.\n");
        fprintf(stderr, "Please check your file type and compiler options.\n");
        ABORT(0);
#endif
    }

    // Recast the local treetype string to a global TreeType enum.
    if (strcasecmp(my_treetype, "genesis_lhalo_hdf5") == 0) {
        run_params.TreeType = genesis_lhalo_hdf5;
    } else if (strcasecmp(my_treetype, "lhalo_binary") == 0) {
        run_params.TreeType = lhalo_binary;
    } else {
        fprintf(stderr, "TreeType %s is not supported\n", my_treetype);
        ABORT(0);
    }

    myfree(used_tag);
  
}

