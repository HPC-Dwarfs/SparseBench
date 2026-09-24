#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scamac_lut.h"
#include "scamac_dof_fermions.h"

ScamacErrorCode scamac_dof_fermions_alloc(
    int n_sites, int n_fermions, scamac_dof_fermions_st **dof)
{
  if (!dof) {
    return SCAMAC_ENULL;
  }
  if ((n_sites <= 0) || (n_fermions < 0)) {
    return SCAMAC_ERANGE;
  }
  if ((n_sites > SCAMACHUGEINT) || (n_fermions > SCAMACHUGEINT)) {
    return SCAMAC_EHUGEINT;
  }
  if (n_fermions > n_sites) {
    return SCAMAC_EINVALID;
  }

  scamac_dof_fermions_st *mydof = malloc(sizeof *mydof);
  if (!mydof) {
    return SCAMAC_EMALLOCFAIL;
  }

  mydof->n_sites    = n_sites;
  mydof->n_fermions = n_fermions;

  ScamacErrorCode err;
  err = scamac_lut_onezero_construct(n_sites, n_fermions, &(mydof->ns), &(mydof->cnt));
  if (err) {
    return err;
  }

  *dof = mydof;

  return SCAMAC_EOK;
}

void scamac_dof_fermions_free(scamac_dof_fermions_st *dof)
{
  if (dof) {
    free(dof->cnt);
    free(dof);
  }
}

ScamacIdx scamac_dof_fermions_ns(const scamac_dof_fermions_st *dof)
{
  if (dof) {
    return dof->ns;
  } else {
    return 0;
  }
}

scamac_rep_fermions_st *scamac_rep_fermions_alloc(const scamac_dof_fermions_st *dof)
{
  scamac_rep_fermions_st *myrep = malloc(dof->n_sites * sizeof *myrep);
  return myrep;
}

void scamac_rep_fermions_free(scamac_rep_fermions_st *rep)
{
  if (rep) {
    free(rep);
  }
}

void scamac_rep_fermions_copy(const scamac_dof_fermions_st *dof,
    const scamac_rep_fermions_st *rep,
    scamac_rep_fermions_st *repcpy)
{
  if (rep && repcpy) {
    memcpy(repcpy, rep, dof->n_sites * sizeof *rep);
  }
}

ScamacErrorCode scamac_fermions_decode(
    const scamac_dof_fermions_st *dof, ScamacIdx idx, scamac_rep_fermions_st *rep)
{
  if (0 <= idx && idx < dof->ns) {
    scamac_lut_onezero_decode(dof->n_sites, dof->n_fermions, dof->cnt, idx, rep);
    return SCAMAC_EOK;
  } else {
    return SCAMAC_ERANGE;
  }
}

ScamacIdx scamac_fermions_encode(
    const scamac_dof_fermions_st *dof, const scamac_rep_fermions_st *rep)
{
  return scamac_lut_onezero_encode(dof->n_sites, dof->n_fermions, dof->cnt, rep);
}

static int fermions_cdc(int i, const int *x)
{
  return x[i];
}

static int fermions_cdicj(int i, int j, int *x)
{
  if (x[i] == 1 || x[j] == 0) {
    return 0;
  } else {
    int s1, s2;
    s1 = 0;
    s2 = 0;
    int k;
    for (k = 0; k < i; k++) {
      s1 = s1 + x[k];
    }
    x[j] = 0;
    for (k = 0; k < j; k++) {
      s2 = s2 + x[k];
    }
    x[i] = 1;

    if ((s1 * s2) % 2 == 0) {
      return 1;
    } else {
      return -1;
    }
  }
}

static int fermions_nn(int i, int j, const int *x)
{
  if (x[i] && x[j]) {
    return 1;
  } else {
    return 0;
  }
}

static int fermions_hop(int i, int j, int *x)
{
  if (i == j) {
    if (x[i]) {
      return 2;
    } else {
      return 0;
    }
  } else if ((x[i] == 0 && x[j] == 0) || (x[i] == 1 && x[j] == 1)) {
    return 0;
  } else {
    int il, ir;
    if (i < j) {
      il = i;
      ir = j;
    } else {
      il = j;
      ir = i;
    }
    int fsgn, k;
    fsgn = 0;
    for (k = il + 1; k < ir; k++) {
      if (x[k])
        fsgn = 1 - fsgn;
    }
    x[i] = 1 - x[i];
    x[j] = 1 - x[j];
    if (fsgn) {
      return -1;
    } else {
      return 1;
    }
  }
}

double scamac_op_fermions_cdc(
    const scamac_dof_fermions_st *dof, const scamac_rep_fermions_st *rep, int ic)
{
  if (0 <= ic && ic < dof->n_sites) {
    return (double)fermions_cdc(ic, rep);
  } else {
    return 0.0;
  }
}

double scamac_op_fermions_nn(
    const scamac_dof_fermions_st *dof, const scamac_rep_fermions_st *rep, int i, int j)
{
  if (0 <= i && i < dof->n_sites && 0 <= j && j < dof->n_sites) {
    return (double)fermions_nn(i, j, rep);
  } else {
    return 0.0;
  }
}

double scamac_op_fermions_cdicj(
    const scamac_dof_fermions_st *dof, scamac_rep_fermions_st *rep, int i, int j)
{
  if (0 <= i && i < dof->n_sites && 0 <= j && j < dof->n_sites) {
    return (double)fermions_cdicj(i, j, rep);
  } else {
    return 0.0;
  }
}

double scamac_op_fermions_hop(
    const scamac_dof_fermions_st *dof, scamac_rep_fermions_st *rep, int i, int j)
{
  if (0 <= i && i < dof->n_sites && 0 <= j && j < dof->n_sites) {
    return (double)fermions_hop(i, j, rep);
  } else {
    return 0.0;
  }
}
