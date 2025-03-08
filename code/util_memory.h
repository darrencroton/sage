#ifndef UTIL_MEMORY_H
#define UTIL_MEMORY_H

#include <stddef.h>

/* Memory allocation utilities */
void *mymalloc(size_t size);
void *myrealloc(void *ptr, size_t size);
void myfree(void *ptr);
void print_allocated(void);

#endif /* UTIL_MEMORY_H */
