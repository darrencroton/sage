#ifndef UTIL_MEMORY_H
#define UTIL_MEMORY_H

#include <stddef.h>

/* Memory categories for component-level tracking */
typedef enum {
  MEM_UNKNOWN = 0,
  MEM_GALAXIES,
  MEM_HALOS,
  MEM_TREES,
  MEM_IO,
  MEM_UTILITY,
  MEM_MAX_CATEGORY /* For bounds checking */
} MemoryCategory;

/* Memory reporting levels */
#define MEMORY_REPORT_NONE 0
#define MEMORY_REPORT_MINIMAL 1
#define MEMORY_REPORT_DETAILED 2

/* Configuration */
#ifndef DEFAULT_MAX_MEMORY_BLOCKS
#define DEFAULT_MAX_MEMORY_BLOCKS 1024 /* Increased from 256 */
#endif

/* Memory allocation utilities */
void init_memory_system(unsigned long max_blocks);
void *mymalloc(size_t size);
void *mymalloc_cat(size_t size, MemoryCategory category);
void *myrealloc(void *ptr, size_t size);
void *myrealloc_cat(void *ptr, size_t size, MemoryCategory category);
void myfree(void *ptr);

/* Memory reporting and debugging */
void set_memory_reporting(int level);
void print_allocated(void);
void print_allocated_by_category(void);
void print_memory_brief(void);
void check_memory_leaks(void);
int validate_memory_block(void *ptr);
int validate_all_memory(void);
void cleanup_memory_system(void);

#endif /* UTIL_MEMORY_H */
