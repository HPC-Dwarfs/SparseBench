#include <stdlib.h>

#include "scamac_internal.h"
#include "scamac_safeint.h"
#include "scamac_string.h"

#include "scamac_matrix_Hubbard.h"
#include "scamac_matrix_Hubbard_inc.c"

ScamacErrorCode scamac_matrix_Hubbard_work_alloc(
    const scamac_matrix_Hubbard_params_st *par,
    const scamac_matrix_Hubbard_tables_st *tab,
    scamac_matrix_Hubbard_work_st **ws)
{
  if (!ws) {
    return SCAMAC_ENULL;
  }
  if ((!par) || (!tab)) {
    return SCAMAC_ENULL;
  }

  scamac_matrix_Hubbard_work_st *my_ws = malloc(sizeof *my_ws);

  my_ws->rep                           = malloc(2 * sizeof *(my_ws->rep));
  my_ws->repinit                       = malloc(2 * sizeof *(my_ws->repinit));
  int k;
  for (k = 0; k < 2; k++) {
    my_ws->rep[k]     = scamac_rep_fermions_alloc(tab->dof[k]);
    my_ws->repinit[k] = scamac_rep_fermions_alloc(tab->dof[k]);
  }
  *ws = my_ws;
  return SCAMAC_EOK;
}

ScamacErrorCode scamac_matrix_Hubbard_work_free(scamac_matrix_Hubbard_work_st *ws)
{
  if (ws) {
    if (ws->rep) {
      int k;
      for (k = 0; k < 2; k++) {
        if (ws->rep[k]) {
          scamac_rep_fermions_free(ws->rep[k]);
        }
      }
      free(ws->rep);
    }
    if (ws->repinit) {
      int k;
      for (k = 0; k < 2; k++) {
        if (ws->rep[k]) {
          scamac_rep_fermions_free(ws->repinit[k]);
        }
      }
      free(ws->repinit);
    }
    free(ws);
  }
  return SCAMAC_EOK;
}

ScamacErrorCode scamac_matrix_Hubbard_tables_create(
    const scamac_matrix_Hubbard_params_st *par,
    scamac_matrix_Hubbard_tables_st **tab,
    scamac_info_st *info)
{
  ScamacErrorCode err;

  scamac_matrix_Hubbard_tables_st *my_tab = malloc(sizeof *my_tab);
  err                                     = scamac_multidx_alloc(2, &(my_tab->midx));
  if (err) {
    return (err | SCAMAC_EINTERNAL);
  }

  my_tab->dof = malloc(2 * sizeof *(my_tab->dof));
  my_tab->ns  = 1;
  int k;
  for (k = 0; k < 2; k++) {
    err = scamac_dof_fermions_alloc(par->n_sites, par->n_fermions, &(my_tab->dof[k]));
    if (err) {
      return err;
    }
    err = scamac_multidx_set(my_tab->midx, k, scamac_dof_fermions_ns(my_tab->dof[k]));
    if (err) {
      return err;
    }
  }
  my_tab->ns           = scamac_multidx_nidx(my_tab->midx);
  my_tab->maxrowlength = 2 * par->n_sites + 1;
  my_tab->onsite       = NULL;
  if (par->ranpot > 0.0) {
    my_tab->onsite = malloc(par->n_sites * sizeof *(my_tab->onsite));
    scamac_ranvec_double(
        par->seed, 0, par->n_sites, -par->ranpot, par->ranpot, my_tab->onsite);
  }
  *tab = my_tab;
  if (info) {
    info->nrow     = my_tab->ns;
    info->ncol     = my_tab->ns;
    info->maxnzrow = my_tab->maxrowlength;
    info->maxnzcol = my_tab->maxrowlength;
    info->maxnzgen = my_tab->maxrowlength + 1;
    info->maxnz    = scamac_safeidx_mult(my_tab->ns, my_tab->maxrowlength);
    if (info->maxnz < 0) {
      return SCAMAC_EOVERFLOW;
    }
    info->valtype  = SCAMAC_VAL_REAL;
    info->symmetry = SCAMAC_SYMMETRIC;
  }
  return SCAMAC_EOK;
}

ScamacErrorCode scamac_matrix_Hubbard_tables_destroy(scamac_matrix_Hubbard_tables_st *tab)
{
  if (tab) {
    if (tab->dof) {
      int k;
      for (k = 0; k < 2; k++) {
        if (tab->dof[k]) {
          scamac_dof_fermions_free(tab->dof[k]);
        }
      }
      free(tab->dof);
    }
    if (tab->midx) {
      scamac_multidx_free(tab->midx);
    }
    if (tab->onsite) {
      free(tab->onsite);
    }
    free(tab);
  }
  return SCAMAC_EOK;
}

ScamacErrorCode scamac_matrix_Hubbard_generate_row(
    const scamac_matrix_Hubbard_params_st *par,
    const scamac_matrix_Hubbard_tables_st *tab,
    scamac_matrix_Hubbard_work_st *ws,
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

  int k;
  for (k = 0; k < 2; k++) {
    scamac_fermions_decode(
        tab->dof[k], scamac_multidx_decode(tab->midx, k, irow), ws->repinit[k]);
  }

  if ((par->t != 0.0) || fl_keepzeros) {

    for (k = 0; k < 2; k++) {
      int i;
      for (i = 0; i < par->n_sites - 1; i++) {
        scamac_rep_fermions_copy(tab->dof[k], ws->repinit[k], ws->rep[k]);
        double val = scamac_op_fermions_hop(tab->dof[k], ws->rep[k], i, i + 1);
        if ((val != 0.0) || fl_keepzeros) {
          SCAMAC_APPROVE(scamac_sparserow_real_add(row,
              par->t * val,
              scamac_multidx_upd(
                  tab->midx, k, scamac_fermions_encode(tab->dof[k], ws->rep[k]), irow)));
        }
      }
      if (par->boundary_conditions == option_periodic) {
        scamac_rep_fermions_copy(tab->dof[k], ws->repinit[k], ws->rep[k]);
        double val = scamac_op_fermions_hop(tab->dof[k], ws->rep[k], 0, par->n_sites - 1);
        if ((val != 0.0) || fl_keepzeros) {
          SCAMAC_APPROVE(scamac_sparserow_real_add(row,
              par->t * val,
              scamac_multidx_upd(
                  tab->midx, k, scamac_fermions_encode(tab->dof[k], ws->rep[k]), irow)));
        }
      }
    }
  }

  if ((par->U != 0.0) || fl_keepzeros) {
    double val = 0.0;
    int i;
    for (i = 0; i < par->n_sites; i++) {
      val = val + scamac_op_fermions_cdc(tab->dof[0], ws->repinit[0], i) *
                      scamac_op_fermions_cdc(tab->dof[1], ws->repinit[1], i);
    }
    SCAMAC_APPROVE(scamac_sparserow_real_add(row, par->U * val, irow));
  }

  if (tab->onsite) {
    double val = 0.0;
    int i;
    for (i = 0; i < par->n_sites; i++) {
      val = val +
            tab->onsite[i] * (scamac_op_fermions_cdc(tab->dof[0], ws->repinit[0], i) +
                                 scamac_op_fermions_cdc(tab->dof[1], ws->repinit[1], i));
    }
    SCAMAC_APPROVE(scamac_sparserow_real_add(row, val, irow));
  }

  return SCAMAC_EOK;
}
