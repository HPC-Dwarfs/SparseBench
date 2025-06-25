#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "matrix/matrixTests.h"
#include "solver/solverTests.h"

#ifdef _MPI
#include <mpi.h>
#endif

int main(int argc, char** argv)
{
#ifdef _MPI
  MPI_Init(&argc, &argv);
#endif

  matrixTests(argc, argv);
  solverTests(argc, argv);

#ifdef _MPI
  MPI_Finalize();
#endif

  return 0;
}