#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <limits.h>
#include <inttypes.h>

#include "../core_allvars.h"
#include "../core_mymalloc.h"
#include "../core_utils.h"
#include "tree_binary.h"
#include "../sglib.h"


int fix_mergertree_index(struct halo_data *tree, const int64_t nhalos, const int32_t *index);
int reorder_lhalo_to_lhvt(const int32_t nhalos, struct halo_data *tree, const int32_t test, int32_t **orig_index);

// Local Variables //

static FILE *load_fd = NULL;
// Local Proto-Types //

// External Functions //
void load_forest_table_binary(const int32_t filenr, int *nforests, int **forestnhalos)
{
    char buf[4*MAX_STRING_LEN + 1];
    
	// open the file each time this function is called
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
    // must have an FD
    assert(load_fd );

    /* also assumes that file pointer is at correct location */
    struct halo_data *local_halos = mymalloc(sizeof(struct halo_data) * nhalos);
    myfread(local_halos, nhalos, sizeof(struct halo_data), load_fd);

    /* re-arrange the halos into a locally horizontal vertical forest */
    int status = reorder_lhalo_to_lhvt(nhalos, local_halos, 0, orig_index);/* the 3rd parameter is for testing the reorder code */
    if(status != EXIT_SUCCESS) {
        ABORT(100024323);
    }
    
    *halos = local_halos;
}

int reorder_lhalo_to_lhvt(const int32_t nhalos, struct halo_data *forest, int32_t test, int32_t **orig_index)
{
    int32_t *prog_len=NULL, *desc_len=NULL;
    int32_t *len=NULL, *foflen=NULL;
    if(test > 0) {
        prog_len = calloc(nhalos, sizeof(*prog_len));
        desc_len = calloc(nhalos, sizeof(*prog_len));
        len = calloc(nhalos, sizeof(*len));
        foflen = calloc(nhalos, sizeof(*foflen));

        if(prog_len == NULL || desc_len == NULL || len == NULL || foflen == NULL ) {
            fprintf(stderr,"Warning: malloc failure for LHalotree fields - disabling tests even though tests were requested\n");
            test = 0;
        }
    }


    /* Sort LHalotree into snapshot, FOF group, */
    int32_t *index = calloc(nhalos, sizeof(*index));
    if(index == NULL) {
        perror(NULL);
        fprintf(stderr,"Error: Could not allocate memory for the index array for a forest with nhalos = %d. "
                "Requested size = %zu\n",nhalos, sizeof(*index) * nhalos);
        return EXIT_FAILURE;
    }

    /* Care must be taken for the indices */
    for(int32_t i=0;i<nhalos;i++) {
        index[i] = i;//Keep track of the original indices
        if(test > 0) {
            len[i] = forest[i].Len;
            if(forest[i].FirstHaloInFOFgroup < 0 || forest[i].FirstHaloInFOFgroup >= nhalos){
                fprintf(stderr,"For halonum = %d fofhalo index = %d should be within limits [0, %d)",
                        i, forest[i].FirstHaloInFOFgroup, nhalos);
                return EXIT_FAILURE;
            }
            foflen[i] = forest[forest[i].FirstHaloInFOFgroup].Len;
            if(forest[i].FirstProgenitor == -1 || (forest[i].FirstProgenitor >= 0 && forest[i].FirstProgenitor < nhalos)) {
                prog_len[i] = forest[i].FirstProgenitor == -1 ? -1:forest[forest[i].FirstProgenitor].Len;
            } else {
                fprintf(stderr,"Error. In %s: halonum = %d with FirstProg = %d has invalid value. Should be within [0, %d)\n",
                        __FUNCTION__,i,forest[i].FirstProgenitor, nhalos);
                return EXIT_FAILURE;
            }
            desc_len[i] = forest[i].Descendant == -1 ? -1:forest[forest[i].Descendant].Len;
        }
    }

    /* Sort on snapshots, then sort on FOF groups, then ensure FOF halo comes first within group, then sort by subhalo mass  */
#define SNAPNUM_FOFHALO_MVIR_COMPARATOR(x, i, j)    ((x[i].SnapNum != x[j].SnapNum) ? (x[i].SnapNum - x[j].SnapNum):FOFHALO_COMPARATOR(x, i, j))
#define FOFHALO_COMPARATOR(x, i, j) ((x[i].FirstHaloInFOFgroup != x[j].FirstHaloInFOFgroup) ? (x[i].FirstHaloInFOFgroup - x[j].FirstHaloInFOFgroup):FOFHALO_SUBLEN_COMPARATOR(x,i, j))

#define FOFHALO_SUBLEN_COMPARATOR(x, i, j)     ((x[i].FirstHaloInFOFgroup == index[i]) ? -1:( (x[j].FirstHaloInFOFgroup == index[j]) ? 1: (x[j].Len - x[i].Len)) )

#define MULTIPLE_ARRAY_EXCHANGER(type,a,i,j) {                      \
        SGLIB_ARRAY_ELEMENTS_EXCHANGER(struct halo_data, forest,i,j); \
        SGLIB_ARRAY_ELEMENTS_EXCHANGER(int32_t, index, i, j);       \
    }

    SGLIB_ARRAY_HEAP_SORT_MULTICOMP(struct halo_data, forest, nhalos, SNAPNUM_FOFHALO_MVIR_COMPARATOR, MULTIPLE_ARRAY_EXCHANGER);

