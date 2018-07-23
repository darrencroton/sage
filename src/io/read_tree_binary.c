#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "read_tree_binary.h"
#include "../core_mymalloc.h"
#include "../core_utils.h"
#include "../core_tree_utils.h"


/* Externally visible Functions */
void load_forest_table_binary(struct forest_info *forests_info)
{
    const char *filename = forests_info->filename;
    forests_info->fp = fopen(filename, "r");
    if(forests_info->fp == NULL) {
        printf("Error: can't open file `%s'\n", filename);
        ABORT(FILE_NOT_FOUND);
    }
    int local_nforests=0;
    int local_totnhalos=0;
    
    myfread(&local_nforests, 1, sizeof(int), forests_info->fp);
    myfread(&local_totnhalos, 1, sizeof(int), forests_info->fp);

    int *local_forestnhalos = mymalloc(sizeof(int) * local_nforests);

    myfread(local_forestnhalos, local_nforests, sizeof(int), forests_info->fp);

    forests_info->nforests = local_nforests;
    forests_info->totnhalos_per_forest = local_forestnhalos;/* this will work because I am modifying a pointer contained within a struct */
}

void load_forest_binary(const int32_t nhalos, struct halo_data **halos, struct forest_info *forests_info)
{
    /* must have an FD */
    assert(forests_info->fp );

    /* also assumes that file pointer is at correct location */
    struct halo_data *local_halos = mymalloc(sizeof(struct halo_data) * nhalos);
    myfread(local_halos, nhalos, sizeof(struct halo_data), forests_info->fp);

    *halos = local_halos;
}

void close_binary_file(struct forest_info *forests_info)
{
    if(forests_info->fp != NULL) {
        fclose(forests_info->fp);
        forests_info->fp = NULL;
    }
}



