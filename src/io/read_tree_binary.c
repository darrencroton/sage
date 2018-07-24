#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

#include "read_tree_binary.h"
#include "../core_mymalloc.h"
#include "../core_utils.h"
#include "../core_tree_utils.h"


/* Externally visible Functions */
void load_forest_table_binary(struct forest_info *forests_info)
{
    const char *filename = forests_info->filename;

#ifdef USE_FWRITE    
    forests_info->fp = fopen(filename, "r");
    if(forests_info->fp == NULL) {
        printf("Error: can't open file `%s'\n", filename);
        ABORT(FILE_NOT_FOUND);
    }
#else
    forests_info->fd = open(filename, O_RDONLY);
    if(forests_info->fd < 0) {
        printf("Error: can't open file `%s'\n", filename);
        perror(NULL);
        ABORT(FILE_NOT_FOUND);
    }
#endif
    
    int nforests=0;
    int totnhalos=0;

#ifdef USE_FWRITE    
    myfread(&nforests, 1, sizeof(int), forests_info->fp);
#else
    mypread(forests_info->fd, &nforests, sizeof(int), 0);
#endif


    int *forestnhalos = mymalloc(sizeof(int) * nforests);
    
#ifdef USE_FWRITE
    myfread(&totnhalos, 1, sizeof(int), forests_info->fp);
    myfread(forestnhalos, nforests, sizeof(int), forests_info->fp);
#else
    mypread(forests_info->fd, &totnhalos, sizeof(int), 4);
    mypread(forests_info->fd, forestnhalos, nforests*sizeof(int), 8);
#endif

    forests_info->nforests = nforests;
    forests_info->totnhalos_per_forest = forestnhalos;/* this will work because I am modifying a pointer contained within a struct */
    /*
      number of bytes offset to get to the first tree.
      int32_t nforests, int32_t totnhalos, (int32_t \times nforests) forestnhalos
       
     */
    off_t size_so_far = 4 + 4 + sizeof(int32_t) * nforests;
    forests_info->bytes_offset_for_forest = mycalloc(nforests, sizeof(forests_info->bytes_offset_for_forest[0]));
    for(int32_t i=0;i<nforests;i++) {
        forests_info->bytes_offset_for_forest[i] = size_so_far;
        size_so_far += forestnhalos[i] * sizeof(struct halo_data);
    }
}

void load_forest_binary(const int32_t forestnr, const int32_t nhalos, struct halo_data **halos, struct forest_info *forests_info)
{
    /* must have an FD */
    assert(forests_info->fp );

    /* also assumes that file pointer is at correct location */
    struct halo_data *local_halos = mymalloc(sizeof(struct halo_data) * nhalos);

#ifdef USE_FWRITE
    (void) forestnr;
    myfread(local_halos, nhalos, sizeof(struct halo_data), forests_info->fp);
#else
    const off_t offset = forests_info->bytes_offset_for_forest[forestnr];
    mypread(forests_info->fd, local_halos, sizeof(struct halo_data) * nhalos, offset);
#endif

    *halos = local_halos;
}

void close_binary_file(struct forest_info *forests_info)
{
#ifdef USE_FWRITE
    if(forests_info->fp != NULL) {
        fclose(forests_info->fp);
        forests_info->fp = NULL;
    }
#else
    if(forests_info->fd > 0) {
        close(forests_info->fd);
        forests_info->fd = -1;
    }

    if(forests_info->bytes_offset_for_forest) {
        free(forests_info->bytes_offset_for_forest);
    }
#endif
}



