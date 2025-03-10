#ifndef UTIL_MEMORY_H
#define UTIL_MEMORY_H

#include <stddef.h>

/* Configuration */
#ifndef DEFAULT_MAX_MEMORY_BLOCKS
#define DEFAULT_MAX_MEMORY_BLOCKS 1024  /* Increased from 256 */
#endif

/* Memory allocation utilities */
void init_memory_system(unsigned long max_blocks);
void *mymalloc(size_t size);
void *myrealloc(void *ptr, size_t size);
void myfree(void *ptr);
void print_allocated(void);
void check_memory_leaks(void);
void cleanup_memory_system(void);

#endif /* UTIL_MEMORY_H */
