#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "globals.h"
#include "types.h"
#include "config.h"
#include "constants.h"
#include "core_proto.h"

void read_parameter_file(char *fname)
{
  FILE *fd;
#define MAX_BUF_SIZE_FILE_LIST (3*MAX_STRING_LEN)
  char buf[MAX_BUF_SIZE_FILE_LIST];
  char buf1[MAX_STRING_LEN];
  char buf2[MAX_STRING_LEN], buf3[MAX_STRING_LEN];
  int i, j, done;
  int errorFlag = 0;
  int *used_tag = 0;
  char my_treetype[MAX_STRING_LEN];
  NParam = 0;

#ifdef MPI
  if(ThisTask == 0)
#endif
    printf("\nreading parameter file:\n\n");

  strcpy(ParamTag[NParam], "FileNameGalaxies");
  ParamAddr[NParam] = SageConfig.FileNameGalaxies;
  ParamID[NParam++] = STRING;

  strcpy(ParamTag[NParam], "OutputDir");
  ParamAddr[NParam] = SageConfig.OutputDir;
  ParamID[NParam++] = STRING;

  strcpy(ParamTag[NParam], "TreeType");
  ParamAddr[NParam] = my_treetype;
  ParamID[NParam++] = STRING;

  strcpy(ParamTag[NParam], "TreeName");
  ParamAddr[NParam] = SageConfig.TreeName;
  ParamID[NParam++] = STRING;

  strcpy(ParamTag[NParam], "SimulationDir");
  ParamAddr[NParam] = SageConfig.SimulationDir;
  ParamID[NParam++] = STRING;

  strcpy(ParamTag[NParam], "FileWithSnapList");
  ParamAddr[NParam] = SageConfig.FileWithSnapList;
  ParamID[NParam++] = STRING;

  strcpy(ParamTag[NParam], "LastSnapShotNr");
  ParamAddr[NParam] = &SageConfig.LastSnapShotNr;
  ParamID[NParam++] = INT;

  strcpy(ParamTag[NParam], "FirstFile");
  ParamAddr[NParam] = &SageConfig.FirstFile;
  ParamID[NParam++] = INT;

  strcpy(ParamTag[NParam], "LastFile");
  ParamAddr[NParam] = &SageConfig.LastFile;
  ParamID[NParam++] = INT;

  strcpy(ParamTag[NParam], "ThreshMajorMerger");
  ParamAddr[NParam] = &SageConfig.ThreshMajorMerger;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "RecycleFraction");
  ParamAddr[NParam] = &SageConfig.RecycleFraction;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "ReIncorporationFactor");
  ParamAddr[NParam] = &SageConfig.ReIncorporationFactor;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "UnitVelocity_in_cm_per_s");
  ParamAddr[NParam] = &UnitVelocity_in_cm_per_s;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "UnitLength_in_cm");
  ParamAddr[NParam] = &UnitLength_in_cm;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "UnitMass_in_g");
  ParamAddr[NParam] = &UnitMass_in_g;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "Hubble_h");
  ParamAddr[NParam] = &SageConfig.Hubble_h;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "ReionizationOn");
  ParamAddr[NParam] = &SageConfig.ReionizationOn;
  ParamID[NParam++] = INT;

  strcpy(ParamTag[NParam], "SupernovaRecipeOn");
  ParamAddr[NParam] = &SageConfig.SupernovaRecipeOn;
  ParamID[NParam++] = INT;

  strcpy(ParamTag[NParam], "DiskInstabilityOn");
  ParamAddr[NParam] = &SageConfig.DiskInstabilityOn;
  ParamID[NParam++] = INT;

  strcpy(ParamTag[NParam], "SFprescription");
  ParamAddr[NParam] = &SageConfig.SFprescription;
  ParamID[NParam++] = INT;

  strcpy(ParamTag[NParam], "AGNrecipeOn");
  ParamAddr[NParam] = &SageConfig.AGNrecipeOn;
  ParamID[NParam++] = INT;

  strcpy(ParamTag[NParam], "BaryonFrac");
  ParamAddr[NParam] = &SageConfig.BaryonFrac;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "Omega");
  ParamAddr[NParam] = &SageConfig.Omega;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "OmegaLambda");
  ParamAddr[NParam] = &SageConfig.OmegaLambda;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "PartMass");
  ParamAddr[NParam] = &SageConfig.PartMass;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "EnergySN");
  ParamAddr[NParam] = &SageConfig.EnergySN;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "EtaSN");
  ParamAddr[NParam] = &SageConfig.EtaSN;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "Yield");
  ParamAddr[NParam] = &SageConfig.Yield;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "FracZleaveDisk");
  ParamAddr[NParam] = &SageConfig.FracZleaveDisk;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "SfrEfficiency");
  ParamAddr[NParam] = &SageConfig.SfrEfficiency;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "FeedbackReheatingEpsilon");
  ParamAddr[NParam] = &SageConfig.FeedbackReheatingEpsilon;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "FeedbackEjectionEfficiency");
  ParamAddr[NParam] = &SageConfig.FeedbackEjectionEfficiency;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "BlackHoleGrowthRate");
  ParamAddr[NParam] = &SageConfig.BlackHoleGrowthRate;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "RadioModeEfficiency");
  ParamAddr[NParam] = &SageConfig.RadioModeEfficiency;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "QuasarModeEfficiency");
  ParamAddr[NParam] = &SageConfig.QuasarModeEfficiency;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "Reionization_z0");
  ParamAddr[NParam] = &SageConfig.Reionization_z0;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "Reionization_zr");
  ParamAddr[NParam] = &SageConfig.Reionization_zr;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "ThresholdSatDisruption");
  ParamAddr[NParam] = &SageConfig.ThresholdSatDisruption;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "NumOutputs");
  ParamAddr[NParam] = &SageConfig.NOUT;
  ParamID[NParam++] = INT;

  used_tag = mymalloc(sizeof(int) * NParam);
  for(i=0; i<NParam; i++)
    used_tag[i]=1;

  fd = fopen(fname, "r");
  if (fd == NULL) {
    printf("Parameter file %s not found.\n", fname);
    errorFlag = 1;
  }

  if(fd != NULL)
  {
    while(!feof(fd))
      {
        *buf = 0;
        fgets(buf, MAX_BUF_SIZE_FILE_LIST, fd);
        if(sscanf(buf, "%s%s%s", buf1, buf2, buf3) < 2)
          continue;

        if(buf1[0] == '%' || buf1[0] == '-')
          continue;

        for(i = 0, j = -1; i < NParam; i++)
          if(strcmp(buf1, ParamTag[i]) == 0)
            {
              j = i;
              ParamTag[i][0] = 0;
              used_tag[i] = 0;
              break;
            }

        if(j >= 0)
          {
#ifdef MPI
            if(ThisTask == 0)
#endif
              printf("%35s\t%10s\n", buf1, buf2);

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
          }
        else
        {
          printf("Error in file %s:   Tag '%s' not allowed or multiply defined.\n", fname, buf1);
          errorFlag = 1;
        }
      }
    fclose(fd);

    i = strlen(SageConfig.OutputDir);
    if(i > 0)
      if(SageConfig.OutputDir[i - 1] != '/')
        strcat(SageConfig.OutputDir, "/");
  }

  for(i = 0; i < NParam; i++)
    {
      if(used_tag[i])
        {
          printf("Error. I miss a value for tag '%s' in parameter file '%s'.\n", ParamTag[i], fname);
          errorFlag = 1;
        }
    }

  if(errorFlag) {
    ABORT(1);
  }
  printf("\n");

  if( ! (SageConfig.LastSnapShotNr+1 > 0 && SageConfig.LastSnapShotNr+1 < ABSOLUTEMAXSNAPS) ) {
    fprintf(stderr,"LastSnapshotNr = %d should be in [0, %d) \n", SageConfig.LastSnapShotNr, ABSOLUTEMAXSNAPS);
    ABORT(1);
  }
  MAXSNAPS = SageConfig.LastSnapShotNr + 1;

  if(!(SageConfig.NOUT == -1 || (SageConfig.NOUT > 0 && SageConfig.NOUT <= ABSOLUTEMAXSNAPS))) {
    fprintf(stderr,"NumOutputs must be -1 or between 1 and %i\n", ABSOLUTEMAXSNAPS);
    ABORT(1);
  }

  // read in the output snapshot list
  if(SageConfig.NOUT == -1)
    {
      SageConfig.NOUT = MAXSNAPS;
      for (i=SageConfig.NOUT-1; i>=0; i--)
        ListOutputSnaps[i] = i;
      printf("all %i snapshots selected for output\n", SageConfig.NOUT);
    }
  else
    {
      printf("%i snapshots selected for output: ", SageConfig.NOUT);
      // reopen the parameter file
      fd = fopen(fname, "r");

      done = 0;
      while(!feof(fd) && !done)
        {
          // scan down to find the line with the snapshots
          fscanf(fd, "%s", buf);
          if(strcmp(buf, "->") == 0)
            {
              // read the snapshots into ListOutputSnaps
              for (i=0; i<SageConfig.NOUT; i++)
                {
                  fscanf(fd, "%d", &ListOutputSnaps[i]);
                  printf("%i ", ListOutputSnaps[i]);
                }
              done = 1;
            }
        }

      fclose(fd);
      if(! done ) {
        fprintf(stderr,"Error: Could not properly parse output snapshots\n");
        ABORT(2);
      }
      printf("\n");
    }
    
  // Sync the global variable with the config structure
  NOUT = SageConfig.NOUT;
  printf("Debug: core_read_parameter_file.c - Set NOUT=%d from SageConfig.NOUT=%d\n", NOUT, SageConfig.NOUT);

  // Check file type is valid.
  if (strncmp(my_treetype, "lhalo_binary", 511) != 0) // strncmp returns 0 if the two strings are equal. Only available options are HDF5 or binary files.
  {
    snprintf(SageConfig.TreeExtension, 511, ".hdf5");
#ifndef HDF5
    fprintf(stderr, "You have specified to use a HDF5 file but have no compiled with the HDF5 option enabled.\n");
    fprintf(stderr, "Please check your file type and compiler options.\n");
    ABORT(0);
#endif
  }

  // Recast the local treetype string to a global TreeType enum.

  if (strcasecmp(my_treetype, "genesis_lhalo_hdf5") == 0)
  {
    SageConfig.TreeType = genesis_lhalo_hdf5;
  }
  else if (strcasecmp(my_treetype, "lhalo_binary") == 0)
  {
    SageConfig.TreeType = lhalo_binary;
  }
  else
  {
    fprintf(stderr, "TreeType %s is not supported\n", my_treetype);
    ABORT(0);
  }

  myfree(used_tag);

}
