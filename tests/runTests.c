
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef SCS
#include "matrix/matrixTests.h"
#endif
#include "solver/solverTestsSPMMV.h"
#include "solver/solverTestsSPMV.h"
#include "solver/solverTestsSPMVSplit.h"

#if defined(RUNTIME_BACKEND_IS_CUDA) || defined(RUNTIME_BACKEND_IS_HIP)
#include "../src/cuda/cuda_kernels.h"
#endif

int main(int argc, char **argv)
{
  int failures = 0;

#if defined(RUNTIME_BACKEND_IS_CUDA) || defined(RUNTIME_BACKEND_IS_HIP)
  gpu_init(0);
#endif
#ifdef SCS
  failures += (matrixTests(argc, argv) != 0);
#endif
  // Self-contained and fast, so run it before the data-driven tests
  failures += (solverTestsSPMVSplit(argc, argv) != 0);
  failures += (solverTestsSPMV(argc, argv) != 0);
  failures += (solverTestsSPMMV(argc, argv) != 0);
#if defined(RUNTIME_BACKEND_IS_CUDA) || defined(RUNTIME_BACKEND_IS_HIP)
  gpu_finalize();
#endif

  if (failures != 0) {
    printf("\n%d test suite(s) reported failures.\n", failures);
    return 1;
  }
  printf("\nAll test suites passed.\n");
  return 0;
}
