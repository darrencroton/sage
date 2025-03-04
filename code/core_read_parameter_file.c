#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "core_allvars.h"
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
  ParamAddr[NParam] = FileNameGalaxies;
  ParamID[NParam++] = STRING;

  strcpy(ParamTag[NParam], "OutputDir");
  ParamAddr[NParam] = OutputDir;
  ParamID[NParam++] = STRING;

  strcpy(ParamTag[NParam], "TreeType");
  ParamAddr[NParam] = my_treetype;
  ParamID[NParam++] = STRING;

  strcpy(ParamTag[NParam], "TreeName");
  ParamAddr[NParam] = TreeName;
  ParamID[NParam++] = STRING;

  strcpy(ParamTag[NParam], "SimulationDir");
  ParamAddr[NParam] = SimulationDir;
  ParamID[NParam++] = STRING;

  strcpy(ParamTag[NParam], "FileWithSnapList");
  ParamAddr[NParam] = FileWithSnapList;
  ParamID[NParam++] = STRING;

  strcpy(ParamTag[NParam], "LastSnapShotNr");
  ParamAddr[NParam] = &LastSnapShotNr;
  ParamID[NParam++] = INT;

  strcpy(ParamTag[NParam], "FirstFile");
  ParamAddr[NParam] = &FirstFile;
  ParamID[NParam++] = INT;

  strcpy(ParamTag[NParam], "LastFile");
  ParamAddr[NParam] = &LastFile;
  ParamID[NParam++] = INT;

  strcpy(ParamTag[NParam], "ThreshMajorMerger");
  ParamAddr[NParam] = &ThreshMajorMerger;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "RecycleFraction");
  ParamAddr[NParam] = &RecycleFraction;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "ReIncorporationFactor");
  ParamAddr[NParam] = &ReIncorporationFactor;
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
  ParamAddr[NParam] = &Hubble_h;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "ReionizationOn");
  ParamAddr[NParam] = &ReionizationOn;
  ParamID[NParam++] = INT;

  strcpy(ParamTag[NParam], "SupernovaRecipeOn");
  ParamAddr[NParam] = &SupernovaRecipeOn;
  ParamID[NParam++] = INT;

  strcpy(ParamTag[NParam], "DiskInstabilityOn");
  ParamAddr[NParam] = &DiskInstabilityOn;
  ParamID[NParam++] = INT;

  strcpy(ParamTag[NParam], "SFprescription");
  ParamAddr[NParam] = &SFprescription;
  ParamID[NParam++] = INT;

  strcpy(ParamTag[NParam], "AGNrecipeOn");
  ParamAddr[NParam] = &AGNrecipeOn;
  ParamID[NParam++] = INT;

  strcpy(ParamTag[NParam], "BaryonFrac");
  ParamAddr[NParam] = &BaryonFrac;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "Omega");
  ParamAddr[NParam] = &Omega;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "OmegaLambda");
  ParamAddr[NParam] = &OmegaLambda;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "PartMass");
  ParamAddr[NParam] = &PartMass;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "EnergySN");
  ParamAddr[NParam] = &EnergySN;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "EtaSN");
  ParamAddr[NParam] = &EtaSN;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "Yield");
  ParamAddr[NParam] = &Yield;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "FracZleaveDisk");
  ParamAddr[NParam] = &FracZleaveDisk;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "SfrEfficiency");
  ParamAddr[NParam] = &SfrEfficiency;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "FeedbackReheatingEpsilon");
  ParamAddr[NParam] = &FeedbackReheatingEpsilon;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "FeedbackEjectionEfficiency");
  ParamAddr[NParam] = &FeedbackEjectionEfficiency;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "BlackHoleGrowthRate");
  ParamAddr[NParam] = &BlackHoleGrowthRate;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "RadioModeEfficiency");
  ParamAddr[NParam] = &RadioModeEfficiency;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "QuasarModeEfficiency");
  ParamAddr[NParam] = &QuasarModeEfficiency;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "Reionization_z0");
  ParamAddr[NParam] = &Reionization_z0;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "Reionization_zr");
  ParamAddr[NParam] = &Reionization_zr;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "ThresholdSatDisruption");
  ParamAddr[NParam] = &ThresholdSatDisruption;
  ParamID[NParam++] = DOUBLE;

  strcpy(ParamTag[NParam], "NumOutputs");
  ParamAddr[NParam] = &NOUT;
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

    i = strlen(OutputDir);
    if(i > 0)
      if(OutputDir[i - 1] != '/')
        strcat(OutputDir, "/");
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

  if( ! (LastSnapShotNr+1 > 0 && LastSnapShotNr+1 < ABSOLUTEMAXSNAPS) ) {
    fprintf(stderr,"LastSnapshotNr = %d should be in [0, %d) \n", LastSnapShotNr, ABSOLUTEMAXSNAPS);
    ABORT(1);
  }
  MAXSNAPS = LastSnapShotNr + 1;

  if(!(NOUT == -1 || (NOUT > 0 && NOUT <= ABSOLUTEMAXSNAPS))) {
    fprintf(stderr,"NumOutputs must be -1 or between 1 and %i\n", ABSOLUTEMAXSNAPS);
    ABORT(1);
  }

  // read in the output snapshot list
  if(NOUT == -1)
    {
      NOUT = MAXSNAPS;
      for (i=NOUT-1; i>=0; i--)
        ListOutputSnaps[i] = i;
      printf("all %i snapshots selected for output\n", NOUT);
    }
  else
    {
      printf("%i snapshots selected for output: ", NOUT);
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
              for (i=0; i<NOUT; i++)
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

  // Check file type is valid.
  if (strncmp(my_treetype, "lhalo_binary", 511) != 0) // strncmp returns 0 if the two strings are equal. Only available options are HDF5 or binary files.
  {
    snprintf(TreeExtension, 511, ".hdf5");
#ifndef HDF5
    fprintf(stderr, "You have specified to use a HDF5 file but have no compiled with the HDF5 option enabled.\n");
    fprintf(stderr, "Please check your file type and compiler options.\n");
    ABORT(0);
#endif
  }

  // Recast the local treetype string to a global TreeType enum.

  if (strcasecmp(my_treetype, "genesis_lhalo_hdf5") == 0)
  {
    TreeType = genesis_lhalo_hdf5;
  }
  else if (strcasecmp(my_treetype, "lhalo_binary") == 0)
  {
    TreeType = lhalo_binary;
  }
  else
  {
    fprintf(stderr, "TreeType %s is not supported\n", my_treetype);
    ABORT(0);
  }

  myfree(used_tag);

}
