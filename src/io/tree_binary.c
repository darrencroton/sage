#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>

#include "../core_allvars.h"
#include "../core_proto.h"
#include "tree_binary.h"

// Local Variables //

static FILE *load_fd = NULL;
// Local Proto-Types //

// External Functions //
void load_tree_table_binary(const int32_t filenr, int *ntrees, int **treenhalos, int **treefirsthalo)
{
    char buf[MAX_STRING_LEN + 1];

	// open the file each time this function is called
    snprintf(buf, MAX_STRING_LEN, "%s/%s.%d%s", SimulationDir, TreeName, filenr, TreeExtension);
    load_fd = fopen(buf, "r");
    if(load_fd == NULL) {
        printf("can't open file `%s'\n", buf);
        ABORT(0);
    }
    int local_ntrees=0;
    int local_totnhalos=0;
    
    myfread(&local_ntrees, 1, sizeof(int), load_fd);
    myfread(&local_totnhalos, 1, sizeof(int), load_fd);

    int *local_treenhalos = mymalloc(sizeof(int) * local_ntrees);
    int *local_treefirsthalo = mymalloc(sizeof(int) * local_ntrees);

    myfread(local_treenhalos, local_ntrees, sizeof(int), load_fd);

    if(local_ntrees) {
        local_treefirsthalo[0] = 0;
    }
    for(int i = 1; i < local_ntrees; i++) {
        local_treefirsthalo[i] = local_treefirsthalo[i - 1] + local_treenhalos[i - 1];
    }

    *ntrees = local_ntrees;
    *treenhalos = local_treenhalos;
    *treefirsthalo = local_treefirsthalo;
}

void load_tree_binary(const int32_t nhalos, struct halo_data **halos)
{
    // must have an FD
    assert(load_fd );

    /* also assumes that file pointer is at correct location */
    struct halo_data *local_halos = mymalloc(sizeof(struct halo_data) * nhalos);
    myfread(local_halos, nhalos, sizeof(struct halo_data), load_fd);

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

