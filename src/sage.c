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
static void sage_per_forest(const int filenr, const int treenr, int *TreeNHalos, int *TotGalaxies, int **TreeNgals, FILE **save_fd, int *interrupted);

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

    int Nforests = 0;
    int *ForestNHalos = NULL;
    int TotGalaxies[ABSOLUTEMAXSNAPS];
    int *ForestNgals[ABSOLUTEMAXSNAPS];
    FILE* save_fd[ABSOLUTEMAXSNAPS] = { 0 };
    
    load_forest_table(ThisTask, filenr, run_params.TreeType, &Nforests, &ForestNHalos, (int **) ForestNgals, (int *) TotGalaxies);

    /* open all the output files corresponding to this tree file (specified by filenr) */
    initialize_galaxy_files(filenr, Nforests, save_fd);
    
    int interrupted=0;
    if(ThisTask == 0) {
        init_my_progressbar(stderr, Nforests, &interrupted);
    }
    
    
    for(int forestnr = 0; forestnr < Nforests; forestnr++) {
        if(ThisTask == 0) {
            my_progressbar(stderr, forestnr, &interrupted);
        }

        /* the millennium tree is really a collection of trees, viz., a forest */
        sage_per_forest(filenr, forestnr, ForestNHalos, TotGalaxies, ForestNgals, save_fd, &interrupted);
    }
    
    finalize_galaxy_file(Nforests, (const int *) TotGalaxies, (const int **) ForestNgals, save_fd);
    free_forest_table(run_params.TreeType, (int **) ForestNgals, ForestNHalos);

    if(ThisTask == 0) {
        finish_myprogressbar(stderr, &interrupted);
        fprintf(stdout, "\ndone file %d\n\n", filenr);
    }
}


void sage_per_forest(const int filenr, const int forestnr, int *ForestNHalos, int *TotGalaxies, int **ForestNgals, FILE **save_fd, int *interrupted)
{
    (void) interrupted;
    
    /*  galaxy data  */
    struct GALAXY  *Gal = NULL, *HaloGal = NULL;
    
    /* simulation merger-tree data */
    struct halo_data *Halo = NULL;
    
    /*  auxiliary halo data  */
    struct halo_aux_data  *HaloAux = NULL;
    int nfofs_all_snaps[ABSOLUTEMAXSNAPS] = {0};
    const int nhalos = ForestNHalos[forestnr];
    int maxgals = load_forest(forestnr, nhalos, run_params.TreeType, &Halo, &HaloAux, &Gal, &HaloGal);
    
#if 0        
    for(int halonr = 0; halonr < nhalos; halonr++) {
        fprintf(stderr,"halonr = %d snap = %03d mvir = %14.6e firstfofhalo = %8d nexthalo = %8d\n",
                halonr, Halo[halonr].SnapNum, Halo[halonr].Mvir, Halo[halonr].FirstHaloInFOFgroup, Halo[halonr].NextHaloInFOFgroup);
    }
#endif
    
    int numgals = 0;
    int galaxycounter = 0;
    

#ifdef PROCESS_LHVT_STYLE
    /* this will be the new processing style --> one snapshot at a time */
    uint32_t ngal = 0;
    for(int snapshot=min_snapshot;snapshot <= max_snapshot; snapshot++) {
        uint32_t nfofs_this_snap = get_nfofs_at_snap(forestnr, snapshot);
        for(int ifof=0;ifof<nfofs_this_snap;ifof++) {
            ngal = process_fof_at_snap(ifof, snapshot, ngal);
        }
    }

#else
    
    /* First run construct_galaxies outside for loop -> takes care of the main tree */
    construct_galaxies(0, &numgals, &galaxycounter, &maxgals, Halo, HaloAux, &Gal, &HaloGal);
    
    /* But there are sub-trees within one forest file that are not reachable via the recursive routine -> do those as well */
    for(int halonr = 0; halonr < nhalos; halonr++) {
        if(HaloAux[halonr].DoneFlag == 0) {
            construct_galaxies(halonr, &numgals, &galaxycounter, &maxgals, Halo, HaloAux, &Gal, &HaloGal);
        }
    }

#endif /* PROCESS_LHVT_STYLE */    

    
    save_galaxies(filenr, forestnr, numgals, Halo, HaloAux, HaloGal, (int **) ForestNgals, (int *) TotGalaxies, save_fd);
    free_galaxies_and_forest(Gal, HaloGal, HaloAux, Halo);
}    
