#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "scamac_lut.h"
#include "scamac_dof_bosons.h"

ScamacErrorCode scamac_dof_bosons_alloc(
    int n_sites, int n_bosons, int n_bosons_per_site, scamac_dof_bosons_st **dof)
{
  if (!dof) {
    return SCAMAC_ENULL;
  }
  if ((n_sites <= 0) || (n_bosons < 0) || (n_bosons_per_site < 0)) {
    return SCAMAC_ERANGE;
  }
  if ((n_sites > SCAMACHUGEINT) || (n_bosons > SCAMACHUGEINT) ||
      (n_bosons_per_site > SCAMACHUGEINT)) {
    return SCAMAC_EHUGEINT;
  }
  if (n_bosons / n_sites > n_bosons_per_site) {
    return SCAMAC_EINVALID;
  }

  scamac_dof_bosons_st *mydof = malloc(sizeof *mydof);
  if (!mydof) {
    return SCAMAC_EMALLOCFAIL;
  }

  mydof->n_sites           = n_sites;
  mydof->n_bosons          = n_bosons;
  mydof->n_bosons_per_site = n_bosons_per_site;

  ScamacErrorCode err;
  err = scamac_lut_construct(
      false, n_sites, n_bosons_per_site, n_bosons, &(mydof->ns), &(mydof->cnt));
  if (err) {
    return err;
  }

  *dof = mydof;

  return SCAMAC_EOK;
}

void scamac_dof_bosons_free(scamac_dof_bosons_st *dof)
{
  if (dof) {
    free(dof->cnt);
    free(dof);
  }
}

ScamacIdx scamac_dof_bosons_ns(const scamac_dof_bosons_st *dof)
{
  if (dof) {
    return dof->ns;
  } else {
    return 0;
  }
}

scamac_rep_bosons_st *scamac_rep_bosons_alloc(const scamac_dof_bosons_st *dof)
{
  scamac_rep_bosons_st *myrep = malloc(dof->n_sites * sizeof *myrep);
  return myrep;
}

void scamac_rep_bosons_free(scamac_rep_bosons_st *rep)
{
  free(rep);
}

void scamac_rep_bosons_copy(const scamac_dof_bosons_st *dof,
    const scamac_rep_bosons_st *rep,
    scamac_rep_bosons_st *repcpy)
{
  if (rep && repcpy) {
    memcpy(repcpy, rep, dof->n_sites * sizeof *rep);
  }
}

ScamacErrorCode scamac_bosons_decode(
    const scamac_dof_bosons_st *dof, ScamacIdx idx, scamac_rep_bosons_st *rep)
{
  if (0 <= idx && idx < dof->ns) {
    scamac_lut_decode(false, dof->n_sites, dof->n_bosons, dof->cnt, idx, rep);
    return SCAMAC_EOK;
  } else {
    return SCAMAC_ERANGE;
  }
}

ScamacIdx scamac_bosons_encode(
    const scamac_dof_bosons_st *dof, const scamac_rep_bosons_st *rep)
{
  int i;
  for (i = 0; i < dof->n_sites; i++) {
    if (rep[i] > dof->n_bosons_per_site) {
      return -1;
    }
  }
  return scamac_lut_encode(false, dof->n_sites, dof->n_bosons, dof->cnt, rep);
}

static int bosons_bdb(int i, const int *x)
{
  return x[i];
}

static double bosons_bdibj(int i, int j, int *x)
{
  if (x[j] == 0) {
    return 0;
  } else {
    double val = sqrt((double)(x[i] + 1) * x[j]);
    x[i]++;
    x[j]--;
    return val;
  }
}

static int bosons_nn(int i, int j, const int *x)
{
  return x[i] * x[j];
}

double scamac_op_bosons_bdb(
    const scamac_dof_bosons_st *dof, const scamac_rep_bosons_st *rep, int ic)
{
  if (0 <= ic && ic < dof->n_sites) {
    return (double)bosons_bdb(ic, rep);
  } else {
    return 0.0;
  }
}

double scamac_op_bosons_nn(
    const scamac_dof_bosons_st *dof, const scamac_rep_bosons_st *rep, int i, int j)
{
  if (0 <= i && i < dof->n_sites && 0 <= j && j < dof->n_sites) {
    return (double)bosons_nn(i, j, rep);
  } else {
    return 0.0;
  }
}

double scamac_op_bosons_bdibj(
    const scamac_dof_bosons_st *dof, scamac_rep_bosons_st *rep, int i, int j)
{
  if (0 <= i && i < dof->n_sites && 0 <= j && j < dof->n_sites) {
    return bosons_bdibj(i, j, rep);
  } else {
    return 0.0;
  }
}
