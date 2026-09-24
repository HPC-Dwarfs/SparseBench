#include <stdlib.h>

#include "scamac_internal.h"
#include "scamac_safeint.h"
#include "scamac_string.h"

#include "scamac_matrix_FreeBosonChain.h"
#include "scamac_matrix_FreeBosonChain_inc.c"

ScamacErrorCode scamac_matrix_FreeBosonChain_tables_create(
    const scamac_matrix_FreeBosonChain_params_st *par,
    scamac_matrix_FreeBosonChain_tables_st **tab,
    scamac_info_st *info)
{
  ScamacErrorCode err;

  if (!par || !tab) {
    return SCAMAC_ENULL;
  }

  scamac_matrix_FreeBosonChain_tables_st *my_tab = malloc(sizeof *my_tab);
  if (!my_tab) {
    return SCAMAC_EMALLOCFAIL;
  }

  my_tab->ndof = par->n_species;

  err          = scamac_multidx_alloc(par->n_species, &(my_tab->midx));
  if (err) {
    return (err | SCAMAC_EINTERNAL);
  }

  my_tab->dof = malloc(par->n_species * sizeof *(my_tab->dof));

  int k;
  for (k = 0; k < par->n_species; k++) {
    err = scamac_dof_bosons_alloc(
        par->n_sites, par->n_bosons, par->n_bosons, &(my_tab->dof[k]));
    if (err) {
      return err;
    }
    err = scamac_multidx_set(my_tab->midx, k, scamac_dof_bosons_ns(my_tab->dof[k]));
    if (err) {
      return err;
    }
  }
  my_tab->ns       = scamac_multidx_nidx(my_tab->midx);
  my_tab->maxnzrow = 2 * par->n_species * par->n_sites;

  *tab             = my_tab;

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
    info->valtype  = SCAMAC_VAL_REAL;
    info->symmetry = SCAMAC_SYMMETRIC;
  }

  return SCAMAC_EOK;
}

ScamacErrorCode scamac_matrix_FreeBosonChain_tables_destroy(
    scamac_matrix_FreeBosonChain_tables_st *tab)
{
  if (tab) {
    if (tab->midx) {
      scamac_multidx_free(tab->midx);
    }
    if (tab->dof) {
      int k;
      for (k = 0; k < tab->ndof; k++) {
        if (tab->dof[k]) {
          scamac_dof_bosons_free(tab->dof[k]);
        }
      }
      free(tab->dof);
    }
    free(tab);
  }

  return SCAMAC_EOK;
}

ScamacErrorCode scamac_matrix_FreeBosonChain_work_alloc(
    const scamac_matrix_FreeBosonChain_params_st *par,
    const scamac_matrix_FreeBosonChain_tables_st *tab,
    scamac_matrix_FreeBosonChain_work_st **ws)
{

  if ((!par) || (!tab) || (!ws)) {
    return SCAMAC_ENULL;
  }

  scamac_matrix_FreeBosonChain_work_st *my_ws = malloc(sizeof *my_ws);
  if (!my_ws) {
    return SCAMAC_EMALLOCFAIL;
  }

  my_ws->ndof    = par->n_species;
  my_ws->rep     = malloc(par->n_species * sizeof *(my_ws->rep));
  my_ws->repinit = malloc(par->n_species * sizeof *(my_ws->repinit));
  int k;
  for (k = 0; k < par->n_species; k++) {
    my_ws->rep[k]     = scamac_rep_bosons_alloc(tab->dof[k]);
    my_ws->repinit[k] = scamac_rep_bosons_alloc(tab->dof[k]);
  }

  *ws = my_ws;

  return SCAMAC_EOK;
}

ScamacErrorCode scamac_matrix_FreeBosonChain_work_free(
    scamac_matrix_FreeBosonChain_work_st *ws)
{
  if (ws) {
    if (ws->rep) {
      int k;
      for (k = 0; k < ws->ndof; k++) {
        if (ws->rep[k]) {
          scamac_rep_bosons_free(ws->rep[k]);
        }
      }
      free(ws->rep);
    }
    if (ws->repinit) {
      int k;
      for (k = 0; k < ws->ndof; k++) {
        if (ws->rep[k]) {
          scamac_rep_bosons_free(ws->repinit[k]);
        }
      }
      free(ws->repinit);
    }
    free(ws);
  }

  return SCAMAC_EOK;
}

ScamacErrorCode scamac_matrix_FreeBosonChain_generate_row(
    const scamac_matrix_FreeBosonChain_params_st *par,
    const scamac_matrix_FreeBosonChain_tables_st *tab,
    scamac_matrix_FreeBosonChain_work_st *ws,
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

  if ((par->t != 0.0) || fl_keepzeros) {
    int k;
    for (k = 0; k < par->n_species; k++) {
      scamac_bosons_decode(
          tab->dof[k], scamac_multidx_decode(tab->midx, k, irow), ws->repinit[k]);
    }

    for (k = 0; k < par->n_species; k++) {
      int i;
      double val;
      for (i = 0; i < par->n_sites - 1; i++) {
        scamac_rep_bosons_copy(tab->dof[k], ws->repinit[k], ws->rep[k]);
        val = scamac_op_bosons_bdibj(tab->dof[k], ws->rep[k], i, i + 1);
        scamac_sparserow_real_add(row,
            par->t * val,
            scamac_multidx_upd(
                tab->midx, k, scamac_bosons_encode(tab->dof[k], ws->rep[k]), irow));
        scamac_rep_bosons_copy(tab->dof[k], ws->repinit[k], ws->rep[k]);
        val = scamac_op_bosons_bdibj(tab->dof[k], ws->rep[k], i + 1, i);
        SCAMAC_APPROVE(scamac_sparserow_real_add(row,
            par->t * val,
            scamac_multidx_upd(
                tab->midx, k, scamac_bosons_encode(tab->dof[k], ws->rep[k]), irow)));
      }
      if (par->bc == option_periodic) {
        scamac_rep_bosons_copy(tab->dof[k], ws->repinit[k], ws->rep[k]);
        val = scamac_op_bosons_bdibj(tab->dof[k], ws->rep[k], 0, par->n_sites - 1);
        scamac_sparserow_real_add(row,
            par->t * val,
            scamac_multidx_upd(
                tab->midx, k, scamac_bosons_encode(tab->dof[k], ws->rep[k]), irow));
        scamac_rep_bosons_copy(tab->dof[k], ws->repinit[k], ws->rep[k]);
        val = scamac_op_bosons_bdibj(tab->dof[k], ws->rep[k], par->n_sites - 1, 0);
        SCAMAC_APPROVE(scamac_sparserow_real_add(row,
            par->t * val,
            scamac_multidx_upd(
                tab->midx, k, scamac_bosons_encode(tab->dof[k], ws->rep[k]), irow)));
      }
    }
  }

  return SCAMAC_EOK;
}
