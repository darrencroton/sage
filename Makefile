EXEC   = sage 

OBJS   = 	./code/main.o \
			./code/core_read_parameter_file.o \
			./code/core_init.o \
			./code/core_io_tree.o \
			./code/core_cool_func.o \
			./code/core_build_model.o \
			./code/core_save.o \
			./code/core_mymalloc.o \
			./code/core_allvars.o \
			./code/model_infall.o \
			./code/model_cooling.o \
			./code/model_starformation_and_feedback.o \
			./code/model_disk_instability.o \
			./code/model_reincorporation.o \
			./code/model_mergers.o \
			./code/model_misc.o \
			./code/model_z_dependent_sf.o

INCL   =	./code/core_allvars.h  \
			./code/core_proto.h  \
			./code/core_simulation.h  \
			./Makefile


OPT += -DNOUT=1           # This sets the number of galaxy output times

OPT += -DMILLENNIUM         # Millennium simulation trees
# OPT += -DBOLSHOI            # Bolshoi simulation trees
# OPT += -DGIGGLEZ						# GiggleZ simulation trees

SYSTYPE = "mac"
# SYSTYPE = "green"
# SYSTYPE = "gstar"


CC       =   mpicc            # sets the C-compiler (default)
OPTIMIZE =   -g -O0 -Wall    # optimization and warning flags (default)
GSL_INCL = -I$(GSL_DIR)/include
GSL_LIBS = -L$(GSL_DIR)/lib

# ifeq ($(SYSTYPE),"mac")
# CC       =  mpicc
# GSL_INCL = -I/opt/local/include
# GSL_LIBS = -L/opt/local/lib
# endif

# ifeq ($(SYSTYPE),"green")
# CC       = /usr/local/gnu/x86_64/openmpi-1.4/bin/mpicc
# OPTIMIZE = -O3 -Wall
# GSL_INCL = -I/usr/local/gnu/x86_64/gsl/include
# GSL_LIBS = -L/usr/local/gnu/x86_64/gsl/lib
# endif

# ifeq ($(SYSTYPE),"gstar")
# CC       = /usr/local/x86_64/gnu/openmpi-1.4.5/bin/mpicc
# OPTIMIZE = -O3 -Wall
# GSL_INCL = -I/usr/local/x86_64/gnu/gsl-1.9/include
# GSL_LIBS = -L/usr/local/x86_64/gnu/gsl-1.9/lib
# endif


LIBS   =   -g -lm  $(GSL_LIBS) -lgsl -lgslcblas 

CFLAGS =   $(OPTIONS) $(OPT) $(OPTIMIZE) $(GSL_INCL)

default: all

$(EXEC): $(OBJS) 
	$(CC) $(OPTIMIZE) $(OBJS) $(LIBS)   -o  $(EXEC)

$(OBJS): $(INCL) 

clean:
	rm -f $(OBJS)

tidy:
	rm -f $(OBJS) ./$(EXEC)

all:  tidy $(EXEC) clean
