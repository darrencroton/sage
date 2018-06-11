#include <stdio.h>
#include <sys/stat.h>

#include "sage.h"
#include "core_allvars.h"
#include "core_init.h"
#include "core_read_parameter_file.h"
#include "core_io_tree.h"
#include "core_mymalloc.h"
#include "core_build_model.h"
#include "core_save.h"
#include "progressbar.h"


/* main sage -> not exposed externally */
static void sage_per_file(const int ThisTask, const int filenr);

void init_sage(const int ThisTask, const char *param_file)
{
    read_parameter_file(ThisTask, param_file);
    
    init(ThisTask);
}

void sage(const int ThisTask, const int NTasks)
{

#ifdef MPI
    for(int filenr = run_params.FirstFile+ThisTask; filenr <= run_params.LastFile; filenr += NTasks) {
#else
    (void) NTasks;
    for(int filenr = run_params.FirstFile; filenr <= run_params.LastFile; filenr++) {
#endif
        /* run the sage model on all trees within the file*/
        sage_per_file(ThisTask, filenr);
    }

}


void finalize_sage(void)
{

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
    run_params.Age--;
    myfree(run_params.Age);                              
    
    gsl_rng_free(random_generator); 
}



/* Main sage -> not exposed externally */ 
void sage_per_file(const int ThisTask, const int filenr)
{
    char buffer[4*MAX_STRING_LEN + 1];
    snprintf(buffer, 4*MAX_STRING_LEN, "%s/%s.%d%s", run_params.SimulationDir, run_params.TreeName, filenr, run_params.TreeExtension);
    FILE *fd = fopen(buffer, "r");
    if (fd == NULL) {
        printf("-- missing tree %s ... skipping\n", buffer);
        return;
    } else {
        fclose(fd);
    }
    
    snprintf(buffer, 4*MAX_STRING_LEN, "%s/%s_z%1.3f_%d", run_params.OutputDir, run_params.FileNameGalaxies, run_params.ZZ[run_params.ListOutputSnaps[0]], filenr);
    struct stat filestatus;

    if(stat(buffer, &filestatus) == 0) {
        printf("-- output for tree %s already exists ... skipping\n", buffer);
        return;  // output seems to already exist, dont overwrite, move along
    }
    
    fd = fopen(buffer, "w");
    if(fd != NULL) {
        fclose(fd);
    }

    int Ntrees = 0;
    int *TreeNHalos = NULL;
    int *TreeFirstHalo = NULL;
    int TotGalaxies[ABSOLUTEMAXSNAPS];
    int *TreeNgals[ABSOLUTEMAXSNAPS];
    FILE* save_fd[ABSOLUTEMAXSNAPS] = { 0 };
    
    load_tree_table(ThisTask, filenr, run_params.TreeType, &Ntrees, &TreeNHalos, &TreeFirstHalo, (int **) TreeNgals, (int *) TotGalaxies);

    /* open all the output files corresponding to this tree file (specified by filenr) */
    initialize_galaxy_files(filenr, Ntrees, save_fd);
    
    int interrupted=0;
    if(ThisTask == 0) {
        init_my_progressbar(stderr, Ntrees, &interrupted);
    }
    
    for(int treenr = 0; treenr < Ntrees; treenr++) {

        /*  galaxy data  */
        struct GALAXY  *Gal = NULL, *HaloGal = NULL;
            
        /* simulation merger-tree data */
        struct halo_data *Halo = NULL;

        /*  auxiliary halo data  */
        struct halo_aux_data  *HaloAux = NULL;
            
        if(ThisTask == 0) {
            my_progressbar(stderr, treenr, &interrupted);
        }
        
        const int nhalos = TreeNHalos[treenr];
        int maxgals = load_tree(treenr, nhalos, run_params.TreeType, &Halo, &HaloAux, &Gal, &HaloGal);
        gsl_rng_set(random_generator, filenr * 100000 + treenr);
        int numgals = 0;
        int galaxycounter = 0;
        for(int halonr = 0; halonr < nhalos; halonr++) {
            if(HaloAux[halonr].DoneFlag == 0) {
                construct_galaxies(halonr, &numgals, &galaxycounter, &maxgals, Halo, HaloAux, &Gal, &HaloGal);
            }
        }

        save_galaxies(filenr, treenr, numgals, Halo, HaloAux, HaloGal, (int **) TreeNgals, (int *) TotGalaxies, save_fd);
        free_galaxies_and_tree(Gal, HaloGal, HaloAux, Halo);
    }
    
    finalize_galaxy_file(Ntrees, (const int *) TotGalaxies, (const int **) TreeNgals, save_fd);
    free_tree_table(run_params.TreeType, (int **) TreeNgals, TreeNHalos, TreeFirstHalo);

    if(ThisTask == 0) {
        finish_myprogressbar(stderr, &interrupted);
        fprintf(stdout, "\ndone file %d\n\n", filenr);
    }
}


 
