#include <stdlib.h>
#include <math.h>
#include <complex.h>

#include "scamac_internal.h"
#include "scamac_string.h"
#include "scamac_safeint.h"

#include "scamac_matrix_Tridiagonal.h"
#include "scamac_matrix_Tridiagonal_inc.c"

ScamacErrorCode scamac_matrix_Tridiagonal_tables_create(
    const scamac_matrix_Tridiagonal_params_st *par, void **tab, scamac_info_st *info)
{
  if (info) {
    info->nrow     = par->n;
    info->ncol     = par->n;
    info->maxnzrow = 3;
    info->maxnzcol = 3;
    info->maxnzgen = 3;
    info->maxnz    = scamac_safeidx_mult(info->nrow, info->maxnzrow);
    if (info->maxnz < 0) {
      return SCAMAC_EOVERFLOW;
    }
    info->valtype  = SCAMAC_VAL_COMPLEX;
    info->symmetry = SCAMAC_HERMITIAN;
  }
  return SCAMAC_EOK;
}

ScamacErrorCode scamac_matrix_Tridiagonal_generate_row(
    const scamac_matrix_Tridiagonal_params_st *par,
    const void *tab,
    void *ws,
    ScamacIdx irow,
    ScamacFlag flag,
    scamac_sparserow_cplx_st *row)
{

  if (!par || !row) {
    return SCAMAC_ENULL;
  }

  if (flag & ~SCAMAC_TRANSPOSE & ~SCAMAC_CONJUGATE & ~SCAMAC_KEEPZEROS) {
    return SCAMAC_EINVALID;
  }
  bool fl_transpose = (flag & SCAMAC_TRANSPOSE) != 0;
  bool fl_conjugate = (flag & SCAMAC_CONJUGATE) != 0;

  if ((irow < 0) || (irow >= par->n)) {
    return SCAMAC_ERANGE;
  }

  double complex eiphi = cexp(I * par->phi);
  if (fl_conjugate != fl_transpose) {
    eiphi = conj(eiphi);
  }

  if (irow == 0) {
    scamac_sparserow_cplx_add(row, par->diag, 0);
    scamac_sparserow_cplx_add(row, eiphi * par->offdiag, 1);
  } else if (0 < irow && irow < (par->n - 1)) {
    scamac_sparserow_cplx_add(row, conj(eiphi) * par->offdiag, irow - 1);
    scamac_sparserow_cplx_add(row, par->diag, irow);
    scamac_sparserow_cplx_add(row, eiphi * par->offdiag, irow + 1);
  } else if (irow == (par->n - 1)) {
    scamac_sparserow_cplx_add(row, conj(eiphi) * par->offdiag, irow - 1);
    scamac_sparserow_cplx_add(row, par->diag, irow);
  } else {
    return SCAMAC_ERANGE;
  }

  return SCAMAC_EOK;
}
