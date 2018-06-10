#pragma once

#ifdef __cplusplus
extern "C" {
#endif

    /* functions in core_mymalloc.c */
    void *mymalloc(size_t n);
    void *myrealloc(void *p, size_t n);
    void myfree(void *p);
    void print_allocated(void);

#ifdef __cplusplus
}
#endif

    

