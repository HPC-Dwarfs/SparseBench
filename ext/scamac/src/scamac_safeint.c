#include "scamac_safeint.h"

ScamacIdx scamac_safeidx_add(ScamacIdx a, ScamacIdx b)
{
  if ((a < 0) || (b < 0)) {
    return -1;
  } else if (a > (SCAMAC_IDX_MAX - b)) {
    return -2;
  } else {
    return a + b;
  }
}

ScamacIdx scamac_safeidx_mult(ScamacIdx a, ScamacIdx b)
{
  if ((a < 0) || (b < 0)) {
    return -1;
  } else if (b == 0) {
    return 0;
  } else if (a > (SCAMAC_IDX_MAX / b)) {
    return -2;
  } else {
    return a * b;
  }
}

ScamacInt scamac_safeint_add(ScamacInt a, ScamacInt b)
{
  if ((a < 0) || (b < 0)) {
    return -1;
  } else if (a > (SCAMAC_INT_MAX - b)) {
    return -2;
  } else {
    return a + b;
  }
}

ScamacInt scamac_safeint_mult(ScamacInt a, ScamacInt b)
{
  if ((a < 0) || (b < 0)) {
    return -1;
  } else if (b == 0) {
    return 0;
  } else if (a > (SCAMAC_INT_MAX / b)) {
    return -2;
  } else {
    return a * b;
  }
}
