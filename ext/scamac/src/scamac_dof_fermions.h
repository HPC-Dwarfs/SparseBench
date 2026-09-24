/** \file
 *  \author Andreas Alvermann (University of Greifswald)
 *  \date   October 2017 --- today
 *  \brief  ...
 *  \ingroup internal
 */

#ifndef SCAMAC_DOF_FERMIONS_H
#define SCAMAC_DOF_FERMIONS_H

#include "scamac_error.h"

/* quantum mechanical degree of freedom: fermions */

typedef struct {
  /* number of sites */
  int n_sites;
  /* number of fermions */
  int n_fermions;

  /* number of states */
  ScamacIdx ns;

  /* count information */
  ScamacIdx *cnt;

} scamac_dof_fermions_st;

//typedef unsigned char * scamac_rep_fermions_st; // "bit" would suffice
typedef int scamac_rep_fermions_st;

ScamacErrorCode scamac_dof_fermions_alloc(
    int n_sites, int n_fermions, scamac_dof_fermions_st **dof);
void scamac_dof_fermions_free(scamac_dof_fermions_st *dof);

ScamacIdx scamac_dof_fermions_ns(const scamac_dof_fermions_st *dof);

scamac_rep_fermions_st *scamac_rep_fermions_alloc(const scamac_dof_fermions_st *dof);
void scamac_rep_fermions_free(scamac_rep_fermions_st *rep);

void scamac_rep_fermions_copy(const scamac_dof_fermions_st *dof,
    const scamac_rep_fermions_st *rep,
    scamac_rep_fermions_st *repcpy);

ScamacErrorCode scamac_fermions_decode(
    const scamac_dof_fermions_st *dof, ScamacIdx idx, scamac_rep_fermions_st *rep);
ScamacIdx scamac_fermions_encode(
    const scamac_dof_fermions_st *dof, const scamac_rep_fermions_st *rep);

/* operators */
double scamac_op_fermions_cdc(
    const scamac_dof_fermions_st *dof, const scamac_rep_fermions_st *rep, int ic);
double scamac_op_fermions_nn(
    const scamac_dof_fermions_st *dof, const scamac_rep_fermions_st *rep, int i, int j);

double scamac_op_fermions_cdicj(
    const scamac_dof_fermions_st *dof, scamac_rep_fermions_st *rep, int i, int j);
double scamac_op_fermions_hop(
    const scamac_dof_fermions_st *dof, scamac_rep_fermions_st *rep, int i, int j);

#endif /* SCAMAC_DOF_FERMIONS_H */
