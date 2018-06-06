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
#include "progressbar.h"

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
    struct stat filestatus;

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
        snprintf(bufz0, MAX_STRING_LEN, "%s/%s.%d%s", SimulationDir, TreeName, filenr, TreeExtension);
        FILE *fd = fopen(bufz0, "r");
        if (fd == NULL) {
            printf("-- missing tree %s ... skipping\n", bufz0);
            continue;  // tree file does not exist, move along
        } else {
            fclose(fd);
        }
        
        snprintf(bufz0, MAX_STRING_LEN, "%s/%s_z%1.3f_%d", OutputDir, FileNameGalaxies, ZZ[ListOutputSnaps[0]], filenr);
        if(stat(bufz0, &filestatus) == 0) {
            printf("-- output for tree %s already exists ... skipping\n", bufz0);
            continue;  // output seems to already exist, dont overwrite, move along
        }

        fd = fopen(bufz0, "w");
        if(fd != NULL) {
            fclose(fd);
        }

#ifdef OLD_VERSION
        load_tree_table(filenr, TreeType);
#else
        
        int Ntrees = 0;
        int *TreeNHalos = NULL;
        int *TreeFirstHalo = NULL;
        int TotGalaxies[ABSOLUTEMAXSNAPS];
        int *TreeNgals[ABSOLUTEMAXSNAPS];
        load_tree_table(filenr, TreeType, &Ntrees, &TreeNHalos, &TreeFirstHalo, (int **) TreeNgals, (int *) TotGalaxies);
#endif        
        
        int interrupted=0;
        init_my_progressbar(stderr, Ntrees, &interrupted);
        for(int treenr = 0; treenr < Ntrees; treenr++) {
            assert(!gotXCPU);
#ifndef OLD_VERSION
            /*  galaxy data  */
            struct GALAXY  *Gal = NULL, *HaloGal = NULL;
            
            /* simulation merger-tree data */
            struct halo_data *Halo = NULL;

            /*  auxiliary halo data  */
            struct halo_aux_data  *HaloAux = NULL;
#endif            
            
            
#ifdef MPI
            if(ThisTask == 0)
#endif            
                my_progressbar(stderr, treenr, &interrupted);
        
            const int nhalos = TreeNHalos[treenr];
#ifdef OLD_VERSION
            int maxgals = load_tree(treenr, nhalos, TreeType);
#else            
            int maxgals = load_tree(treenr, nhalos, TreeType, &Halo, &HaloAux, &Gal, &HaloGal);
#endif            
            
            gsl_rng_set(random_generator, filenr * 100000 + treenr);
            int numgals = 0;
            int galaxycounter = 0;
            for(int halonr = 0; halonr < nhalos; halonr++) {
                if(HaloAux[halonr].DoneFlag == 0) {
#ifdef OLD_VERSION        
                    construct_galaxies(halonr, treenr, &numgals, &galaxycounter, &maxgals);
#else
                    construct_galaxies(halonr, &numgals, &galaxycounter, &maxgals, Halo, HaloAux, &Gal, &HaloGal);
#endif
                }
            }

#ifdef OLD_VERSION        
            save_galaxies(filenr, treenr);
            free_galaxies_and_tree();
#else
            save_galaxies(filenr, treenr, Ntrees, numgals, Halo, HaloAux, HaloGal, (int **) TreeNgals, (int *) TotGalaxies);
            free_galaxies_and_tree(Gal, HaloGal, HaloAux, Halo);
#endif        
        }
    
#ifdef OLD_VERSION
        finalize_galaxy_file();
        free_tree_table(TreeType);
#else        

        finalize_galaxy_file(Ntrees, (const int *) TotGalaxies, (const int **) TreeNgals);
        free_tree_table(TreeType, (int **) TreeNgals, TreeNHalos, TreeFirstHalo);
#endif        
        
        finish_myprogressbar(stderr, &interrupted);
    
#ifdef MPI
        if(ThisTask == 0)
#endif            
            fprintf(stdout, "\ndone file %d\n\n", filenr);
        interrupted=1;
    }
    /* printf("Output 0 had %d galaxies\n", TotGalaxies[0]); */
    
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


