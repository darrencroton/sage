# USE-MPI = yes  # set this if you want to run in embarrassingly parallel
# USE-HDF5 = yes # set this if you want to read in hdf5 trees (requires hdf5 libraries)

ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
LIBS :=
OPTS := -DROOT_DIR='"${ROOT_DIR}"' #-DOLD_VERSION
CCFLAGS := -DGNU_SOURCE -std=gnu99
SRC_PREFIX := src

SRC := main.c core_read_parameter_file.c core_init.c core_io_tree.c \
       core_cool_func.c core_build_model.c core_save.c core_mymalloc.c core_utils.c progressbar.c \
       core_allvars.c model_infall.c model_cooling_heating.c model_starformation_and_feedback.c \
       model_disk_instability.c model_reincorporation.c model_mergers.c model_misc.c \
       io/tree_binary.c 
SRC  := $(addprefix $(SRC_PREFIX)/, $(SRC))
OBJS := $(SRC:.c=.o)
INCL := core_allvars.h core_proto.h core_simulation.h core_utils.h progressbar.h io/tree_binary.h 
INCL := $(addprefix $(SRC_PREFIX)/, $(INCL))

EXEC := sage 

ifdef USE-MPI
    OPTS += -DMPI  #  This creates an MPI version that can be used to process files in parallel
    CC := mpicc  # sets the C-compiler
else
    CC := gcc  # sets the C-compiler
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

  ifdef USE-HDF5
    ifndef HDF5_DIR
      ifeq ($(ON_CI), true)
        $(info Looks like we are building on a continuous integration service)
        $(info Assuming that the `hdf5` package and `gsl-config` are both installed by `conda` into the same directory)
        CONDA_BASE := $(shell gsl-config --prefix 2>/dev/null)
        HDF5_DIR := $(CONDA_BASE)
      else
        $(warning $$HDF5_DIR environment variable is not defined but HDF5 is requested)
        $(warning Please install HDF5 (or perhaps load the HDF5 module 'module load hdf5-serial') or disable the 'USE-HDF5' option in the 'Makefile')
        ## Define your HDF5 install directory here
        ## or outside before the USE-HDF5 if condition
        ## to avoid the warning message in the following line
        HDF5_DIR:= ${HOME}/anaconda3
        $(warning Proceeding with a default directory of `[${HDF5_DIR}]` - compilation might fail)
      endif
    endif

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
  ifndef GSL_FOUND
    $(warning GSL not found in path - please install GSL before installing SAGE (or, update the PATH environment variable such that "gsl-config" is found))
    $(warning Assuming GSL *might* be in $(GSL_DIR) and trying to compile)
    # if the automatic detection fails, set GSL_DIR appropriately
    GSL_DIR := /opt/local
    GSL_INCL := -I$(GSL_DIR)/include  
    GSL_LIBDIR := $(GSL_DIR)/lib
    # since GSL is not in PATH, the runtime environment might not be setup correctly either
    # therefore, adding the compiletime library path is even more important (the -Xlinker bit)
    GSL_LIBS := -L$(GSL_LIBDIR) -lgsl -lgslcblas -Xlinker -rpath -Xlinker $(GSL_LIBDIR) 
  else
    # GSL is probably configured correctly, pick up the locations automatically
    GSL_INCL := $(shell gsl-config --cflags)
    GSL_LIBDIR := $(shell gsl-config --prefix)/lib
    GSL_LIBS   := $(shell gsl-config --libs) -Xlinker -rpath -Xlinker $(GSL_LIBDIR)
  endif
  CCFLAGS += $(GSL_INCL)
  LIBS += $(GSL_LIBS)

  # The tests should test with the same compile flags
  # that users are expected to use. Disabled the previous
  # different optimization flags for travis vs regular users
  # This decision was driven by the fact the adding the `-march=native` flag
  # produces test failures on ozstar (https://supercomputing.swin.edu.au/ozstar/)
  # Good news is that even at -O3 the tests pass
  OPTIMIZE := -O2

  CCFLAGS += -g -Wextra -Wshadow -Wall  #-Wpadded # and more warning flags 
  LIBS   +=   -lm
endif # End of DO_CHECKS if condition -> i.e., we do need to care about paths and such

default: all

$(EXEC): $(OBJS) 
	$(CC) $(CFLAGS) $(OBJS) $(LIBS)   -o  $(EXEC)

%.o: %.c $(INCL) Makefile
	$(CC) $(OPTS) $(OPTIMIZE) $(CCFLAGS) -c $< -o $@

.phony: clean celan celna clena

clean:
	rm -f $(OBJS) $(EXEC)

celan celna clena: clean

tests: $(EXEC)
	./src/tests/test_sage.sh

tidy:
	rm -f $(OBJS) ./$(EXEC)

all:  $(EXEC) 