#undef SNAPNUM_FOFHALO_MVIR_COMPARATOR
#undef FOFHALO_MVIR_COMPARATOR
#undef FOF_MVIR_COMPARATOR
#undef MULTIPLE_ARRAY_EXCHANGER


    /* But I have to first create another array that tracks the current position of the original index
       The original linear index was like so: [0 1 2 3 4 ....]
       Now, the index might look like [4 1 3 2 0 ...]
       What I need is the location of "0" -> which would 4 (the "0" index from original array is in the
       4'th index within this new array order)

       This requires another array, identical in size to index.

       This array will have to be sorted based on index -> this new sorted array can be directly
       used with the mergertree indices, e.g., FirstProgenitor, to provide correct links.

    */

    /* fixed mergertree indices from sorting into snapshot, FOF group, mvir order */
    int status = fix_mergertree_index(forest, nhalos, index);
    if(status != EXIT_SUCCESS) {
        return status;
    }

    if(test > 0) {
        status = EXIT_FAILURE;
        /* Run tests. First generate the array for the mapping between old and new values */
        int32_t *index_for_old_order = calloc(nhalos, sizeof(*index_for_old_order));
        if(index_for_old_order == NULL) {
            return EXIT_FAILURE;
        }

        for(int32_t i=0;i<nhalos;i++) {
            index_for_old_order[index[i]] = i;
        }

        /* Now run the tests. Progenitor/Descendant masses must agree. */
        for(int32_t i=0;i<nhalos;i++) {
            const int32_t old_index = index[i];
            if(len[old_index] != forest[i].Len) {
                fprintf(stderr,"Error: forest[%d].Len = %d now. Old index claims len = %d\n",
                        i, forest[i].Len, len[old_index]);
                return EXIT_FAILURE;
            }

            if(foflen[old_index] != forest[forest[i].FirstHaloInFOFgroup].Len) {
                fprintf(stderr,"Error: forest[%d].FirstHaloInFOFgroup = %d fofLen = %d now. Old index = %d claims len = %d (nhalos=%d)\n",
                        i, forest[i].FirstHaloInFOFgroup, forest[forest[i].FirstHaloInFOFgroup].Len,
                        old_index,foflen[old_index], nhalos);
                fprintf(stderr,"%d %d %d %d\n",i,forest[i].FirstHaloInFOFgroup,index[i],old_index);
                return EXIT_FAILURE;
            }


            int32_t desc = forest[i].Descendant;
            if(desc == -1) {
                if(desc_len[old_index] != -1){
                    fprintf(stderr,"Error: forest[%d].descendant = %d (should be -1) now but old descendant contained %d particles\n",
                            i, forest[i].Descendant, desc_len[old_index]);
                    return EXIT_FAILURE;
                }
            } else {
                assert(desc >= 0 && desc < nhalos);
                if(desc_len[old_index] != forest[desc].Len) {
                    fprintf(stderr,"Error: forest[%d].Descendant (Len) = %d (desc=%d) now but old descendant contained %d particles\n",
                            i, forest[desc].Len, desc, desc_len[old_index]);
                    return EXIT_FAILURE;
                }
            }


            int32_t prog = forest[i].FirstProgenitor;
            if(prog == -1) {
                if(prog_len[old_index] != -1){
                    fprintf(stderr,"Error: forest[%d].FirstProgenitor = %d (should be -1) now but old FirstProgenitor contained %d particles\n",
                            i, forest[i].FirstProgenitor, desc_len[old_index]);
                    return EXIT_FAILURE;
                }
            } else {
                if( prog < 0 || prog >= nhalos) {
                    fprintf(stderr,"WEIRD: prog = %d for i=%d is not within [0, %d)\n",prog, i, nhalos);
                }
                assert(prog >=0 && prog < nhalos);
                if(prog_len[old_index] != forest[prog].Len) {
                    fprintf(stderr,"Error: forest[%d].FirstProgenitor (Len) = %d (prog=%d) now but old FirstProgenitor contained %d particles\n",
                            i, forest[prog].Len, prog, prog_len[old_index]);
                    return EXIT_FAILURE;
                }
            }
        }

        /* Check that the first halo is a fof */
        if(forest[0].FirstHaloInFOFgroup != 0) {
            fprintf(stderr,"Error: The first halo should be an FOF halo and point to itself but it points to %d\n", forest[0].FirstHaloInFOFgroup);
            return EXIT_FAILURE;
        }


        /* Now check that all halos associated with a FOF come as a bunch (and are never referred to elsewhere via the FirstHaloInFOFgroup*/
        int32_t start_fofindex = 0;
        while(start_fofindex < nhalos) {
            int32_t end_fofindex;
            for(end_fofindex=start_fofindex + 1;end_fofindex < nhalos; end_fofindex++) {
                if(forest[end_fofindex].FirstHaloInFOFgroup == end_fofindex) break;
            }

            /* Now loop over all halos and make sure the only indices that refer to FirstHaloInFOFgroup are within [start_fofindex, end_fofindex )*/
            for(int32_t i=0;i<nhalos;i++) {
                if(forest[i].FirstHaloInFOFgroup == start_fofindex) {
                    if(i >= start_fofindex && i < end_fofindex) {
                        continue;
                    }

                    fprintf(stderr,"Error: Expected FOF to come first and then *all* subhalos associated with that FOF halo\n");
                    fprintf(stderr,"Result truth condition would be for all (FOF+sub) halos to be contained within indices [%d, %d) \n",
                            start_fofindex, end_fofindex);
                    fprintf(stderr,"However, forest[%d].FirstHaloInFOFgroup = %d violates this truth condition\n", i, forest[i].FirstHaloInFOFgroup);
                    return EXIT_FAILURE;
                }
            }
            start_fofindex = end_fofindex;
        }

        free(index_for_old_order);
        free(prog_len); free(desc_len);
        free(len);free(foflen);
    }

    /* because the halos have been re-ordered, the current
       halo-index does not correspond to that in the input forest.
       For any matching purposes, we return the original index for
       each halo
    */
    *orig_index = index;

    return EXIT_SUCCESS;
}



