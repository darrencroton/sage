#pragma once

#ifdef __cplusplus
extern "C" {
#endif

    /* functions in core_mymalloc.c */
    extern void *mymalloc(size_t n);
    extern void *mycalloc(const size_t count, const size_t size);
    extern void *myrealloc(void *p, size_t n);
    extern void myfree(void *p);
    extern void print_allocated(void);

#ifdef __cplusplus
}
#endif

    

