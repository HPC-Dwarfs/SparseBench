#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "scamac_lut.h"
#include "scamac_dof_spins.h"

ScamacErrorCode scamac_dof_spins_alloc(int n_sites, scamac_dof_spins_st **dof)
{
  if (!dof) {
    return SCAMAC_ENULL;
  }
  if (n_sites <= 0) {
    return SCAMAC_ERANGE;
  }
  if (n_sites > SCAMACHUGEINT) {
    return SCAMAC_EHUGEINT;
  }

  scamac_dof_spins_st *mydof = malloc(sizeof *mydof);
  if (!mydof) {
    return SCAMAC_EMALLOCFAIL;
  }

  mydof->n_sites = n_sites;
  mydof->n_up    = -1;

  mydof->ns      = 1U << n_sites;
  mydof->cnt     = NULL;

  *dof           = mydof;
  return SCAMAC_EOK;
}

ScamacErrorCode scamac_dof_spinssz_alloc(int n_sites, int n_up, scamac_dof_spins_st **dof)
{
  if (!dof) {
    return SCAMAC_ENULL;
  }
  if (n_sites <= 0) {
    return SCAMAC_ERANGE;
  }
  if (n_sites > SCAMACHUGEINT) {
    return SCAMAC_EHUGEINT;
  }
  if ((n_up < 0) || (n_up > n_sites)) {
    return SCAMAC_EINVALID;
  }

  scamac_dof_spins_st *mydof = malloc(sizeof *mydof);
  if (!mydof) {
    return SCAMAC_EMALLOCFAIL;
  }

  mydof->n_sites = n_sites;
  mydof->n_up    = n_up;

  ScamacErrorCode err;
  err = scamac_lut_onezero_construct(n_sites, n_up, &(mydof->ns), &(mydof->cnt));
  if (err) {
    return err;
  }

  *dof = mydof;
  return SCAMAC_EOK;
}

void scamac_dof_spins_free(scamac_dof_spins_st *dof)
{
  if (dof) {
    if (dof->cnt) {
      free(dof->cnt);
    }
    free(dof);
  }
}

ScamacIdx scamac_dof_spins_ns(const scamac_dof_spins_st *dof)
{
  return dof->ns;
}

scamac_rep_spins_st *scamac_rep_spins_alloc(const scamac_dof_spins_st *dof)
{
  scamac_rep_spins_st *myrep = malloc(dof->n_sites * sizeof *myrep);
  return myrep;
}

void scamac_rep_spins_free(scamac_rep_spins_st *rep)
{
  if (rep) {
    free(rep);
  }
}

void scamac_rep_spins_copy(const scamac_dof_spins_st *dof,
    const scamac_rep_spins_st *rep,
    scamac_rep_spins_st *repcpy)
{
  if (rep && repcpy) {
    memcpy(repcpy, rep, dof->n_sites * sizeof *rep);
  }
}

int scamac_spins_decode(
    const scamac_dof_spins_st *dof, ScamacIdx idx, scamac_rep_spins_st *rep)
{
  if (0 <= idx && idx < dof->ns) {
    if (dof->n_up < 0) {
      int i;
      for (i = 0; i < dof->n_sites; i++) {
        if (idx & 1) {
          rep[dof->n_sites - i - 1] = 1;
        } else {
          rep[dof->n_sites - i - 1] = 0;
        }
        idx = idx >> 1;
      }
    } else {
      scamac_lut_onezero_decode(dof->n_sites, dof->n_up, dof->cnt, idx, rep);
    }
    return 0;
  } else {
    return SCAMAC_ERANGE;
  }
}

ScamacIdx scamac_spins_encode(
    const scamac_dof_spins_st *dof, const scamac_rep_spins_st *rep)
{
  if (dof->n_up < 0) {
    ScamacIdx idx = 0;
    int i;
    for (i = 0; i < dof->n_sites; i++) {
      idx = idx << 1;
      if (rep[i]) {
        idx++;
      }
    }
    return idx;
  } else {
    return scamac_lut_onezero_encode(dof->n_sites, dof->n_up, dof->cnt, rep);
  }
}

double scamac_op_spins_Sz(
    const scamac_dof_spins_st *dof, const scamac_rep_spins_st *rep, int i)
{
  if (0 <= i && i < dof->n_sites) {
    if (rep[i]) {
      return 0.5;
    } else {
      return -0.5;
    }
  } else {
    return 0.0;
  }
}

