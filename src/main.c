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
#include "sage.h"

char bufz0[MAX_STRING_LEN + 1];
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

    if(exitfail) {
        unlink(bufz0);
        
#ifdef MPI
        if(ThisTask == 0 && gotXCPU == 1) {
            printf("Received XCPU, exiting. But we'll be back.\n");
        }
#endif
    }
}



int main(int argc, char **argv)
{
    struct sigaction current_XCPU;

#ifdef MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &ThisTask);
    MPI_Comm_size(MPI_COMM_WORLD, &NTask);

    ThisNode = malloc(MPI_MAX_PROCESSOR_NAME * sizeof(char));

    MPI_Get_processor_name(ThisNode, &nodeNameLen);
    if (nodeNameLen >= MPI_MAX_PROCESSOR_NAME) {
        printf("Node name string not long enough!...\n");
        ABORT(0);
    }
#endif

    if(argc != 2) {
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

#if 0    
#ifdef HDF5
    if(HDF5Output) {
        calc_hdf5_props();
    }
#endif
#endif


#ifdef MPI
    for(int filenr = FirstFile+ThisTask; filenr <= LastFile; filenr += NTask) {
#else
    for(int filenr = FirstFile; filenr <= LastFile; filenr++) {
#endif
        /* run the sage model */
        sage(filenr);
    }
    
#if 0
    if(HDF5Output) {
        free_hdf5_ids();
      
#ifdef MPI
        // Create a single master HDF5 file with links to the other files...
        MPI_Barrier(MPI_COMM_WORLD);
        if (ThisTask == 0)
#endif
            write_master_file();
    }
#endif /* commented out the section for hdf5 output */    


    //free Ages. But first
    //reset Age to the actual allocated address
    Age--;
    myfree(Age);                              
    
    gsl_rng_free(random_generator); 
    
    exitfail = 0;
    return EXIT_SUCCESS;
}


