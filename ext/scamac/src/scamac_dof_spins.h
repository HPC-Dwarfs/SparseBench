/** \file
 *  \author Andreas Alvermann (University of Greifswald)
 *  \date   October 2017 --- today
 *  \brief  ...
 *  \ingroup internal
 */

#ifndef SCAMAC_DOF_SPINS_H
#define SCAMAC_DOF_SPINS_H

#include "scamac_error.h"

/* quantum mechanical degree of freedom: spins 1/2 */
/* either with J_z = const., or without J_z constraint */

typedef struct {
  /* number of sites */
  int n_sites;
  /* number of "up" spins
     negative value indicates "no constraint" */
  int n_up;

  /* number of states */
  ScamacIdx ns;

  /* count information */
  ScamacIdx *cnt;

} scamac_dof_spins_st;

typedef int scamac_rep_spins_st;

ScamacErrorCode scamac_dof_spins_alloc(int n_sites, scamac_dof_spins_st **dof);
ScamacErrorCode scamac_dof_spinssz_alloc(
    int n_sites, int n_up, scamac_dof_spins_st **dof);
void scamac_dof_spins_free(scamac_dof_spins_st *dof);

ScamacIdx scamac_dof_spins_ns(const scamac_dof_spins_st *dof);

scamac_rep_spins_st *scamac_rep_spins_alloc(const scamac_dof_spins_st *dof);
void scamac_rep_spins_free(scamac_rep_spins_st *rep);

void scamac_rep_spins_copy(const scamac_dof_spins_st *dof,
    const scamac_rep_spins_st *rep,
    scamac_rep_spins_st *repcpy);

int scamac_spins_decode(
    const scamac_dof_spins_st *dof, ScamacIdx idx, scamac_rep_spins_st *rep);
ScamacIdx scamac_spins_encode(
    const scamac_dof_spins_st *dof, const scamac_rep_spins_st *rep);

/* operators */
double scamac_op_spins_Sz(
    const scamac_dof_spins_st *dof, const scamac_rep_spins_st *rep, int i);
double scamac_op_spins_SzSz(
    const scamac_dof_spins_st *dof, const scamac_rep_spins_st *rep, int i, int j);
double scamac_op_spins_SzSum(
    const scamac_dof_spins_st *dof, const scamac_rep_spins_st *rep);

double scamac_op_spins_Sp(
    const scamac_dof_spins_st *dof, scamac_rep_spins_st *rep, int i);
double scamac_op_spins_Sm(
    const scamac_dof_spins_st *dof, scamac_rep_spins_st *rep, int i);
double scamac_op_spins_Sx(
    const scamac_dof_spins_st *dof, scamac_rep_spins_st *rep, int i);

double scamac_op_spins_SpSp(
    const scamac_dof_spins_st *dof, scamac_rep_spins_st *rep, int i, int j);
double scamac_op_spins_SpSm(
    const scamac_dof_spins_st *dof, scamac_rep_spins_st *rep, int i, int j);
double scamac_op_spins_SmSm(
    const scamac_dof_spins_st *dof, scamac_rep_spins_st *rep, int i, int j);
double scamac_op_spins_SxSx(
    const scamac_dof_spins_st *dof, scamac_rep_spins_st *rep, int i, int j);
/* Sx Sx + Sy Sy = (S+ S- + S- S+)/2 */
double scamac_op_spins_Sflip(
    const scamac_dof_spins_st *dof, scamac_rep_spins_st *rep, int i, int j);

#endif /* SCAMAC_DOF_SPINS_H */
