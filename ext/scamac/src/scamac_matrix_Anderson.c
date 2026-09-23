#include <stdlib.h>

#include "scamac_internal.h"
#include "scamac_safeint.h"
#include "scamac_string.h"

#include "scamac_matrix_Anderson.h"
#include "scamac_matrix_Anderson_inc.c"

ScamacErrorCode scamac_matrix_Anderson_tables_create(
    const scamac_matrix_Anderson_params_st *par, void **tab, scamac_info_st *info)
{
  if (!par) {
    return SCAMAC_ENULL;
  }

  if (tab) {
    *tab = NULL;
  }

  if (info) {
    info->nrow = par->Lx;
    info->nrow = scamac_safeidx_mult(info->nrow, par->Ly);
    if (info->nrow < 0) {
      return SCAMAC_EOVERFLOW;
    }
    info->nrow = scamac_safeidx_mult(info->nrow, par->Lz);
    if (info->nrow < 0) {
      return SCAMAC_EOVERFLOW;
    }

    info->ncol     = info->nrow;
    info->maxnzrow = 7;
    info->maxnzcol = info->maxnzrow;

    info->maxnzgen = 7;

    info->maxnz    = scamac_safeidx_mult(info->nrow, info->maxnzrow);
    if (info->maxnz < 0) {
      return SCAMAC_EOVERFLOW;
    }
    info->valtype  = SCAMAC_VAL_REAL;
    info->symmetry = SCAMAC_SYMMETRIC;
  }

  return SCAMAC_EOK;
}

ScamacErrorCode scamac_matrix_Anderson_work_alloc(
    const scamac_matrix_Anderson_params_st *par,
    const void *tab,
    scamac_matrix_Anderson_work_st **ws)
{
  if ((!par) || (!ws)) {
    return SCAMAC_ENULL;
  }
  scamac_matrix_Anderson_work_st *my_ws = malloc(sizeof *my_ws);
  if (!my_ws) {
    return SCAMAC_EMALLOCFAIL;
  }

  ScamacErrorCode err;
  err = scamac_ransrc_alloc(par->seed, &(my_ws->rng));
  if (err) {
    free(my_ws);
    return err;
  }
  *ws = my_ws;

  return SCAMAC_EOK;
}

ScamacErrorCode scamac_matrix_Anderson_work_free(scamac_matrix_Anderson_work_st *ws)
{
  if (ws) {
    scamac_ransrc_free(ws->rng);
    free(ws);
  }

  return SCAMAC_EOK;
}

static ScamacIdx back(ScamacIdx L, ScamacIdx i)
{
  if (i & 1) {
    return L - (i + 1) / 2;
  } else {
    return i / 2;
  }
}

static ScamacIdx forth(ScamacIdx L, ScamacIdx i)
{
  if (i < (L + 1) / 2) {
    return 2 * i;
  } else {
    return 2 * (L - i) - 1;
  }
}

