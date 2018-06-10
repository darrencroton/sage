#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef MPI
#include <mpi.h>
#endif

#include "core_allvars.h"
#include "core_proto.h"
#include "sage.h"


void myexit(int signum)
{
#ifdef MPI
    printf("Task: %d out of %d tasks \tis exiting\n\n\n", ThisTask, NTasks);
#else
    printf("We're exiting\n\n\n");
#endif
    exit(signum);
}



void bye()
{
#ifdef MPI
    MPI_Finalize();
#endif

}



int main(int argc, char **argv)
{
#ifdef MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &ThisTask);
    MPI_Comm_size(MPI_COMM_WORLD, &NTasks);
#endif

    if(argc != 2) {
        printf("\n  usage: sage <parameterfile>\n\n");
        ABORT(0);
    }

    atexit(bye);

    read_parameter_file(argv[1]);
    init();

#ifdef MPI
    for(int filenr = run_params.FirstFile+ThisTask; filenr <= run_params.LastFile; filenr += NTasks) {
#else
    for(int filenr = run_params.FirstFile; filenr <= run_params.LastFile; filenr++) {
#endif
        /* run the sage model */
        sage(filenr);
    }
    
    /* run the final steps and any cleanup */
    finalize_sage();
    
    return EXIT_SUCCESS;
}


