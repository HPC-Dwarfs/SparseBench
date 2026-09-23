#include "scamac_string.h"
#include "scamac_wrapper_OneFermion.h"
#include "scamac_wrapper_OneFermion_inc.c"
ScamacErrorCode scamac_wrapper_OneFermion_unwrap(
    const scamac_wrapper_OneFermion_params_st *par_in,
    scamac_matrix_FreeFermionChain_params_st *par_out)
{
  (par_out->t)          = (par_in->t);
  (par_out->n_fermions) = 1;
  (par_out->n_species)  = 1;
  (par_out->n_sites)    = (par_in->n_sites);
  (par_out->bc)         = (par_in->bc);
  return SCAMAC_EOK;
}
