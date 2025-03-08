#ifndef IO_SAVE_BINARY_H
#define IO_SAVE_BINARY_H

/* Functions for binary file saving */
void save_galaxies(int filenr, int tree);
void finalize_galaxy_file(int filenr);
void close_galaxy_files(void);

#endif /* IO_SAVE_BINARY_H */
