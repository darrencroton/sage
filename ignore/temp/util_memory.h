#ifndef UTIL_MEMORY_H
#define UTIL_MEMORY_H

#include <stdlib.h>

// Allocation functions
void *mymalloc(size_t size);
int myfree(void *p);
void *mymalloc_fullinfo(size_t size, const char *func, const char *file, int line, const char *msg);
int myfree_fullinfo(void *p, const char *func, const char *file, int line);
void *myrealloc_fullinfo(void *p, size_t size, const char *func, const char *file, int line, const char *msg);
void report_memory_usage(void);

// Memory management macros with debugging info
#ifdef DEBUG_MEMORY_USE
#define mymalloc(size) mymalloc_fullinfo(size, __FUNCTION__, __FILE__, __LINE__, #size)
#define myfree(p) myfree_fullinfo(p, __FUNCTION__, __FILE__, __LINE__)
#define myrealloc(p, size) myrealloc_fullinfo(p, size, __FUNCTION__, __FILE__, __LINE__, #size)
#endif

#endif /* UTIL_MEMORY_H */
