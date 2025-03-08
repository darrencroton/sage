#!/bin/zsh
# Script to update references to renamed files in the SAGE codebase

cd /Users/dcroton/Local/testing-ground/sage/code

# Update include statements for I/O files
find . -type f -name "*.c" -o -name "*.h" | xargs sed -i '' 's|#include "tree_binary.h"|#include "io_tree_binary.h"|g'
find . -type f -name "*.c" -o -name "*.h" | xargs sed -i '' 's|#include "tree_hdf5.h"|#include "io_tree_hdf5.h"|g'
find . -type f -name "*.c" -o -name "*.h" | xargs sed -i '' 's|#include "error_handling.h"|#include "util_error.h"|g'
find . -type f -name "*.c" -o -name "*.h" | xargs sed -i '' 's|#include "parameter_table.h"|#include "util_parameters.h"|g'

# Update core_proto.h with new function prototypes and includes
sed -i '' 's|#include "error_handling.h"|#include "util_error.h"|g' core_proto.h
sed -i '' 's|#include "parameter_table.h"|#include "util_parameters.h"|g' core_proto.h

# Add new header includes to appropriate files
sed -i '' '/^#include "core_proto.h"/a\
#include "io_tree.h"
' io_tree.c
sed -i '' '/^#include "util_error.h"/a\
#include "util_memory.h"
' util_memory.c
sed -i '' '/^#include "core_proto.h"/a\
#include "io_save_binary.h"
' io_save_binary.c

# Update Makefile
cd /Users/dcroton/Local/testing-ground/sage
sed -i '' 's|./code/parameter_table.o|./code/util_parameters.o|g' Makefile
sed -i '' 's|./code/error_handling.o|./code/util_error.o|g' Makefile
sed -i '' 's|./code/core_io_tree.o|./code/io_tree.o|g' Makefile
sed -i '' 's|./code/core_save.o|./code/io_save_binary.o|g' Makefile
sed -i '' 's|./code/core_mymalloc.o|./code/util_memory.o|g' Makefile
sed -i '' 's|./code/tree_binary.o|./code/io_tree_binary.o|g' Makefile

sed -i '' 's|./code/parameter_table.h|./code/util_parameters.h|g' Makefile
sed -i '' 's|./code/error_handling.h|./code/util_error.h|g' Makefile
sed -i '' 's|./code/tree_binary.h|./code/io_tree_binary.h|g' Makefile

# Add new header files to INCL in Makefile
sed -i '' '/INCL :=/a\
	./code/io_tree.h \\\n\
	./code/io_save_binary.h \\\n\
	./code/util_memory.h \\\n\
' Makefile

# Update HDF5 section in Makefile
sed -i '' 's|./code/tree_hdf5.o|./code/io_tree_hdf5.o|g' Makefile
sed -i '' 's|./code/tree_hdf5.h|./code/io_tree_hdf5.h|g' Makefile

echo "Reference updates complete."
