#include <stdlib.h>

#include "scamac_internal.h"
#include "scamac_string.h"
#include "scamac_safeint.h"

#include "scamac_matrix_SpinChainXXZ.h"
#include "scamac_matrix_SpinChainXXZ_inc.c"

ScamacErrorCode scamac_matrix_SpinChainXXZ_tables_create(
    const scamac_matrix_SpinChainXXZ_params_st *par,
    scamac_matrix_SpinChainXXZ_tables_st **tab,
    scamac_info_st *info)
{
  ScamacErrorCode err;

  scamac_matrix_SpinChainXXZ_tables_st *my_tab = malloc(sizeof *my_tab);
  err = scamac_dof_spinssz_alloc(par->n_sites, par->n_up, &(my_tab->dof));
  if (err) {
    return (err | SCAMAC_EINTERNAL);
  }
  my_tab->ns       = scamac_dof_spins_ns(my_tab->dof);
  my_tab->maxnzrow = par->n_sites + 1;
  *tab             = my_tab;

  if (info) {
    info->nrow     = my_tab->ns;
    info->ncol     = my_tab->ns;
    info->maxnzrow = my_tab->maxnzrow;
    info->maxnzcol = my_tab->maxnzrow;
    info->maxnzgen = my_tab->maxnzrow;
    info->maxnz    = scamac_safeidx_mult(my_tab->ns, my_tab->maxnzrow);
    if (info->maxnz < 0) {
      return SCAMAC_EOVERFLOW;
    }
    info->valtype  = SCAMAC_VAL_REAL;
    info->symmetry = SCAMAC_SYMMETRIC;
  }
  return SCAMAC_EOK;
}

ScamacErrorCode scamac_matrix_SpinChainXXZ_tables_destroy(
    scamac_matrix_SpinChainXXZ_tables_st *tab)
{
  if (tab) {
    if (tab->dof) {
      scamac_dof_spins_free(tab->dof);
    }
    free(tab);
  }
  return SCAMAC_EOK;
}

ScamacErrorCode scamac_matrix_SpinChainXXZ_work_alloc(
    const scamac_matrix_SpinChainXXZ_params_st *par,
    const scamac_matrix_SpinChainXXZ_tables_st *tab,
    scamac_matrix_SpinChainXXZ_work_st **ws)
{
  if ((!par) || (!tab) || (!ws)) {
    return SCAMAC_ENULL;
  }

  scamac_matrix_SpinChainXXZ_work_st *my_ws = malloc(sizeof *my_ws);
  if (!my_ws) {
    return SCAMAC_EMALLOCFAIL;
  }

  my_ws->rep     = scamac_rep_spins_alloc(tab->dof);
  my_ws->repinit = scamac_rep_spins_alloc(tab->dof);

  *ws            = my_ws;

  return SCAMAC_EOK;
}

ScamacErrorCode scamac_matrix_SpinChainXXZ_work_free(
    scamac_matrix_SpinChainXXZ_work_st *ws)
{
  if (ws) {
    if (ws->rep) {
      scamac_rep_spins_free(ws->rep);
    }
    if (ws->repinit) {
      scamac_rep_spins_free(ws->repinit);
    }
    free(ws);
  }
  return SCAMAC_EOK;
}

ScamacErrorCode scamac_matrix_SpinChainXXZ_generate_row(
    const scamac_matrix_SpinChainXXZ_params_st *par,
    const scamac_matrix_SpinChainXXZ_tables_st *tab,
    scamac_matrix_SpinChainXXZ_work_st *ws,
    ScamacIdx irow,
    ScamacFlag flag,
    scamac_sparserow_real_st *row)
{
  if (!par || !tab || !ws || !row) {
    return SCAMAC_ENULL;
  }

  if (flag & ~SCAMAC_TRANSPOSE & ~SCAMAC_CONJUGATE & ~SCAMAC_KEEPZEROS) {
    return SCAMAC_EINVALID;
  }
  bool fl_keepzeros = (flag & SCAMAC_KEEPZEROS) != 0;

  if ((irow < 0) || (irow >= tab->ns)) {
    return SCAMAC_ERANGE;
  }

  scamac_spins_decode(tab->dof, irow, ws->repinit);

  if ((par->Jz != 0.0) || fl_keepzeros) {
    int i;
    double val = 0.0;
    for (i = 0; i < par->n_sites - 1; i++) {
      val += scamac_op_spins_SzSz(tab->dof, ws->repinit, i, i + 1);
    }
    if (par->boundary_conditions == option_periodic) {
      val += scamac_op_spins_SzSz(tab->dof, ws->repinit, 0, par->n_sites - 1);
    }
    SCAMAC_APPROVE(scamac_sparserow_real_add(row, par->Jz * val, irow));
  }

  if ((par->Bz != 0.0) || fl_keepzeros) {
    int i;
    double val = 0.0;
    for (i = 0; i < par->n_sites; i++) {
      val += scamac_op_spins_Sz(tab->dof, ws->repinit, i);
    }
    SCAMAC_APPROVE(scamac_sparserow_real_add(row, par->Bz * val, irow));
  }

  if ((par->Jxy != 0.0) || fl_keepzeros) {
    int i;
    for (i = 0; i < par->n_sites - 1; i++) {
      scamac_rep_spins_copy(tab->dof, ws->repinit, ws->rep);
      double val = scamac_op_spins_Sflip(tab->dof, ws->rep, i, i + 1);
      SCAMAC_APPROVE(scamac_sparserow_real_add(
          row, par->Jxy * val, scamac_spins_encode(tab->dof, ws->rep)));
    }
    if (par->boundary_conditions == option_periodic) {
      scamac_rep_spins_copy(tab->dof, ws->repinit, ws->rep);
      double val = scamac_op_spins_Sflip(tab->dof, ws->rep, 0, par->n_sites - 1);
      SCAMAC_APPROVE(scamac_sparserow_real_add(
          row, par->Jxy * val, scamac_spins_encode(tab->dof, ws->rep)));
    }
  }

  return SCAMAC_EOK;
}
