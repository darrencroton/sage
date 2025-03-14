# USE-MPI = yes  # set this if you want to run in embarrassingly parallel
# USE-HDF5 = yes # set this if you want to read in hdf5 trees (requires hdf5 libraries)

LIBS :=
CFLAGS :=
OPT := 

EXEC := sage 
OBJS := ./code/main.o \
        ./code/util_parameters.o \
        ./code/util_error.o \
        ./code/util_integration.o \
        ./code/util_numeric.o \
        ./code/util_version.o \
        ./code/io_util.o \
	./code/core_read_parameter_file.o \
	./code/core_init.o \
	./code/io_tree.o \
	./code/core_cool_func.o \
	./code/core_build_model.o \
	./code/io_save_binary.o \
	./code/util_memory.o \
	./code/core_allvars.o \
	./code/core_simulation_state.o \
	./code/model_infall.o \
	./code/model_cooling_heating.o \
	./code/model_starformation_and_feedback.o \
	./code/model_disk_instability.o \
	./code/model_reincorporation.o \
	./code/model_mergers.o \
	./code/model_misc.o \
	./code/io_tree_binary.o

INCL := ./code/core_allvars.h  \
        ./code/io_tree.h \
        ./code/io_save_binary.h \
        ./code/util_memory.h \
        ./code/util_integration.h \
        ./code/util_numeric.h \
        ./code/util_version.h \
	./code/io_util.h \
	./code/core_proto.h  \
	./code/core_simulation.h  \
	./code/util_parameters.h \
	./code/util_error.h \
	./code/config.h \
	./code/constants.h \
	./code/globals.h \
	./code/types.h \
	./code/io_tree_binary.h \
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

    OBJS += ./code/io_tree_hdf5.o ./code/io_save_hdf5.o
    INCL += ./code/io_tree_hdf5.h ./code/io_save_hdf5.h

    OPT += -DHDF5
    LIBS += $(HDF5LIB)
    CFLAGS += $(HDF5INCL) 
endif

GITREF = -DGITREF_STR='"$(shell git show-ref --head | head -n 1 | cut -d " " -f 1)"'

# GSL dependency removed - using custom implementations instead

OPTIMIZE = -g -O0 -Wall # optimization and warning flags

LIBS   +=   -g -lm
CFLAGS +=   $(OPTIONS) $(OPT) $(OPTIMIZE)


default: all

$(EXEC): $(OBJS) 
	$(CC) $(OPTIMIZE) $(OBJS) $(LIBS)   -o  $(EXEC)

$(OBJS): $(INCL) 

clean:
	rm -f $(OBJS) $(EXEC)

tidy:
	rm -f $(OBJS) ./$(EXEC)

all:  $(EXEC) 

