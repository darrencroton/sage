# USE-MPI = yes  # set this if you want to run in embarrassingly parallel
# USE-HDF5 = yes # set this if you want to read in hdf5 trees (requires hdf5 libraries)

LIBS :=
CFLAGS :=
OPT := 

EXEC := sage 
OBJS := ./code/main.o \
	./code/parameter_table.o \
	./code/core_read_parameter_file.o \
	./code/core_init.o \
	./code/core_io_tree.o \
	./code/core_cool_func.o \
	./code/core_build_model.o \
	./code/core_save.o \
	./code/core_mymalloc.o \
	./code/core_allvars.o \
	./code/model_infall.o \
	./code/model_cooling_heating.o \
	./code/model_starformation_and_feedback.o \
	./code/model_disk_instability.o \
	./code/model_reincorporation.o \
	./code/model_mergers.o \
	./code/model_misc.o \
	./code/io/tree_binary.o

INCL := ./code/core_allvars.h  \
	./code/core_proto.h  \
	./code/core_simulation.h  \
	./code/parameter_table.h \
	./code/config.h \
	./code/constants.h \
	./code/globals.h \
	./code/types.h \
	./code/io/tree_binary.h \
	./Makefile 

ifdef USE-MPI
    OPT += -DMPI  #  This creates an MPI version that can be used to process files in parallel
    CC = mpicc  # sets the C-compiler
else
    CC = cc  # sets the C-compiler
endif

ifdef USE-HDF5
    HDF5DIR := usr/local/x86_64/gnu/hdf5-1.8.17-openmpi-1.10.2-psm
    HDF5INCL := -I$(HDF5DIR)/include
    HDF5LIB := -L$(HDF5DIR)/lib -lhdf5 -Xlinker -rpath -Xlinker $(HDF5DIR)/lib

    OBJS += ./code/io/tree_hdf5.o
    INCL += ./code/io/tree_hdf5.h

    OPT += -DHDF5
    LIBS += $(HDF5LIB)
    CFLAGS += $(HDF5INCL) 
endif

GITREF = -DGITREF_STR='"$(shell git show-ref --head | head -n 1 | cut -d " " -f 1)"'

# GSL automatic detection
GSL_FOUND := $(shell gsl-config --version 2>/dev/null)
ifndef GSL_FOUND
  $(warning GSL not found in path - please install GSL before installing SAGE (or, update the PATH environment variable such that "gsl-config" is found))
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

OPTIMIZE = -g -O0 -Wall # optimization and warning flags

LIBS   +=   -g -lm  $(GSL_LIBS) 
CFLAGS +=   $(OPTIONS) $(OPT) $(OPTIMIZE) $(GSL_INCL)


default: all

$(EXEC): $(OBJS) 
	$(CC) $(OPTIMIZE) $(OBJS) $(LIBS)   -o  $(EXEC)

$(OBJS): $(INCL) 

clean:
	rm -f $(OBJS) $(EXEC)

tidy:
	rm -f $(OBJS) ./$(EXEC)

all:  $(EXEC) 