/* This is a more generic function (accepts forests sorted into an arbitary order + original indices as shuffled along with the forest */
int fix_mergertree_index(struct halo_data *forest, const int64_t nhalos, const int32_t *index)
{
    if(nhalos > INT_MAX) {
        fprintf(stderr,"Error: nhalos=%"PRId64" can not be larger than INT_MAX=%d\n", nhalos, INT_MAX);
        return EXIT_FAILURE;
    }

    int32_t *current_index_for_old_order = calloc(nhalos, sizeof(*current_index_for_old_order));
    if(current_index_for_old_order == NULL) {
        return EXIT_FAILURE;
    }


    /* The individual mergertree indices contain references to the old order -> as to where they were and need to
       be updated to where the halo is now. So, we need an array that can tells us, for any index in the old order,
       the location for that halo in the new order.
       
       index[i] contains where the halo was in the old order and I need the opposite information. The following
       lines contain this inverting proces -- only applicable because *ALL* values in index[i] are unique (i.e.,
       this loop can be vectorized with a #pragma simd style). The value on the RHS, i, is the *CURRENT* index
       while the key, on the LHS, is the *OLD* index. Thus, current_index_for_old_order is an array that tells
       us where *ANY* halo index from the *OLD* order can be found in the *NEW* order.
       
       Looks deceptively simple, it isn't. Took 3-days of my time + 2 hours of YQ's to nail this down and have
       validations pass. - MS 19/11/2016
       
    */
    for(int32_t i=0;i<nhalos;i++) {
        current_index_for_old_order[index[i]] = i;
    }


    //the array current_index_for_old_order now contains the current positions for the older index pointers
#define UPDATE_LHALOTREE_INDEX(FIELD) {                                 \
        const int32_t ii = this_halo->FIELD;                            \
        if(ii >=0 && ii < nhalos) {                                     \
            const int32_t dst = current_index_for_old_order[ii];        \
            this_halo->FIELD = dst;                                     \
        }                                                               \
    }
    
    //Now fix *all* the mergertree indices
    for(int64_t i=0;i<nhalos;i++) {
        struct halo_data *this_halo = &(forest[i]);
        UPDATE_LHALOTREE_INDEX(FirstProgenitor);
        UPDATE_LHALOTREE_INDEX(NextProgenitor);
        UPDATE_LHALOTREE_INDEX(Descendant);
        UPDATE_LHALOTREE_INDEX(FirstHaloInFOFgroup);
        UPDATE_LHALOTREE_INDEX(NextHaloInFOFgroup);
    }
#undef UPDATE_LHALOTREE_INDEX
    
    free(current_index_for_old_order);
    return EXIT_SUCCESS;
}


void close_binary_file(void)
{
    if(load_fd != NULL) {
        fclose(load_fd);
        load_fd = NULL;
    }
}

// Local Functions are after here //

