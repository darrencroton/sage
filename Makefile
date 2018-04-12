ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
SRC := main.c core_read_parameter_file.c core_init.c core_io_tree.c \
       core_cool_func.c core_build_model.c core_save.c core_mymalloc.c \
       core_allvars.c model_infall.c model_cooling_heating.c model_starformation_and_feedback.c \
       model_disk_instability.c model_reincorporation.c model_mergers.c model_misc.c \
       io/tree_binary.c io/tree_hdf5.c
SRC  := $(addprefix src/, $(SRC))
OBJS := $(SRC:.c=.o)
INCL := core_allvars.h core_proto.h core_simulation.h io/tree_binary.h io/tree_hdf5.h
INCL := $(addprefix src/, $(INCL))

EXEC := sage 

# USE-MPI = yes  # set this if you want to run in embarrassingly parallel
USE-HDF5 = yes

LIBS :=
OPTS := -DROOT_DIR='"${ROOT_DIR}"'
CCFLAGS := -DGNU_SOURCE -std=gnu99


ifdef USE-MPI
    OPTS += -DMPI  #  This creates an MPI version that can be used to process files in parallel
    CC := mpicc  # sets the C-compiler
else
    CC := gcc  # sets the C-compiler
endif


ON_CI := false
ifeq ($(CI), true)
    ON_CI := true
endif

ifeq ($(TRAVIS), true)
    ON_CI := true
endif

# Add the -Werror flag if running on some continuous integration provider
ifeq ($(ON_CI), true)
    CCFLAGS += -Werror -Wno-unknown-warning-option
endif

ifdef USE-HDF5
    ifndef HDF5_DIR
        $(warning $$HDF5_DIR environment variable is not defined but HDF5 is requested)
        $(warning Please install HDF5 (or perhaps load the HDF5 module 'module load hdf5-serial') or disable the 'USE-HDF5' option in the 'Makefile')
        ifeq ($(ON_CI), true)
            $(info Looks like we are building on a continuous integration service. Assuming that the package `hdf5tools` are installed)
            CONDA_FOUND := $(shell conda -V 2>/dev/null)
            ifndef CONDA_FOUND
                H5DIFF_LOC := $(shell which h5diff 2>/dev/null)
                $(warning $$H5DIFF_LOC is [${H5DIFF_LOC}])
                ifndef H5DIFF_LOC
                   $(warning Could not locate HDF5_DIR on continuous integration service)
                endif 
                HDF5_DIR := $(H5DIFF_LOC)/..
            else 
                CONDA_BASE := $(shell conda info --base 2>/dev/null)
                HDF5_DIR := $(CONDA_BASE)
            endif
        else
            HDF5_DIR := /usr/local/x86_64/gnu/hdf5-1.8.17-openmpi-1.10.2-psm
        endif

        $(warning Proceeding with a default directory of `[${HDF5_DIR}]` - compilation might fail)
    endif

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


OPTIMIZE = -O3 -march=native # optimization and warning flags
CCFLAGS += -g -Wextra -Wshadow -Wall  #-Wpadded 
LIBS   +=   -lm

default: all

$(EXEC): $(OBJS) 
	$(CC) $(CFLAGS) $(OBJS) $(LIBS)   -o  $(EXEC)

%.o: %.c $(INCL) Makefile
	$(CC) $(OPTS) $(OPTIMIZE) $(CCFLAGS) -c $< -o $@

.phony: clean celan celna clena

clean:
	rm -f $(OBJS) $(EXEC)

celan celna clena: clean

tidy:
	rm -f $(OBJS) ./$(EXEC)

all:  $(EXEC) 

