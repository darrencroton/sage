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

#include "globals.h"
#include "config.h"
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

  // Default log level
  LogLevel log_level = LOG_LEVEL_INFO;
  
  // Check for special flags
  int i;
  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      // Initialize error handling early for proper message formatting
      initialize_error_handling(log_level, NULL);
      
      // Display help and exit
      INFO_LOG("SAGE Help");
      printf("\nSAGE Semi-Analytic Galaxy Evolution Model\n");
      printf("Usage: sage [options] <parameterfile>\n\n");
      printf("Options:\n");
      printf("  -h, --help       Display this help message and exit\n");
      printf("  -v, --verbose    Show debug messages (most verbose)\n");
      printf("  -q, --quiet      Show only warnings and errors (least verbose)\n\n");
      exit(0);
    } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
      log_level = LOG_LEVEL_DEBUG;
      // Remove the argument
      int k;
      for (k = i; k < argc - 1; k++) {
        argv[k] = argv[k + 1];
      }
      argc--;
      i--;
    } else if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--quiet") == 0) {
      log_level = LOG_LEVEL_WARNING;
      // Remove the argument
      int k;
      for (k = i; k < argc - 1; k++) {
        argv[k] = argv[k + 1];
      }
      argc--;
      i--;
    }
  }
  
  if(argc != 2)
  {
    FATAL_ERROR("Incorrect usage! Please use: sage [options] <parameterfile>\n"
                "For help, use: sage --help");
  }

  atexit(bye);

  sigaction(SIGXCPU, NULL, &saveaction_XCPU);
  current_XCPU = saveaction_XCPU;
  current_XCPU.sa_handler = termination_handler;
  sigaction(SIGXCPU, &current_XCPU, NULL);

  // Initialize error handling system with the log level determined from command line
  // This should be done before any other initialization that might produce errors
  initialize_error_handling(log_level, NULL);
  
  // Log the start of the program with different verbosity levels
  DEBUG_LOG("Starting SAGE with verbosity level: %s", get_log_level_name(log_level));
  INFO_LOG("SAGE Semi-Analytic Galaxy Evolution model starting up");
  
  // These will only show if the verbosity is high enough
  DEBUG_LOG("Command line argument count: %d", argc);
  int j;
  for (j = 0; j < argc; j++) {
    DEBUG_LOG("Argument %d: %s", j, argv[j]);
  }
  
  read_parameter_file(argv[1]);
  init();

#ifdef MPI
  for(filenr = SageConfig.FirstFile+ThisTask; filenr <= SageConfig.LastFile; filenr += NTask)
#else
  for(filenr = SageConfig.FirstFile; filenr <= SageConfig.LastFile; filenr++)
#endif
  {
    snprintf(bufz0, MAX_BUFZ0_SIZE, "%s/%s.%d%s", SageConfig.SimulationDir, SageConfig.TreeName, filenr, SageConfig.TreeExtension);
    if(!(fd = fopen(bufz0, "r")))
    {
      INFO_LOG("Missing tree %s ... skipping", bufz0);
      continue;  // tree file does not exist, move along
    }
    else
      fclose(fd);

    snprintf(bufz0, MAX_BUFZ0_SIZE, "%s/%s_z%1.3f_%d", SageConfig.OutputDir, SageConfig.FileNameGalaxies, ZZ[ListOutputSnaps[0]], filenr);
    if(stat(bufz0, &filestatus) == 0)
    {
      INFO_LOG("Output for tree %s already exists ... skipping", bufz0);
      continue;  // output seems to already exist, dont overwrite, move along
    }

    if((fd = fopen(bufz0, "w")))
      fclose(fd);

    FileNum = filenr;
    load_tree_table(filenr, SageConfig.TreeType);

    for(treenr = 0; treenr < Ntrees; treenr++)
    {

                        assert(!gotXCPU);

      if(treenr % 10000 == 0)
      {
#ifdef MPI
        INFO_LOG("Processing task: %d node: %s file: %i tree: %i of %i", ThisTask, ThisNode, filenr, treenr, Ntrees);
#else
        INFO_LOG("Processing file: %i tree: %i of %i", filenr, treenr, Ntrees);
#endif
      }

      TreeID = treenr;
      load_tree(filenr, treenr, SageConfig.TreeType);

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
    free_tree_table(SageConfig.TreeType);

    INFO_LOG("Completed processing file %d", filenr);
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