double scamac_op_spins_SzSz(
    const scamac_dof_spins_st *dof, const scamac_rep_spins_st *rep, int i, int j)
{
  if (0 <= i && i < dof->n_sites && 0 <= j && j < dof->n_sites) {
    if (rep[i] == rep[j]) {
      return 0.25;
    } else {
      return -0.25;
    }
  } else {
    return 0.0;
  }
}

double scamac_op_spins_SzSum(
    const scamac_dof_spins_st *dof, const scamac_rep_spins_st *rep)
{
  if (dof->n_up >= 0) {
    return ((double)dof->n_up - 0.5 * dof->n_sites);
  } else {
    int i;
    double val = 0.0;
    for (i = 0; i < dof->n_sites; i++) {
      if (rep[i]) {
        val = val + 0.5;
      } else {
        val = val - 0.5;
      }
    }
    return val;
  }
}

double scamac_op_spins_Sp(const scamac_dof_spins_st *dof, scamac_rep_spins_st *rep, int i)
{
  assert(dof->n_up < 0);
  if (0 <= i && i < dof->n_sites) {
    if (rep[i]) {
      return 0.0;
    } else {
      rep[i] = 1;
      return 1.0;
    }
  } else {
    return 0.0;
  }
}

double scamac_op_spins_Sm(const scamac_dof_spins_st *dof, scamac_rep_spins_st *rep, int i)
{
  assert(dof->n_up < 0);
  if (0 <= i && i < dof->n_sites) {
    if (rep[i]) {
      rep[i] = 0;
      return 1.0;
    } else {
      return 0.0;
    }
  } else {
    return 0.0;
  }
}

double scamac_op_spins_Sx(const scamac_dof_spins_st *dof, scamac_rep_spins_st *rep, int i)
{
  assert(dof->n_up < 0);
  if (0 <= i && i < dof->n_sites) {
    rep[i] = 1 - rep[i];
    return 0.5;
  } else {
    return 0.0;
  }
}

double scamac_op_spins_SpSp(
    const scamac_dof_spins_st *dof, scamac_rep_spins_st *rep, int i, int j)
{
  assert(dof->n_up < 0);
  if (i == j) {
    return 0.0;
  }
  if (0 <= i && i < dof->n_sites && 0 <= j && j < dof->n_sites) {
    if (rep[i] || rep[j]) {
      return 0.0;
    } else {
      rep[i] = 1;
      rep[j] = 1;
      return 1.0;
    }
  } else {
    return 0.0;
  }
}

double scamac_op_spins_SpSm(
    const scamac_dof_spins_st *dof, scamac_rep_spins_st *rep, int i, int j)
{
  if (0 <= i && i < dof->n_sites && 0 <= j && j < dof->n_sites) {
    if (i == j) {
      if (rep[i]) {
        return 1.0;
      } else {
        return 0.0;
      }
    } else {
      if (!rep[i] && rep[j]) {
        rep[i] = 1;
        rep[j] = 0;
        return 1.0;
      } else {
        return 0.0;
      }
    }
  } else {
    return 0.0;
  }
}

double scamac_op_spins_SmSm(
    const scamac_dof_spins_st *dof, scamac_rep_spins_st *rep, int i, int j)
{
  assert(dof->n_up < 0);
  if (i == j) {
    return 0.0;
  }
  if (0 <= i && i < dof->n_sites && 0 <= j && j < dof->n_sites) {
    if (rep[i] && rep[j]) {
      rep[i] = 0;
      rep[j] = 0;
      return 0.0;
    } else {
      return 1.0;
    }
  } else {
    return 0.0;
  }
}

double scamac_op_spins_SxSx(
    const scamac_dof_spins_st *dof, scamac_rep_spins_st *rep, int i, int j)
{
  assert(dof->n_up < 0);
  if (i == j) {
    return 0.25;
  }
  if (0 <= i && i < dof->n_sites && 0 <= j && j < dof->n_sites) {
    rep[i] = 1 - rep[i];
    rep[j] = 1 - rep[j];
    return 0.25;
  } else {
    return 0.0;
  }
}

double scamac_op_spins_Sflip(
    const scamac_dof_spins_st *dof, scamac_rep_spins_st *rep, int i, int j)
{
  if (i == j) {
    return 0.0;
  }
  if (0 <= i && i < dof->n_sites && 0 <= j && j < dof->n_sites) {
    if (rep[i] != rep[j]) {
      rep[i] = 1 - rep[i];
      rep[j] = 1 - rep[j];
      return 0.5;
    } else {
      return 0.0;
    }
  } else {
    return 0.0;
  }
}
