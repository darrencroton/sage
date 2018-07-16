#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "read_tree_binary.h"
#include "../core_mymalloc.h"
#include "../core_utils.h"
#include "../core_tree_utils.h"


/* Local Variables */
static FILE *load_fd = NULL;

/* Externally visible Functions */
void load_forest_table_binary(const int32_t filenr, int *nforests, int **forestnhalos)
{
    char buf[4*MAX_STRING_LEN + 1];
    
	/* open the file each time this function is called */
    snprintf(buf, 4*MAX_STRING_LEN, "%s/%s.%d%s", run_params.SimulationDir, run_params.TreeName, filenr, run_params.TreeExtension);
    load_fd = fopen(buf, "r");
    if(load_fd == NULL) {
        printf("can't open file `%s'\n", buf);
        ABORT(0);
    }
    int local_nforests=0;
    int local_totnhalos=0;
    
    myfread(&local_nforests, 1, sizeof(int), load_fd);
    myfread(&local_totnhalos, 1, sizeof(int), load_fd);

    int *local_forestnhalos = mymalloc(sizeof(int) * local_nforests);

    myfread(local_forestnhalos, local_nforests, sizeof(int), load_fd);

    *nforests = local_nforests;
    *forestnhalos = local_forestnhalos;
}

void load_forest_binary(const int32_t nhalos, struct halo_data **halos, int32_t **orig_index)
{
    /* must have an FD */
    assert(load_fd );

    /* also assumes that file pointer is at correct location */
    struct halo_data *local_halos = mymalloc(sizeof(struct halo_data) * nhalos);
    myfread(local_halos, nhalos, sizeof(struct halo_data), load_fd);

    /* re-arrange the halos into a locally horizontal vertical forest */
    int status = reorder_lhalo_to_lhvt(nhalos, local_halos, 0, orig_index);/* the 3rd parameter is for testing the reorder code */
    if(status != EXIT_SUCCESS) {
        ABORT(status);
    }
    
    *halos = local_halos;
}

void close_binary_file(void)
{
    if(load_fd != NULL) {
        fclose(load_fd);
        load_fd = NULL;
    }
}

// Local Functions are after here //