ScamacErrorCode scamac_matrix_Anderson_generate_row(
    const scamac_matrix_Anderson_params_st *par,
    const void *tab,
    scamac_matrix_Anderson_work_st *ws,
    ScamacIdx irow,
    ScamacFlag flag,
    scamac_sparserow_real_st *row)
{

  if (!par || !ws || !row) {
    return SCAMAC_ENULL;
  }

  if (flag & ~SCAMAC_TRANSPOSE & ~SCAMAC_CONJUGATE & ~SCAMAC_KEEPZEROS) {
    return SCAMAC_EINVALID;
  }
  bool fl_keepzeros = (flag & SCAMAC_KEEPZEROS) != 0;

  ScamacIdx ns      = (ScamacIdx)par->Lx * (ScamacIdx)par->Ly * (ScamacIdx)par->Lz;
  if ((irow < 0) || (irow >= ns)) {
    return SCAMAC_ERANGE;
  }

  ScamacIdx dx, dy, dz;
  dx = 1;
  dy = par->Lx;
  dz = par->Lx * par->Ly;

  ScamacIdx xo, yo, zo;
  zo = irow / dz;
  yo = (irow - zo * dz) / dy;
  xo = (irow - zo * dz - yo * dy) / dx;

  ScamacIdx x, y, z;
  if (par->sweep == option_backforth) {
    x = back(par->Lx, xo);
    y = back(par->Ly, yo);
    z = back(par->Lz, zo);
  } else {
    x = xo;
    y = yo;
    z = zo;
  }

  double val = 0.0;
  if (par->ranpot != 0.0) {
    val = scamac_ransrc_double(ws->rng, -par->ranpot, par->ranpot, irow);
  }
  if (val != 0.0 || fl_keepzeros) {
    SCAMAC_APPROVE(scamac_sparserow_real_add(row, val, irow));
  }

  if (par->t != 0.0 || fl_keepzeros) {
    if (par->Lx > 1) {
      ScamacIdx xx, idx;
      if (x < par->Lx - 1 || par->boundary_conditions == option_periodic) {
        xx = (x + 1) % par->Lx;
        if (par->sweep == option_backforth) {
          xx = forth(par->Lx, xx);
        }
        idx = irow + (xx - xo) * dx;
        SCAMAC_APPROVE(scamac_sparserow_real_add(row, -par->t, idx));
      }
      if (x > 0 || par->boundary_conditions == option_periodic) {
        xx = (x - 1 + par->Lx) % par->Lx;
        if (par->sweep == option_backforth) {
          xx = forth(par->Lx, xx);
        }
        idx = irow + (xx - xo) * dx;
        SCAMAC_APPROVE(scamac_sparserow_real_add(row, -par->t, idx));
      }
    }
    if (par->Ly > 1) {
      ScamacIdx yy, idx;
      if (y < par->Ly - 1 || par->boundary_conditions == option_periodic) {
        yy = (y + 1) % par->Ly;
        if (par->sweep == option_backforth) {
          yy = forth(par->Ly, yy);
        }
        idx = irow + (yy - yo) * dy;
        SCAMAC_APPROVE(scamac_sparserow_real_add(row, -par->t, idx));
      }
      if (y > 0 || par->boundary_conditions == option_periodic) {
        yy = (y - 1 + par->Ly) % par->Ly;
        if (par->sweep == option_backforth) {
          yy = forth(par->Ly, yy);
        }
        idx = irow + (yy - yo) * dy;
        SCAMAC_APPROVE(scamac_sparserow_real_add(row, -par->t, idx));
      }
    }
    if (par->Lz > 1) {
      ScamacIdx zz, idx;
      if (z < par->Lz - 1 || par->boundary_conditions == option_periodic) {
        zz = (z + 1) % par->Lz;
        if (par->sweep == option_backforth) {
          zz = forth(par->Lz, zz);
        }
        idx = irow + (zz - zo) * dz;
        SCAMAC_APPROVE(scamac_sparserow_real_add(row, -par->t, idx));
      }
      if (z > 0 || par->boundary_conditions == option_periodic) {
        zz = (z - 1 + par->Lz) % par->Lz;
        if (par->sweep == option_backforth) {
          zz = forth(par->Lz, zz);
        }
        idx = irow + (zz - zo) * dz;
        SCAMAC_APPROVE(scamac_sparserow_real_add(row, -par->t, idx));
      }
    }
  }

  return SCAMAC_EOK;
}

ScamacErrorCode scamac_matrix_Anderson_coord(const scamac_matrix_Anderson_params_st *par,
    const void *tab,
    scamac_matrix_Anderson_work_st *ws,
    ScamacIdx idx,
    double *x,
    double *y,
    double *z,
    int *ilayer,
    ScamacIdx *iblock)
{
  if (idx == -1) {
    *ilayer = 3;
  } else if (idx >= 0) {
    ScamacIdx dx, dy, dz;
    dx = 1;
    dy = par->Lx;
    dz = par->Lx * par->Ly;
    ScamacIdx xo, yo, zo;
    zo      = idx / dz;
    yo      = (idx - zo * dz) / dy;
    xo      = (idx - zo * dz - yo * dy) / dx;
    *x      = xo;
    *y      = yo;
    *z      = zo;
    *ilayer = 0;
    *iblock = 0;
  }
  return SCAMAC_EOK;
}
