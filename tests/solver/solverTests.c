#include "../common.h"
#ifdef CRS
#include "spmvCRS.h"
#endif
#ifdef SCS
#include "spmvSCS.h"
#endif

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int solverTests(int argc, char** argv)
{
  const char* dataDir = "data/";

  Test tests[] = {
#ifdef CRS
    { "SpMV CRS", test_spmvCRS },
#endif
#ifdef SCS
    { "SpMV Sell-1-1", test_spmvSCS }, // Test 1
    { "SpMV Sell-2-1", test_spmvSCS }, // Test 2
    { "SpMV Sell-4-1", test_spmvSCS }, // Test 3
#endif
    // Add more here...
  };

  int num_tests = sizeof(tests) / sizeof(tests[0]);
  int passed    = 0;

  Args args[num_tests];

#ifdef SCS
  // Manually assign one configuration per test
  SET_SCS_ARGS(0, 1, 1); // Test 1
  SET_SCS_ARGS(1, 2, 1); // Test 2
  SET_SCS_ARGS(2, 4, 1); // Test 3
#endif

  printf("Running %d Solver tests:\n", num_tests);
  for (int i = 0; i < num_tests; ++i) {
    printf("[%-2d/%-2d] %-20s ... \n", i + 1, num_tests, tests[i].name);
    fflush(stdout);

    if (!(tests[i].func(&args[i], dataDir))) {
      printf("✅ PASS\n");
      passed++;
    } else {
      printf("❌ FAIL\n");
    }
  }

  printf("\nSummary: %d/%d Solver tests passed.\n", passed, num_tests);

  return (passed == num_tests) ? 0 : 1;
}