# USE-MPI = yes  # set this if you want to run in embarrassingly parallel
# USE-HDF5 = yes # set this if you want to read in hdf5 trees (requires hdf5 libraries)

ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
LIBS :=
OPTS := -DROOT_DIR='"${ROOT_DIR}"'
CCFLAGS := -DGNU_SOURCE -std=gnu99 -fPIC
SRC_PREFIX := src

LIBNAME := sage
LIBSRC :=  sage.c core_read_parameter_file.c core_init.c core_io_tree.c \
           core_cool_func.c core_build_model.c core_save.c core_mymalloc.c core_utils.c progressbar.c \
           core_allvars.c model_infall.c model_cooling_heating.c model_starformation_and_feedback.c \
           model_disk_instability.c model_reincorporation.c model_mergers.c model_misc.c \
           io/tree_binary.c
LIBINCL := $(LIBSRC:.c=.h)

SRC := main.c $(LIBSRC)
SRC  := $(addprefix $(SRC_PREFIX)/, $(SRC))
OBJS := $(SRC:.c=.o)
INCL := core_allvars.h macros.h core_simulation.h $(LIBINCL)
INCL := $(addprefix $(SRC_PREFIX)/, $(INCL))

LIBSRC  := $(addprefix $(SRC_PREFIX)/, $(LIBSRC))
LIBINCL := $(addprefix $(SRC_PREFIX)/, $(LIBINCL))
LIBOBJS := $(LIBSRC:.c=.o)
SAGELIB := lib$(LIBNAME).a

EXEC := $(LIBNAME)

UNAME := $(shell uname)
ifdef USE-MPI
    OPTS += -DMPI  #  This creates an MPI version that can be used to process files in parallel
    CC := mpicc  # sets the C-compiler
else
    # use clang by default on OSX and gcc on linux
    ifeq ($(UNAME), Darwin)
      CC := clang
    else
      CC := gcc
    endif
endif

# No need to do the path + library checks if
# only attempting to clean the build
DO_CHECKS := 1
CLEAN_CMDS := celan celna clean clena
ifneq ($(filter $(CLEAN_CMDS),$(MAKECMDGOALS)),)
  DO_CHECKS := 0
endif

