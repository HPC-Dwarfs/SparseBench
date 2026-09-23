/** \file
 *  \author Andreas Alvermann (University of Greifswald)
 *  \date   October 2017 --- today
 *  \brief  ...
 *  \ingroup internal
 */

#ifndef SCAMAC_DOF_BOSONS_H
#define SCAMAC_DOF_BOSONS_H

#include "scamac_error.h"

/* quantum mechanical degree of freedom: conserved bosons */

typedef struct {
  /* number of sites */
  int n_sites;
  /* number of bosons */
  int n_bosons;
  /* maximal number of bosons per site */
  int n_bosons_per_site;

  /* number of states */
  ScamacIdx ns;

  /* count information */
  ScamacIdx *cnt;

} scamac_dof_bosons_st;

typedef int scamac_rep_bosons_st;

ScamacErrorCode scamac_dof_bosons_alloc(
    int n_sites, int n_bosons, int n_bosons_per_site, scamac_dof_bosons_st **dof);
void scamac_dof_bosons_free(scamac_dof_bosons_st *dof);

ScamacIdx scamac_dof_bosons_ns(const scamac_dof_bosons_st *dof);

scamac_rep_bosons_st *scamac_rep_bosons_alloc(const scamac_dof_bosons_st *dof);
void scamac_rep_bosons_free(scamac_rep_bosons_st *rep);

void scamac_rep_bosons_copy(const scamac_dof_bosons_st *dof,
    const scamac_rep_bosons_st *rep,
    scamac_rep_bosons_st *repcpy);

ScamacErrorCode scamac_bosons_decode(
    const scamac_dof_bosons_st *dof, ScamacIdx idx, scamac_rep_bosons_st *rep);
ScamacIdx scamac_bosons_encode(
    const scamac_dof_bosons_st *dof, const scamac_rep_bosons_st *rep);

/* operators */
double scamac_op_bosons_bdb(
    const scamac_dof_bosons_st *dof, const scamac_rep_bosons_st *rep, int ic);
double scamac_op_bosons_nn(
    const scamac_dof_bosons_st *dof, const scamac_rep_bosons_st *rep, int i, int j);
double scamac_op_bosons_bdibj(
    const scamac_dof_bosons_st *dof, scamac_rep_bosons_st *rep, int i, int j);

#endif /* SCAMAC_DOF_BOSONS_H */
