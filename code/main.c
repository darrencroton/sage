/**
 * @file    main.c
 * @brief   Main entry point for the SAGE semi-analytic galaxy evolution model
 *
 * This file contains the main program flow for SAGE, handling initialization,
 * file processing, and the galaxy evolution loop. It coordinates the overall
 * execution of the model, including:
 * - Parameter file reading and initialization
 * - Command-line argument processing
 * - Error handling setup
 * - Tree file loading and traversal
 * - Galaxy construction and evolution
 * - Output file generation
 *
 * Key functions:
 * - main(): Program entry point and core execution loop
 * - termination_handler(): Handles CPU time limit signals
 * - bye(): Performs cleanup on program exit
 */

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
static char bufz0[MAX_BUFZ0_SIZE+1]; /* 3 strings + max 19 bytes for a number */
static int exitfail = 1;  /* Flag indicating whether program exit was due to failure */

static struct sigaction saveaction_XCPU;  /* Saved signal action for SIGXCPU */
static volatile sig_atomic_t gotXCPU = 0; /* Flag indicating whether SIGXCPU was received */


/**
 * @brief   Signal handler for CPU time limit exceeded (SIGXCPU)
 *
 * @param   signum    Signal number that triggered the handler
 *
 * This function sets a flag when the CPU time limit is exceeded and
 * passes control to any previously registered handler if one exists.
 * This allows for graceful termination when running on systems with
 * CPU time limits (e.g., batch systems).
 */

void termination_handler(int signum)
{
  gotXCPU = 1;
  sigaction(SIGXCPU, &saveaction_XCPU, NULL);
  if(saveaction_XCPU.sa_handler != NULL)
    (*saveaction_XCPU.sa_handler) (signum);
}


/**
 * @brief   Exit handler for controlled program termination
 *
 * @param   signum    Exit code to be passed to the OS
 *
 * This function prints a termination message and exits the program.
 * Different messages are displayed in MPI versus serial mode.
 */

void myexit(int signum)
{
#ifdef MPI
  printf("Task: %d\tnode: %s\tis exiting\n\n\n", ThisTask, ThisNode);
#else
  printf("We're exiting\n\n\n");
#endif
          exit(signum);
}


/**
 * @brief   Cleanup function registered with atexit()
 *
 * This function performs cleanup operations before the program terminates,
 * including MPI finalization in parallel mode and temporary file removal.
 * It is automatically called when the program exits.
 */

void bye()
{
#ifdef MPI
  MPI_Finalize();
  free(ThisNode);
#endif

  if(exitfail)
  {
    unlink(bufz0);  /* Remove temporary output file if we're exiting due to failure */

#ifdef MPI
    if(ThisTask == 0 && gotXCPU == 1)
      printf("Received XCPU, exiting. But we'll be back.\n");
#endif
          }
}


/**
 * @brief   Main program entry point
 *
 * @param   argc      Number of command-line arguments
 * @param   argv      Array of command-line argument strings
 * @return  Exit status code (0 for success)
 *
 * This function implements the main program flow:
 * 1. Initialize MPI if compiled with MPI support
 * 2. Process command-line arguments
 * 3. Set up signal handling for CPU time limits
 * 4. Initialize the error handling system
 * 5. Read parameter file and initialize simulation
 * 6. Process merger tree files in parallel (MPI) or serially
 * 7. For each tree:
 *    a. Load the merger tree
 *    b. Construct galaxies by walking the tree
 *    c. Save the resulting galaxies
 * 8. Perform cleanup and exit
 */

int main(int argc, char **argv)
{
  int filenr, treenr, halonr;
  struct sigaction current_XCPU;

  struct stat filestatus;
  FILE *fd;

#ifdef MPI
  /* Initialize MPI environment */
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &ThisTask);  /* Get this processor's task ID */
  MPI_Comm_size(MPI_COMM_WORLD, &NTask);     /* Get total number of processors */

  /* Get the name of this processor's node */
  ThisNode = malloc(MPI_MAX_PROCESSOR_NAME * sizeof(char));
  MPI_Get_processor_name(ThisNode, &nodeNameLen);
  if (nodeNameLen >= MPI_MAX_PROCESSOR_NAME)
  {
    printf("Node name string not long enough!...\n");
    ABORT(0);
  }
