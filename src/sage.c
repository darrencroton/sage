#include <stdio.h>
#include <sys/stat.h>

#include "sage.h"
#include "core_proto.h"
#include "progressbar.h"

void sage(const int filenr)
{
    char buffer[MAX_STRING_LEN + 1];
    snprintf(buffer, MAX_STRING_LEN, "%s/%s.%d%s", SimulationDir, TreeName, filenr, TreeExtension);
    FILE *fd = fopen(buffer, "r");
    if (fd == NULL) {
        printf("-- missing tree %s ... skipping\n", buffer);
        return;
    } else {
        fclose(fd);
    }
    
    snprintf(buffer, MAX_STRING_LEN, "%s/%s_z%1.3f_%d", OutputDir, FileNameGalaxies, ZZ[ListOutputSnaps[0]], filenr);
    struct stat filestatus;

    if(stat(buffer, &filestatus) == 0) {
        printf("-- output for tree %s already exists ... skipping\n", buffer);
        return;  // output seems to already exist, dont overwrite, move along
    }
    
    fd = fopen(buffer, "w");
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
