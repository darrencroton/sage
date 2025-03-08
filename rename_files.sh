#!/bin/zsh
# Script to rename SAGE files according to new convention

cd /Users/dcroton/Local/testing-ground/sage/code

# I/O Files (io_*)
mv tree_binary.c io_tree_binary.c
mv tree_binary.h io_tree_binary.h
mv tree_hdf5.c io_tree_hdf5.c
mv tree_hdf5.h io_tree_hdf5.h
mv core_io_tree.c io_tree.c
mv core_save.c io_save_binary.c

# Create header file for io_tree.c (from core_io_tree.c)
# We need to create this as it doesn't exist
cat > io_tree.h << 'EOF'
#ifndef IO_TREE_H
#define IO_TREE_H

/* Functions for tree I/O operations */
void load_tree(int filenr);
void close_tree_files(void);

#endif /* IO_TREE_H */
EOF

# Create header file for io_save_binary.c (from core_save.c)
cat > io_save_binary.h << 'EOF'
#ifndef IO_SAVE_BINARY_H
#define IO_SAVE_BINARY_H

/* Functions for binary file saving */
void save_galaxies(int filenr, int tree);

#endif /* IO_SAVE_BINARY_H */
EOF

# Utility Files (util_*)
mv error_handling.c util_error.c
mv error_handling.h util_error.h
mv parameter_table.c util_parameters.c
mv parameter_table.h util_parameters.h
mv core_mymalloc.c util_memory.c

# Create header file for util_memory.c (from core_mymalloc.c)
cat > util_memory.h << 'EOF'
#ifndef UTIL_MEMORY_H
#define UTIL_MEMORY_H

/* Memory allocation utilities */
void *mymalloc(size_t size);
void *myrealloc(void *ptr, size_t size);
void myfree(void *ptr);

#endif /* UTIL_MEMORY_H */
EOF

echo "File renaming complete."
