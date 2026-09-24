#include <stdlib.h>
#include <math.h>

#include "scamac_internal.h"
#include "scamac_safeint.h"
#include "scamac_string.h"

#include "scamac_matrix_Membrane1.h"
#include "scamac_matrix_Membrane1_inc.c"

ScamacErrorCode scamac_matrix_Membrane1_tables_create(
    const scamac_matrix_Membrane1_params_st *par, void **tab, scamac_info_st *info)
{
  if (!par) {
    return SCAMAC_ENULL;
  }

  if (tab) {
    *tab = NULL;
  }

  if (info) {
    info->nrow     = 2 * par->nx * par->ny;
    info->ncol     = info->nrow;
    info->maxnzrow = 7;
    info->maxnzcol = 7;
    info->maxnzgen = 7;

    info->maxnz    = scamac_safeidx_mult(info->nrow, info->maxnzrow);
    if (info->maxnz < 0) {
      return SCAMAC_EOVERFLOW;
    }
    info->valtype  = SCAMAC_VAL_REAL;
    info->symmetry = SCAMAC_GENERAL;
  }

  return SCAMAC_EOK;
}

ScamacErrorCode scamac_matrix_Membrane1_generate_row(
    const scamac_matrix_Membrane1_params_st *par,
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
  bool fl_transpose = (flag & SCAMAC_TRANSPOSE) != 0;

  if ((irow < 0) || (irow >= 2 * par->nx * par->ny)) {
    return SCAMAC_ERANGE;
  }

  double hx = 1.0 / pow(par->lx / par->nx, 2);
  double hy = 1.0 / pow(par->ly / par->ny, 2);
  double kf = par->rho * par->cwave * par->cwave;

  ScamacIdx dloc, dx, dy;
  ScamacIdx iloc, ix, iy;
  if (par->pack == option_tight) {
    dloc = 1;
    dx   = 2;
    dy   = 2 * par->nx;
    iy   = irow / dy;
    ix   = (irow - iy * dy) / dx;
    iloc = (irow - iy * dy - ix * dx) / dloc;
  } else {
    dloc = par->nx * par->ny;
    dx   = 1;
    dy   = par->nx;
    iloc = irow / dloc;
    iy   = (irow - iloc * dloc) / dy;
    ix   = (irow - iloc * dloc - iy * dy) / dx;
  }

  if (!fl_transpose) {
    if (iloc == 0) {
      scamac_sparserow_real_add(row, 1.0 / par->rho, irow + dloc);
    } else {
      if (ix > 0) {
        scamac_sparserow_real_add(row, kf * hx, irow - dx - dloc);
      }
      if (ix + 1 < par->nx) {
        scamac_sparserow_real_add(row, kf * hx, irow + dx - dloc);
      }
      if (iy > 0) {
        scamac_sparserow_real_add(row, kf * hy, irow - dy - dloc);
      }
      if (iy + 1 < par->ny) {
        scamac_sparserow_real_add(row, kf * hy, irow + dy - dloc);
      }
      scamac_sparserow_real_add(row, -2.0 * kf * (hx + hy), irow - dloc);
      scamac_sparserow_real_add(row, -par->sigma, irow);
    }
  } else {
    if (iloc == 1) {
      scamac_sparserow_real_add(row, 1.0 / par->rho, irow - dloc);
      scamac_sparserow_real_add(row, -par->sigma, irow);
    } else {
      if (ix > 0) {
        scamac_sparserow_real_add(row, kf * hx, irow - dx + dloc);
      }
      if (ix + 1 < par->nx) {
        scamac_sparserow_real_add(row, kf * hx, irow + dx + dloc);
      }
      if (iy > 0) {
        scamac_sparserow_real_add(row, kf * hy, irow - dy + dloc);
      }
      if (iy + 1 < par->ny) {
        scamac_sparserow_real_add(row, kf * hy, irow + dy + dloc);
      }
      scamac_sparserow_real_add(row, -2.0 * kf * (hx + hy), irow + dloc);
    }
  }

  return SCAMAC_EOK;
}
