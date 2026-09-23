#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scamac_safeint.h"

#include "scamac_multidx.h"

ScamacErrorCode scamac_multidx_alloc(int n, scamac_multidx_st **midx)
{
  if (!midx) {
    return SCAMAC_ENULL;
  }
  if (n < 1) {
    return SCAMAC_ERANGE;
  }
  scamac_multidx_st *my_midx = malloc(sizeof *my_midx);
  if (!my_midx) {
    return SCAMAC_EMALLOCFAIL;
  }
  my_midx->n  = n;
  my_midx->ni = calloc(n, sizeof *(my_midx->ni));
  if (!(my_midx->ni)) {
    return SCAMAC_EMALLOCFAIL;
  }
  my_midx->niprod = calloc(n, sizeof *(my_midx->niprod));
  if (!(my_midx->niprod)) {
    return SCAMAC_EMALLOCFAIL;
  }
  my_midx->nidx = 0;
  *midx         = my_midx;
  return SCAMAC_EOK;
}

ScamacErrorCode scamac_multidx_free(scamac_multidx_st *midx)
{
  if (midx) {
    free(midx->ni);
    free(midx->niprod);
    free(midx);
  }
  return SCAMAC_EOK;
}

ScamacErrorCode scamac_multidx_set(scamac_multidx_st *midx, int pos, ScamacIdx ni)
{
  if (!midx) {
    return SCAMAC_ENULL;
  }
  if ((pos < 0) || (pos >= midx->n)) {
    return SCAMAC_ERANGE;
  }
  if (ni < 0) {
    return SCAMAC_ERANGE;
  }

  midx->ni[pos] = ni;
  int i;
  midx->niprod[0] = 1;
  for (i = 1; i < midx->n; i++) {
    midx->niprod[i] = scamac_safeidx_mult(midx->niprod[i - 1], midx->ni[i - 1]);
    if (midx->niprod[i] < 0) {
      return SCAMAC_EOVERFLOW;
    }
  }
  midx->nidx = scamac_safeidx_mult(midx->niprod[midx->n - 1], midx->ni[midx->n - 1]);
  if (midx->nidx < 0) {
    return SCAMAC_EOVERFLOW;
  }

  return SCAMAC_EOK;
}

ScamacIdx scamac_multidx_nidx(const scamac_multidx_st *midx)
{
  if (midx) {
    return midx->nidx;
  } else {
    return -1;
  }
}

ScamacIdx scamac_multidx_decode(const scamac_multidx_st *midx, int pos, ScamacIdx idx)
{
  if (0 <= idx && idx < midx->nidx) {
    idx = idx / midx->niprod[pos];
    idx = idx % midx->ni[pos];
    return idx;
  } else {
    return -1;
  }
}

ScamacIdx scamac_multidx_upd(
    const scamac_multidx_st *midx, int pos, ScamacIdx idx_at_pos, ScamacIdx idx)
{
  if (0 <= pos && pos < midx->n && 0 <= idx_at_pos && idx_at_pos < midx->ni[pos]) {
    ScamacIdx idxnew, idx_at_old;
    idx_at_old = (idx / midx->niprod[pos]) % midx->ni[pos];
    idxnew     = idx + (idx_at_pos - idx_at_old) * midx->niprod[pos];
    return idxnew;
  } else {
    return -1;
  }
}