## Only set everything if the command is not "make clean" (or related to "make clean")
ifeq ($(DO_CHECKS), 1)
  ON_CI := false
  ifeq ($(CI), true)
    ON_CI := true
  endif

  ifeq ($(TRAVIS), true)
    ON_CI := true
  endif

  # Add the -Werror flag if running on some continuous integration provider
  ifeq ($(ON_CI), true)
    CCFLAGS += -Werror
  endif

  ## Check if CC is clang under the hood
  CC_VERSION := $(shell $(CC) --version 2>/dev/null)
  ifndef CC_VERSION
    $(info Error: Could find compiler = ${CC})
    $(info Please either set "CC" in "Makefile" or via the command-line "make CC=yourcompiler")
    $(info And please check that the specified compiler is in your "$$PATH" variables)
    $(error )
  endif
  ifeq (clang,$(findstring clang,$(CC_VERSION)))
    CC_IS_CLANG := 1
  else
    CC_IS_CLANG := 0
  endif
  ifeq ($(UNAME), Darwin)    
    ifeq ($(CC_IS_CLANG), 0)
      ## gcc on OSX has trouble with
      ## AVX+ instructions. This flag uses
      ## the clang assembler
      CCFLAGS += -Wa,-q
    endif
  endif
  ## end of checking is CC 

  ifdef USE-HDF5
    ifndef HDF5_DIR
      ifeq ($(ON_CI), true)
        $(info Looks like we are building on a continuous integration service)
        $(info Assuming that the `hdf5` package and `gsl-config` are both installed by `conda` into the same directory)
        CONDA_BASE := $(shell gsl-config --prefix 2>/dev/null)
        HDF5_DIR := $(CONDA_BASE)
      else
        ## Check if h5tools are installed and use the base directory
        ## Assumes that the return value from 'which' will be
        ## something like /path/to/bin/h5ls; the 'sed' command
        ## replaces the '/bin/h5ls' with '' (i.e., removes '/bin/h5ls')
        ## and returns '/path/to' (without the trailing '/')
        HDF5_DIR := `which h5ls 2>/dev/null | sed 's/\/bin\/h5ls//'`
        ifndef HDF5_DIR
          $(warning $$HDF5_DIR environment variable is not defined but HDF5 is requested)
          $(warning Could not locate hdf5 tools either)
          $(warning Please install HDF5 (or perhaps load the HDF5 module 'module load hdf5-serial') or disable the 'USE-HDF5' option in the 'Makefile')
          ## Define your HDF5 install directory here
          ## or outside before the USE-HDF5 if condition
          ## to avoid the warning message in the following line
          HDF5_DIR:= ${HOME}/anaconda3
          $(warning Proceeding with a default directory of `[${HDF5_DIR}]` - compilation might fail)
        endif
      endif
    endif

    LIBSRCS += $(SRC_PREFIX)/io/tree_hdf5.c
    LIBOBJS += $(SRC_PREFIX)/io/tree_hdf5.o
    LIBINCL += $(SRC_PREFIX)/io/tree_hdf5.h

    SRCS += $(SRC_PREFIX)/io/tree_hdf5.c
    OBJS += $(SRC_PREFIX)/io/tree_hdf5.o
    INCL += $(SRC_PREFIX)/io/tree_hdf5.h

    HDF5_INCL := -I$(HDF5_DIR)/include
    HDF5_LIB := -L$(HDF5_DIR)/lib -lhdf5 -Xlinker -rpath -Xlinker $(HDF5_DIR)/lib

    OPTS += -DHDF5
    LIBS += $(HDF5_LIB)
    CCFLAGS += $(HDF5_INCL) 
  endif

  GITREF = -DGITREF_STR='"$(shell git show-ref --head | head -n 1 | cut -d " " -f 1)"'

  # GSL automatic detection
  GSL_FOUND := $(shell gsl-config --version 2>/dev/null)
  ifdef GSL_FOUND
    OPTS += -DGSL_FOUND
    # GSL is probably configured correctly, pick up the locations automatically
    GSL_INCL := $(shell gsl-config --cflags)
    GSL_LIBDIR := $(shell gsl-config --prefix)/lib
    GSL_LIBS   := $(shell gsl-config --libs) -Xlinker -rpath -Xlinker $(GSL_LIBDIR)
  else
    $(warning GSL not found in $$PATH environment variable. Tests will be disabled)
  endif
  CCFLAGS += $(GSL_INCL)
  LIBS += $(GSL_LIBS)

  # The tests should test with the same compile flags
  # that users are expected to use. Disabled the previous
  # different optimization flags for travis vs regular users
  # This decision was driven by the fact the adding the `-march=native` flag
  # produces test failures on ozstar (https://supercomputing.swin.edu.au/ozstar/)
  # Good news is that even at -O3 the tests pass
  OPTIMIZE := -O2 -march=native -mno-fma

  CCFLAGS += -g -Wextra -Wshadow -Wall  #-Wpadded # and more warning flags 
  LIBS   +=   -lm
endif # End of DO_CHECKS if condition -> i.e., we do need to care about paths and such

all:  $(EXEC) $(SAGELIB)

$(EXEC): $(OBJS)
	$(CC) $(CCFLAGS) $(OBJS) $(LIBS)   -o  $(EXEC)

%.o: %.c $(INCL) Makefile
	$(CC) $(OPTS) $(OPTIMIZE) $(CCFLAGS) -c $< -o $@

$(SAGELIB): $(LIBOBJS)
	ar rcs $@ $(LIBOBJS) 

.phony: clean celan celna clena
celan celna clena: clean
clean:
	rm -f $(OBJS) $(EXEC) $(SAGELIB)

tests: $(EXEC)
ifdef GSL_FOUND
	./src/tests/test_sage.sh
else
	$(error GSL is required to run the tests)
endif




