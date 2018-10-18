#include <stdio.h>
#include <stdlib.h>

#ifdef MPI
#include <mpi.h>
#endif

#include "macros.h"
#include "sage.h"


int main(int argc, char **argv)
{
    int ThisTask = 0;
    int NTasks = 1;

#ifdef MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &ThisTask);
    MPI_Comm_size(MPI_COMM_WORLD, &NTasks);
#endif

    if(argc != 2) {
        printf("\n  usage: sage <parameterfile>\n\n");
        ABORT(0);
    }

    /* initialize sage (read parameter file, setup units, read cooling tables etc) */
    init_sage(ThisTask, argv[1]);

    /* run sage over all files */
    sage(ThisTask, NTasks);
    
    /* run the final steps and any cleanup */
    finalize_sage();

#ifdef MPI
    MPI_Finalize();
#endif
    
    return EXIT_SUCCESS;
}


