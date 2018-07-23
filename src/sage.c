#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include "core_tree_utils.h"

/* main sage -> not exposed externally */
static void sage_per_file(const int ThisTask, const int filenr);
static void sage_per_forest(const int filenr, const int forestnr, const int nhalos, int *TotGalaxies, int **ForestNgals, FILE **save_fd, int *interrupted, struct forest_info *forests_info);

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
static void sage_per_file(const int ThisTask, const int filenr)
{

    struct forest_info forests_info;
    memset(&forests_info, 0, sizeof(struct forest_info));

    forests_info.nforests = 0;
    forests_info.nsnapshots = 0;
    forests_info.totnhalos_per_forest = NULL;
    forests_info.nhalos_per_forest_per_snapshot = NULL;
    snprintf(forests_info.filename, 4*MAX_STRING_LEN, "%s/%s.%d%s", run_params.SimulationDir, run_params.TreeName, filenr, run_params.TreeExtension);

    FILE *fd = fopen(forests_info.filename, "r");
    if (fd == NULL) {
        printf("-- missing tree %s ... skipping\n", forests_info.filename);
        return;
    } else {
        fclose(fd);
    }

    char buffer[4*MAX_STRING_LEN + 1];
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

    int TotGalaxies[ABSOLUTEMAXSNAPS] = { 0 };
    int *ForestNgals[ABSOLUTEMAXSNAPS] = { NULL };
    FILE* save_fd[ABSOLUTEMAXSNAPS] = { NULL };
    
    load_forest_table(run_params.TreeType, &forests_info);
    const int Nforests = forests_info.nforests;
    
    /* allocate memory for the number of galaxies at each output snapshot */
    for(int n = 0; n < run_params.NOUT; n++) {
        /* using calloc removes the need to zero out the memory explicitly*/
        ForestNgals[n] = mycalloc(Nforests, sizeof(int));/* must be calloc*/
    }
    
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
        const int nhalos = forests_info.totnhalos_per_forest[forestnr];
        
        /* the millennium tree is really a collection of trees, viz., a forest */
        sage_per_forest(filenr, forestnr, nhalos, TotGalaxies, ForestNgals, save_fd, &interrupted, &forests_info);
    }
    
    finalize_galaxy_file(Nforests, (const int *) TotGalaxies, (const int **) ForestNgals, save_fd);
    free_forest_table(run_params.TreeType, &forests_info);

    for(int n = 0; n < run_params.NOUT; n++) {
        myfree(ForestNgals[n]);
    }
    myfree(forests_info.totnhalos_per_forest);
    
    if(ThisTask == 0) {
        finish_myprogressbar(stderr, &interrupted);
        fprintf(stdout, "\ndone file %d\n\n", filenr);
    }
}


static void sage_per_forest(const int filenr, const int forestnr, const int nhalos, int *TotGalaxies, int **ForestNgals, FILE **save_fd,
                            int *interrupted, struct forest_info *forests_info)
{
    (void) interrupted;
    
    /*  galaxy data  */
    struct GALAXY  *Gal = NULL, *HaloGal = NULL;
    
    /* simulation merger-tree data */
    struct halo_data *Halo = NULL;
    
    /*  auxiliary halo data  */
    struct halo_aux_data  *HaloAux = NULL;
    
    int nfofs_all_snaps[ABSOLUTEMAXSNAPS] = {0};
    load_forest(forestnr, nhalos, run_params.TreeType, &Halo, forests_info);

    /* re-arrange the halos into a locally horizontal vertical forest */
    int32_t *file_ordering_of_halos=NULL;
    int status = reorder_lhalo_to_lhvt(nhalos, Halo, 0, &file_ordering_of_halos);/* the 3rd parameter is for testing the reorder code */
    if(status != EXIT_SUCCESS) {
        ABORT(status);
    }
    
    int maxgals = (int)(MAXGALFAC * nhalos);
    if(maxgals < 10000) maxgals = 10000;

    HaloAux = mycalloc(nhalos, sizeof(HaloAux[0]));
    HaloGal = mycalloc(maxgals, sizeof(HaloGal[0]));
    Gal = mycalloc(maxgals, sizeof(Gal[0]));/* used to be fof_maxgals instead of maxgals*/

    for(int i = 0; i < nhalos; i++) {
        HaloAux[i].orig_index = file_ordering_of_halos[i];
    }
    free(file_ordering_of_halos);
    /* done with re-ordering the halos into a locally horizontal vertical tree format */
    
    
    /* getting the number of FOF halos at each snapshot */
    get_nfofs_all_snaps(Halo, nhalos, nfofs_all_snaps, ABSOLUTEMAXSNAPS);
    
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

    /* free galaxies and the forest */
    myfree(Gal);
    myfree(HaloGal);
    myfree(HaloAux);
    myfree(Halo);
}    


