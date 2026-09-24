#include <stdlib.h>
#include <complex.h>
#include <math.h>

#include "scamac_internal.h"
#include "scamac_safeint.h"
#include "scamac_string.h"

#include "scamac_matrix_Exciton.h"
#include "scamac_matrix_Exciton_inc.c"

ScamacErrorCode scamac_matrix_Exciton_tables_create(
    const scamac_matrix_Exciton_params_st *par,
    scamac_matrix_Exciton_tables_st **tab,
    scamac_info_st *info)
{
  if (!par || !tab) {
    return SCAMAC_ENULL;
  }

  scamac_matrix_Exciton_tables_st *my_tab = malloc(sizeof *my_tab);
  if (!my_tab) {
    return SCAMAC_EMALLOCFAIL;
  }

  ScamacIdx ns;
  ns = 3;
  ns = scamac_safeidx_mult(ns, 2 * par->L + 1);
  if (info->nrow < 0) {
    return SCAMAC_EOVERFLOW;
  }
  ns = scamac_safeidx_mult(ns, 2 * par->L + 1);
  if (info->nrow < 0) {
    return SCAMAC_EOVERFLOW;
  }
  ns = scamac_safeidx_mult(ns, 2 * par->L + 1);
  if (info->nrow < 0) {
    return SCAMAC_EOVERFLOW;
  }

  my_tab->ns       = ns;
  my_tab->maxnzrow = 19;

  my_tab->par_t1   = 0.0381 / (par->a * par->a * par->mlh);
  my_tab->par_t2   = 0.0381 / (par->a * par->a * par->mhh);
  my_tab->par_te   = 0.0381 / (par->a * par->a * par->me);

  my_tab->eikx     = cos(par->kx) + sin(par->kx) * I;
  my_tab->eiky     = cos(par->ky) + sin(par->ky) * I;
  my_tab->eikz     = cos(par->kz) + sin(par->kz) * I;

  my_tab->dx       = 3;
  my_tab->dy       = 3 * (2 * par->L + 1);
  my_tab->dz       = 3 * (2 * par->L + 1) * (2 * par->L + 1);

  int i, j;
  for (i = 0; i < 3; i++) {
    for (j = 0; j < 3; j++) {
      my_tab->mat_so[i][j] = 0.0;
      my_tab->mat_ex[i][j] = 0.0;
    }
  }

  my_tab->mat_so[1][0] = -(1.0 / 3.0) * par->so / 1000.0 * I;
  my_tab->mat_so[2][0] = (1.0 / 3.0) * par->so / 1000.0;
  my_tab->mat_so[0][1] = (1.0 / 3.0) * par->so / 1000.0 * I;
  my_tab->mat_so[2][1] = -(1.0 / 3.0) * par->so / 1000.0 * I;
  my_tab->mat_so[0][2] = (1.0 / 3.0) * par->so / 1000.0;
  my_tab->mat_so[1][2] = (1.0 / 3.0) * par->so / 1000.0 * I;

  if (par->symm == option_para) {
    my_tab->mat_ex[2][2] = 0.0;
  } else if (par->symm == option_ortho) {
    my_tab->mat_ex[0][0] = par->ex / 1000.0;
  } else {
    return SCAMAC_EINVALID | 1 << SCAMAC_ESHIFT;
  }

  *tab = my_tab;

  if (info) {
    info->nrow     = my_tab->ns;
    info->ncol     = my_tab->ns;
    info->maxnzrow = my_tab->maxnzrow;
    info->maxnzcol = my_tab->maxnzrow;
    info->maxnzgen = my_tab->maxnzrow;
    info->maxnz    = scamac_safeidx_mult(info->nrow, info->maxnzrow);
    if (info->maxnz < 0) {
      return SCAMAC_EOVERFLOW;
    }
    info->valtype  = SCAMAC_VAL_COMPLEX;
    info->symmetry = SCAMAC_HERMITIAN;
  }

  return SCAMAC_EOK;
}

ScamacErrorCode scamac_matrix_Exciton_tables_destroy(scamac_matrix_Exciton_tables_st *tab)
{
  if (tab) {
    free(tab);
  }

  return SCAMAC_EOK;
}

