#include <stdlib.h>

#include "scamac_safeint.h"
#include "scamac_string.h"

#include "scamac_matrix_TightBinding.h"
#include "scamac_matrix_TightBinding_inc.c"

ScamacErrorCode scamac_matrix_TightBinding_tables_create(
    const scamac_matrix_TightBinding_params_st *par,
    scamac_matrix_TightBinding_tables_st **tab,
    scamac_info_st *info)
{
  if (!par || !tab) {
    return SCAMAC_ENULL;
  }

  scamac_matrix_TightBinding_tables_st *my_tab = malloc(sizeof *my_tab);
  if (!my_tab) {
    return SCAMAC_EMALLOCFAIL;
  }

  my_tab->dof.Lx = par->Lx;
  my_tab->dof.Ly = par->Ly;
  my_tab->dof.Lz = par->Lz;
  my_tab->dof.mx = par->mx;
  my_tab->dof.my = par->my;
  my_tab->dof.mz = par->mz;

  my_tab->ns     = scamac_dof_lattice_cubic_ns(&(my_tab->dof));
  if (my_tab->ns < 0) {
    return SCAMAC_EOVERFLOW;
  }

  *tab = my_tab;

  if (info) {
    info->nrow     = my_tab->ns;
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

ScamacErrorCode scamac_matrix_TightBinding_tables_destroy(
    scamac_matrix_TightBinding_tables_st *tab)
{
  if (tab) {
    free(tab);
  }

  return SCAMAC_EOK;
}

ScamacErrorCode scamac_matrix_TightBinding_generate_row(
    const scamac_matrix_TightBinding_params_st *par,
    const scamac_matrix_TightBinding_tables_st *tab,
    void *ws,
    ScamacIdx irow,
    ScamacFlag flag,
    scamac_sparserow_real_st *row)
{

  if (!par || !tab || !row) {
    return SCAMAC_ENULL;
  }

  if (flag & ~SCAMAC_TRANSPOSE & ~SCAMAC_CONJUGATE & ~SCAMAC_KEEPZEROS) {
    return SCAMAC_EINVALID;
  }
  bool fl_keepzeros = (flag & SCAMAC_KEEPZEROS) != 0;

  if ((irow < 0) || (irow >= tab->ns)) {
    return SCAMAC_ERANGE;
  }

  ScamacIdx x, y, z;
  scamac_dof_lattice_cubic_decode(&(tab->dof), irow, &x, &y, &z);

  if (par->t != 0.0 || fl_keepzeros) {
    if (par->Lx > 1) {
      if (x < par->Lx - 1 || par->boundary_conditions == option_periodic) {
        SCAMAC_APPROVE(scamac_sparserow_real_add(row,
            -par->t,
            scamac_dof_lattice_cubic_encode(&(tab->dof), (x + 1) % par->Lx, y, z)));
      }
      if (x > 0 || par->boundary_conditions == option_periodic) {
        SCAMAC_APPROVE(scamac_sparserow_real_add(row,
            -par->t,
            scamac_dof_lattice_cubic_encode(
                &(tab->dof), (x - 1 + par->Lx) % par->Lx, y, z)));
      }
    }
    if (par->Ly > 1) {
      if (y < par->Ly - 1 || par->boundary_conditions == option_periodic) {
        SCAMAC_APPROVE(scamac_sparserow_real_add(row,
            -par->t,
            scamac_dof_lattice_cubic_encode(&(tab->dof), x, (y + 1) % par->Ly, z)));
      }
      if (y > 0 || par->boundary_conditions == option_periodic) {
        SCAMAC_APPROVE(scamac_sparserow_real_add(row,
            -par->t,
            scamac_dof_lattice_cubic_encode(
                &(tab->dof), x, (y - 1 + par->Ly) % par->Ly, z)));
      }
    }
    if (par->Lz > 1) {
      if (z < par->Lz - 1 || par->boundary_conditions == option_periodic) {
        SCAMAC_APPROVE(scamac_sparserow_real_add(row,
            -par->t,
            scamac_dof_lattice_cubic_encode(&(tab->dof), x, y, (z + 1) % par->Lz)));
      }
      if (z > 0 || par->boundary_conditions == option_periodic) {
        SCAMAC_APPROVE(scamac_sparserow_real_add(row,
            -par->t,
            scamac_dof_lattice_cubic_encode(
                &(tab->dof), x, y, (z - 1 + par->Lz) % par->Lz)));
      }
    }
  }

  return SCAMAC_EOK;
}
