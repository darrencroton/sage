# =============================================================================
# SAGE Makefile - Portable version for GNU and BSD make
# =============================================================================

# --- Configuration Flags ---
# To enable MPI for parallel processing, uncomment the following line:
# USE-MPI = yes

# To enable HDF5 for reading HDF5-format merger trees, uncomment the following line:
# USE-HDF5 = yes


# --- Compiler and Flags ---

# Default compiler is 'cc'. This will be overridden if MPI is enabled.
CC = cc

# Compiler flags:
# -g: Include debugging symbols
# -O2: Level 2 optimization
# -Wall: Enable all standard warnings
# -I./code: Add the 'code' directory to the include path
# -MMD -MP: Generate dependency files (.d) for automatic header tracking (GNU compatible)
OPTIMIZE := -g -O2 -Wall
CFLAGS   := $(OPTIMIZE) -I./code -MMD -MP

# Linker flags and libraries
LDFLAGS :=
LIBS    := -lm


# --- Source, Object, and Executable Definitions ---

# Name of the final executable
EXEC := sage

# Directories
SRC_DIR := ./code

# Automatically find all C source files using a portable shell command
SOURCES := $(shell find $(SRC_DIR) -name '*.c')

# Generate object file names from source files
OBJS := $(SOURCES:.c=.o)

# Generate dependency file names (for header tracking)
DEPS := $(OBJS:.o=.d)


# --- Conditional Compilation: MPI ---

ifdef USE-MPI
    CC := mpicc
    CFLAGS += -DMPI
endif


# --- Conditional Compilation: HDF5 ---

ifdef USE-HDF5
    # Standard system paths (works for Homebrew, apt, etc.)
    HDF5INCL :=
    HDF5LIB := -lhdf5

    CFLAGS += $(HDF5INCL) -DHDF5
    LDFLAGS += $(HDF5LIB)
else
    # If HDF5 is not used, filter out HDF5-specific source files
    OBJS := $(filter-out $(SRC_DIR)/io_%hdf5.o, $(OBJS))
endif


# --- Git Versioning ---

GIT_VERSION_IN := ./code/git_version.h.in
GIT_VERSION_H  := ./code/git_version.h


# =============================================================================
# Build Targets
# =============================================================================

# The default target when running 'make'
.PHONY: all
all: $(EXEC)

# Rule to link the final executable
$(EXEC): $(OBJS)
	@echo "==> Linking executable: $@"
	$(CC) $(CFLAGS) $^ $(LDFLAGS) $(LIBS) -o $@
	@echo "==> Build complete: ./$(EXEC)"

# Rule to generate the Git version header.
# Marked as .PHONY to ensure it's regenerated on every build.
.PHONY: $(GIT_VERSION_H)
$(GIT_VERSION_H): $(GIT_VERSION_IN)
	@echo "==> Generating Git version header"
	@sed -e "s/@GIT_COMMIT_HASH@/$$(git rev-parse HEAD)/g" \
	     -e "s/@GIT_BRANCH_NAME@/$$(git rev-parse --abbrev-ref HEAD)/g" \
	     $< > $@

# This rule ensures the git header is created before any compilation starts.
# It's a dependency for all object files.
$(OBJS): $(GIT_VERSION_H)

# Include all the auto-generated dependency files.
# The '-' prefix tells Make to ignore errors if the files don't exist yet.
-include $(DEPS)


# =============================================================================
# Cleaning Targets
# =============================================================================

.PHONY: clean tidy

# 'clean' removes all build artifacts, including the executable and git header
clean:
	@echo "==> Cleaning all build files"
	rm -f $(OBJS) $(DEPS) $(EXEC) $(GIT_VERSION_H)

# 'tidy' removes object files and the executable, but keeps the git header
tidy:
	@echo "==> Tidying build files"
	rm -f $(OBJS) $(DEPS) $(EXEC)