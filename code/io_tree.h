#ifndef IO_TREE_H
#define IO_TREE_H

/* Functions for tree I/O operations */
void load_tree(int filenr, int treenr, enum Valid_TreeTypes TreeType);
void load_tree_table(int filenr, enum Valid_TreeTypes my_TreeType);
void free_tree_table(enum Valid_TreeTypes my_TreeType);
void free_galaxies_and_tree(void);

/* I/O wrapper functions with endianness handling and buffering */
void set_file_endianness(int endianness);
int get_file_endianness(void);
void init_io_buffering(void);
void cleanup_io_buffering(void);
size_t myfread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t myfwrite(void *ptr, size_t size, size_t nmemb, FILE *stream);
int myfseek(FILE *stream, long offset, int whence);

#endif /* IO_TREE_H */
