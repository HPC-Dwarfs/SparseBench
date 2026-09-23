#include <stdlib.h>
#include <math.h>
#include <complex.h>

#include "scamac_internal.h"
#include "scamac_string.h"
#include "scamac_safeint.h"

#include "scamac_matrix_DiagonalReal.h"
#include "scamac_matrix_DiagonalReal_inc.c"

ScamacErrorCode scamac_matrix_DiagonalReal_tables_create(
    const scamac_matrix_DiagonalReal_params_st *par, void **tab, scamac_info_st *info)
{
  if (info) {
    info->nrow     = par->n;
    info->ncol     = par->n;
    info->maxnzrow = 1;
    info->maxnzcol = 1;
    info->maxnzgen = 1;
    info->maxnz    = scamac_safeidx_mult(info->nrow, info->maxnzrow);
    if (info->maxnz < 0) {
      return SCAMAC_EOVERFLOW;
    }
    info->valtype  = SCAMAC_VAL_REAL;
    info->symmetry = SCAMAC_GENERAL;
  }
  return SCAMAC_EOK;
}

ScamacErrorCode scamac_matrix_DiagonalReal_generate_row(
    const scamac_matrix_DiagonalReal_params_st *par,
    const void *tab,
    void *ws,
    ScamacIdx irow,
    ScamacFlag flag,
    scamac_sparserow_real_st *row)
{

  if (!par || !row) {
    return SCAMAC_ENULL;
  }

  if (flag & ~SCAMAC_TRANSPOSE & ~SCAMAC_CONJUGATE & ~SCAMAC_KEEPZEROS) {
    return SCAMAC_EINVALID;
  }

  if ((irow < 0) || (irow >= par->n)) {
    return SCAMAC_ERANGE;
  }

  if (par->n == 1) {
    SCAMAC_APPROVE(scamac_sparserow_real_add(row, 0.5 * (par->dmin + par->dmax), irow));
  } else {
    double v =
        par->dmin + ((double)irow / (double)(par->n - 1)) * (par->dmax - par->dmin);
    SCAMAC_APPROVE(scamac_sparserow_real_add(row, v, irow));
  }

  return SCAMAC_EOK;
}
