#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>

#ifdef MPI
#include <mpi.h>
#endif

#include "core_allvars.h"
#include "core_proto.h"

#include "io/io_save_hdf5.h"

#define MAX_BUFZ0_SIZE (3*MAX_STRING_LEN+25)
char bufz0[MAX_BUFZ0_SIZE+1]; /* 3 strings + max 19 bytes for a number */
int exitfail = 1;

struct sigaction saveaction_XCPU;
volatile sig_atomic_t gotXCPU = 0;



void termination_handler(int signum)
{
  gotXCPU = 1;
  sigaction(SIGXCPU, &saveaction_XCPU, NULL);
  if(saveaction_XCPU.sa_handler != NULL)
    (*saveaction_XCPU.sa_handler) (signum);
}



void myexit(int signum)
{
#ifdef MPI
  printf("Task: %d\tnode: %s\tis exiting\n\n\n", ThisTask, ThisNode);
#else
  printf("We're exiting\n\n\n");
#endif
          exit(signum);
}



void bye()
{
#ifdef MPI
  MPI_Finalize();
  free(ThisNode);
#endif

  if(exitfail)
  {
    unlink(bufz0);

#ifdef MPI
    if(ThisTask == 0 && gotXCPU == 1)
      printf("Received XCPU, exiting. But we'll be back.\n");
#endif
          }
}



int main(int argc, char **argv)
{
  int filenr, treenr, halonr;
  struct sigaction current_XCPU;

  struct stat filestatus;
  FILE *fd;

#ifdef MPI
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &ThisTask);
  MPI_Comm_size(MPI_COMM_WORLD, &NTask);

  ThisNode = malloc(MPI_MAX_PROCESSOR_NAME * sizeof(char));

  MPI_Get_processor_name(ThisNode, &nodeNameLen);
  if (nodeNameLen >= MPI_MAX_PROCESSOR_NAME)
  {
    printf("Node name string not long enough!...\n");
    ABORT(0);
  }
#endif

  if(argc != 2)
  {
    printf("\n  usage: sage <parameterfile>\n\n");
    ABORT(0);
  }

  atexit(bye);

  sigaction(SIGXCPU, NULL, &saveaction_XCPU);
  current_XCPU = saveaction_XCPU;
  current_XCPU.sa_handler = termination_handler;
  sigaction(SIGXCPU, &current_XCPU, NULL);

  read_parameter_file(argv[1]);
  init();

#ifdef MPI
  for(filenr = FirstFile+ThisTask; filenr <= LastFile; filenr += NTask)
#else
  for(filenr = FirstFile; filenr <= LastFile; filenr++)
#endif
  {
    snprintf(bufz0, MAX_BUFZ0_SIZE, "%s/%s.%d%s", SimulationDir, TreeName, filenr, TreeExtension);
    if(!(fd = fopen(bufz0, "r")))
    {
      printf("-- missing tree %s ... skipping\n", bufz0);
      continue;  // tree file does not exist, move along
    }
    else
      fclose(fd);

    snprintf(bufz0, MAX_BUFZ0_SIZE, "%s/%s_z%1.3f_%d", OutputDir, FileNameGalaxies, ZZ[ListOutputSnaps[0]], filenr);
    if(stat(bufz0, &filestatus) == 0)
    {
      printf("-- output for tree %s already exists ... skipping\n", bufz0);
      continue;  // output seems to already exist, dont overwrite, move along
    }

    if((fd = fopen(bufz0, "w")))
      fclose(fd);

    FileNum = filenr;
    load_tree_table(filenr, TreeType);

    for(treenr = 0; treenr < Ntrees; treenr++)
    {

                        assert(!gotXCPU);

      if(treenr % 10000 == 0)
      {
#ifdef MPI
        printf("\ttask: %d\tnode: %s\tfile: %i\ttree: %i of %i\n", ThisTask, ThisNode, filenr, treenr, Ntrees);
#else
                                printf("\tfile: %i\ttree: %i of %i\n", filenr, treenr, Ntrees);
#endif
                                fflush(stdout);
      }

      TreeID = treenr;
      load_tree(filenr, treenr, TreeType);

      gsl_rng_set(random_generator, filenr * 100000 + treenr);
      NumGals = 0;
      GalaxyCounter = 0;
      for(halonr = 0; halonr < TreeNHalos[treenr]; halonr++)
        if(HaloAux[halonr].DoneFlag == 0)
        construct_galaxies(halonr, treenr);

      save_galaxies(filenr, treenr);
      free_galaxies_and_tree();
    }

    finalize_galaxy_file(filenr);
    free_tree_table(TreeType);

    printf("\ndone file %d\n\n", filenr);
  }

/*
  if(HDF5Output){
    free_hdf5_ids();

#ifdef MPI
    // Create a single master HDF5 file with links to the other files...
    MPI_Barrier(MPI_COMM_WORLD);
    if (ThisTask == 0)
#endif
      //write_master_file();

  }
*/


  //free Ages. But first
  //reset Age to the actual allocated address
  Age--;
  myfree(Age);

  gsl_rng_free(random_generator);

  exitfail = 0;
  return 0;
}
