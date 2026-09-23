#include <stdlib.h>
#include <math.h>

#include "scamac_internal.h"
#include "scamac_safeint.h"
#include "scamac_string.h"

#include "scamac_matrix_Harmonic.h"
#include "scamac_matrix_Harmonic_inc.c"

ScamacErrorCode scamac_matrix_Harmonic_tables_create(
    const scamac_matrix_Harmonic_params_st *par, void **tab, scamac_info_st *info)
{
  if (info) {
    info->nrow     = par->n_bos;
    info->ncol     = par->n_bos;
    info->maxnzrow = 3;
    info->maxnzcol = 3;
    info->maxnzgen = 3;
    info->maxnz    = scamac_safeidx_mult(info->nrow, info->maxnzrow);
    if (info->maxnz < 0) {
      return SCAMAC_EOVERFLOW;
    }
    info->valtype  = SCAMAC_VAL_REAL;
    info->symmetry = SCAMAC_SYMMETRIC;
  }
  return SCAMAC_EOK;
}

ScamacErrorCode scamac_matrix_Harmonic_generate_row(
    const scamac_matrix_Harmonic_params_st *par,
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

  if ((irow < 0) || (irow >= par->n_bos)) {
    return SCAMAC_ERANGE;
  }

  if (irow == 0) {
    scamac_sparserow_real_add(row, 0.0, 0);
    scamac_sparserow_real_add(row, par->lambda, 1);
  } else if (0 < irow && irow < (par->n_bos - 1)) {
    scamac_sparserow_real_add(row, par->lambda * sqrt(1.0 * irow), irow - 1);
    scamac_sparserow_real_add(row, par->omega * irow, irow);
    scamac_sparserow_real_add(row, par->lambda * sqrt(1.0 * (irow + 1)), irow + 1);
  } else if (irow == (par->n_bos - 1)) {
    scamac_sparserow_real_add(row, par->lambda * sqrt(1.0 * irow), irow - 1);
    scamac_sparserow_real_add(row, par->omega * irow, irow);
  } else {
    return SCAMAC_ERANGE;
  }

  return SCAMAC_EOK;
}
