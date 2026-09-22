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

VERSION  = --version
CFLAGS   = -O3 -ffast-math $(OPENMP)
# CFLAGS   = -O0 -g -std=c99 $(OPENMP)
# -no-pie: the prebuilt ScaMaC static lib is non-PIC (R_X86_64_32 relocations),
# which the default PIE link cannot consume.
LFLAGS   = $(OPENMP) -g -no-pie
DEFINES  += -D_GNU_SOURCE # -DVERBOSE
INCLUDES =
LIBS     = -lm
