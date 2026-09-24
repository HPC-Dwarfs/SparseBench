#include "scamac_benchmarks.h"
#include "scamac_collection.h"
#include <stdlib.h>
#include <string.h>

int scamac_benchmark_how_many(const char *name)
{
  if (!name) {
    return 0;
  }
#include "scamac_benchmarks_inc1.c"
  return 0;
}

ScamacErrorCode scamac_benchmark_obtain(
    const char *name, int m, int x, ScamacGenerator **gen)
{
  if (!name) {
    return scamac_error_set_par(SCAMAC_ENULL, 1);
  }
  if (m < 1) {
    return scamac_error_set_par(SCAMAC_ERANGE, 2);
  }
  if (x < 1) {
    return scamac_error_set_par(SCAMAC_ERANGE, 3);
  }
  if (!gen) {
    return scamac_error_set_par(SCAMAC_ENULL, 4);
  }
  ScamacErrorCode err;
#include "scamac_benchmarks_inc2.c"
  return scamac_error_set_par(SCAMAC_EINPUT, 1);
}

ScamacErrorCode scamac_list_benchmarks(char **desc)
{
  if (desc) {
    char *my_string;
#include "scamac_benchmarks_inc3.c"
    *desc = my_string;
    return SCAMAC_EOK;
  } else {
    return SCAMAC_ENULL;
  }
}
