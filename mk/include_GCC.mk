ifeq ($(strip $(ENABLE_MPI)),true)
CC = mpicc
DEFINES += -D_MPI
else
CC = gcc
endif

LD = $(CC)

ifeq ($(strip $(ENABLE_OPENMP)),true)
OPENMP   = -fopenmp
endif

# Set default 
C_VERSION = c17
ifeq ($(strip $(HAVE_C23)),true)
C_VERSION = c23
endif

VERSION  = --version
CFLAGS   = -O3  -ffast-math -std=$(C_VERSION) $(OPENMP)
# CFLAGS   = -O0 -g -std=c99 $(OPENMP)
LFLAGS   = $(OPENMP)
DEFINES  += -D_GNU_SOURCE # -DVERBOSE
INCLUDES =
LIBS     = -lm