#endif

  /* Set default logging level */
  LogLevel log_level = LOG_LEVEL_INFO;
  
  /* Parse command-line arguments for special flags like help, verbosity */
  int i;
  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      /* Initialize error handling early for proper message formatting */
      initialize_error_handling(log_level, NULL);
      
      /* Display help and exit */
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
      /* Remove the argument from argv to simplify later parameter file handling */
      int k;
      for (k = i; k < argc - 1; k++) {
        argv[k] = argv[k + 1];
      }
      argc--;
      i--;
    } else if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--quiet") == 0) {
      log_level = LOG_LEVEL_WARNING;
      /* Remove the argument from argv */
      int k;
      for (k = i; k < argc - 1; k++) {
        argv[k] = argv[k + 1];
      }
      argc--;
      i--;
    }
  }
  
  /* Ensure we have exactly one parameter file specified */
  if(argc != 2)
  {
    FATAL_ERROR("Incorrect usage! Please use: sage [options] <parameterfile>\n"
                "For help, use: sage --help");
  }

  /* Register exit handler for cleanup */
  atexit(bye);

  /* Set up signal handling for CPU time limits */
  sigaction(SIGXCPU, NULL, &saveaction_XCPU);
  current_XCPU = saveaction_XCPU;
  current_XCPU.sa_handler = termination_handler;
  sigaction(SIGXCPU, &current_XCPU, NULL);

  /* Initialize error handling system with the log level determined from command line */
  initialize_error_handling(log_level, NULL);
  
  /* Log startup information */
  DEBUG_LOG("Starting SAGE with verbosity level: %s", get_log_level_name(log_level));
  INFO_LOG("SAGE Semi-Analytic Galaxy Evolution model starting up");
  
  /* Log detailed command line arguments at debug level */
  DEBUG_LOG("Command line argument count: %d", argc);
  int j;
  for (j = 0; j < argc; j++) {
    DEBUG_LOG("Argument %d: %s", j, argv[j]);
  }
  
  /* Read parameter file and initialize simulation */
  read_parameter_file(argv[1]);
  init();
  initialize_sim_state(); /* Initialize simulation state */

  /* Main loop to process merger tree files */
#ifdef MPI
  /* In MPI mode, distribute files across processors using stride of NTask */
  for(filenr = SageConfig.FirstFile+ThisTask; filenr <= SageConfig.LastFile; filenr += NTask)
#else
  /* In serial mode, process all files sequentially */
  for(filenr = SageConfig.FirstFile; filenr <= SageConfig.LastFile; filenr++)
#endif
  {
    /* Construct tree filename and check if it exists */
    snprintf(bufz0, MAX_BUFZ0_SIZE, "%s/%s.%d%s", SageConfig.SimulationDir, SageConfig.TreeName, filenr, SageConfig.TreeExtension);
    if(!(fd = fopen(bufz0, "r")))
    {
      INFO_LOG("Missing tree %s ... skipping", bufz0);
      continue;  // tree file does not exist, move along
    }
    else
      fclose(fd);

    /* Check if output file already exists (to avoid reprocessing) */
    snprintf(bufz0, MAX_BUFZ0_SIZE, "%s/%s_z%1.3f_%d", SageConfig.OutputDir, SageConfig.FileNameGalaxies, ZZ[ListOutputSnaps[0]], filenr);
    if(stat(bufz0, &filestatus) == 0)
    {
      INFO_LOG("Output for tree %s already exists ... skipping", bufz0);
      continue;  // output seems to already exist, dont overwrite, move along
    }

    /* Create output file to mark that we're processing this tree */
    if((fd = fopen(bufz0, "w")))
      fclose(fd);

    /* Load the tree table and process each tree */
    SimState.FileNum = filenr;
    sync_sim_state_to_globals(); /* Update FileNum global */
    load_tree_table(filenr, SageConfig.TreeType);

    for(treenr = 0; treenr < Ntrees; treenr++)
    {
      /* Check if we've received a CPU time limit signal */
      assert(!gotXCPU);

      /* Log progress periodically */
      if(treenr % 10000 == 0)
      {
#ifdef MPI
        INFO_LOG("Processing task: %d node: %s file: %i tree: %i of %i", ThisTask, ThisNode, filenr, treenr, Ntrees);
#else
        INFO_LOG("Processing file: %i tree: %i of %i", filenr, treenr, Ntrees);
#endif
      }

      /* Set the current tree ID and load the tree */
      SimState.TreeID = treenr;
      sync_sim_state_to_globals(); /* Update TreeID global */
      load_tree(filenr, treenr, SageConfig.TreeType);

      /* Set random seed based on file and tree numbers for reproducibility */
      gsl_rng_set(random_generator, filenr * 100000 + treenr);
      
      /* Reset galaxy counters */
      SimState.NumGals = 0;
      SimState.GalaxyCounter = 0;
      sync_sim_state_to_globals(); /* Update galaxy counter globals */
      
      /* Construct galaxies for each unprocessed halo in the tree */
      for(halonr = 0; halonr < TreeNHalos[treenr]; halonr++)
        if(HaloAux[halonr].DoneFlag == 0)
          construct_galaxies(halonr, treenr);

      /* Save the processed galaxies and free memory */
      save_galaxies(filenr, treenr);
      free_galaxies_and_tree();
    }

    /* Finalize output files and free memory */
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

  /* Clean up allocated memory */
  
  /* Special handling for Age array - needs to be reset to original allocation point */
  Age--;
  myfree(Age);

  /* Free the random number generator */
  gsl_rng_free(random_generator);

  /* Set exit status to success */
  exitfail = 0;
  return 0;
}
