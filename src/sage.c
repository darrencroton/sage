#include <stdio.h>
#include <sys/stat.h>

#include "sage.h"
#include "core_proto.h"
#include "progressbar.h"

void init_sage(const char *param_file)
{
    read_parameter_file(param_file);
    init();
}


void sage(const int filenr)
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
    load_tree_table(filenr, run_params.TreeType, &Ntrees, &TreeNHalos, &TreeFirstHalo, (int **) TreeNgals, (int *) TotGalaxies);
        
    int interrupted=0;
    init_my_progressbar(stderr, Ntrees, &interrupted);
    
    for(int treenr = 0; treenr < Ntrees; treenr++) {

        /*  galaxy data  */
        struct GALAXY  *Gal = NULL, *HaloGal = NULL;
            
        /* simulation merger-tree data */
        struct halo_data *Halo = NULL;

        /*  auxiliary halo data  */
        struct halo_aux_data  *HaloAux = NULL;
            
#ifdef MPI
        if(ThisTask == 0)
#endif            
            my_progressbar(stderr, treenr, &interrupted);
        
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

        save_galaxies(filenr, treenr, Ntrees, numgals, Halo, HaloAux, HaloGal, (int **) TreeNgals, (int *) TotGalaxies);
        free_galaxies_and_tree(Gal, HaloGal, HaloAux, Halo);
    }
    
    finalize_galaxy_file(Ntrees, (const int *) TotGalaxies, (const int **) TreeNgals);
    free_tree_table(run_params.TreeType, (int **) TreeNgals, TreeNHalos, TreeFirstHalo);
        
    finish_myprogressbar(stderr, &interrupted);
    
#ifdef MPI
    if(ThisTask == 0)
#endif            
        fprintf(stdout, "\ndone file %d\n\n", filenr);
    interrupted=1;
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