ScamacErrorCode scamac_matrix_Exciton_generate_row(
    const scamac_matrix_Exciton_params_st *par,
    const scamac_matrix_Exciton_tables_st *tab,
    void *ws,
    ScamacIdx irow,
    ScamacFlag flag,
    scamac_sparserow_cplx_st *row)
{
  if (!par || !tab || !row) {
    return SCAMAC_ENULL;
  }

  if (flag & ~SCAMAC_TRANSPOSE & ~SCAMAC_CONJUGATE & ~SCAMAC_KEEPZEROS) {
    return SCAMAC_EINVALID;
  }
  bool fl_transpose = (flag & SCAMAC_TRANSPOSE) != 0;
  bool fl_conjugate = (flag & SCAMAC_CONJUGATE) != 0;
  bool fl_keepzeros = (flag & SCAMAC_KEEPZEROS) != 0;
  if (fl_transpose == fl_conjugate) {
    fl_transpose = false;
    fl_conjugate = false;
  }

  if ((irow < 0) || (irow >= tab->ns)) {
    return SCAMAC_ERANGE;
  }

  ScamacErrorCode err;

  const ScamacIdx dx = tab->dx, dy = tab->dy, dz = tab->dz;
  ScamacIdx x, y, z, locdof;
  z      = irow / dz;
  y      = (irow - z * dz) / dy;
  x      = (irow - z * dz - y * dy) / dx;
  locdof = irow - z * dz - y * dy - x * dx;

  x      = x - par->L;
  y      = y - par->L;
  z      = z - par->L;

  double complex hop_x, hop_y, hop_z;
  if (locdof == 0) {
    hop_x = tab->par_t1 + tab->par_te * tab->eikx;
    hop_y = tab->par_t2 + tab->par_te * tab->eiky;
    hop_z = tab->par_t2 + tab->par_te * tab->eikz;
  } else if (locdof == 1) {
    hop_x = tab->par_t2 + tab->par_te * tab->eikx;
    hop_y = tab->par_t1 + tab->par_te * tab->eiky;
    hop_z = tab->par_t2 + tab->par_te * tab->eikz;
  } else {
    hop_x = tab->par_t2 + tab->par_te * tab->eikx;
    hop_y = tab->par_t2 + tab->par_te * tab->eiky;
    hop_z = tab->par_t1 + tab->par_te * tab->eikz;
  }
  hop_x = -hop_x;
  hop_y = -hop_y;
  hop_z = -hop_z;
  if (fl_conjugate) {
    hop_x = conj(hop_x);
    hop_y = conj(hop_y);
    hop_z = conj(hop_z);
  }

  if (x < par->L) {
    err = scamac_sparserow_cplx_add(row, hop_x, irow + dx);
    if (err) {
      return err;
    }
  }
  if (x > -par->L) {
    err = scamac_sparserow_cplx_add(row, conj(hop_x), irow - dx);
    if (err) {
      return err;
    }
  }
  if (y < par->L) {
    err = scamac_sparserow_cplx_add(row, hop_y, irow + dy);
    if (err) {
      return err;
    }
  }
  if (y > -par->L) {
    err = scamac_sparserow_cplx_add(row, conj(hop_y), irow - dy);
    if (err) {
      return err;
    }
  }
  if (z < par->L) {
    err = scamac_sparserow_cplx_add(row, hop_z, irow + dz);
    if (err) {
      return err;
    }
  }
  if (z > -par->L) {
    err = scamac_sparserow_cplx_add(row, conj(hop_z), irow - dz);
    if (err) {
      return err;
    }
  }

  int iloc;
  for (iloc = 0; iloc < 3; iloc++) {
    double complex hp = 0.0;

    if (iloc == locdof) {
      hp = 2.0 * (tab->par_t1 + 2.0 * tab->par_t2) + 6.0 * tab->par_te +
           (par->so / 1000.0) / 3.0 * 2.0;
    }

    hp = hp + tab->mat_so[iloc][locdof];
    if ((x == 0) && (y == 0) && (z == 0)) {
      hp = hp + tab->mat_ex[locdof][iloc];
    }
    if (iloc == locdof) {
      double Coul;
      if ((x == 0) && (y == 0) && (z == 0)) {
        Coul = -1.4400 / (par->lc * par->a);
      } else {
        double r2 = (double)x * x + y * y + z * z;
        Coul      = -1.4400 / (par->eps * par->a * sqrt(r2));
      }
      hp = hp + Coul;
    }
    if (hp != 0.0 || fl_keepzeros) {
      if (fl_conjugate) {
        hp = conj(hp);
      }
      err = scamac_sparserow_cplx_add(row, hp, irow + (iloc - locdof));
      if (err) {
        return err;
      }
    }
  }

  return SCAMAC_EOK;
}
