ScamacErrorCode scamac_matrix_FreeFermionChain_check(const scamac_matrix_FreeFermionChain_params_st * par, char ** desc) {
  if (!par)  { return scamac_error_set_par(SCAMAC_ENULL,1); }
  ScamacErrorCode err = SCAMAC_EOK;
  scamac_string_st str;
  if (desc) { scamac_string_empty(&str); }
  assert((par->bc== option_open) || (par->bc== option_periodic));
  if (!((par->n_fermions) <= (par->n_sites))) {
    err=SCAMAC_EINVALID;
    if (desc) { scamac_string_append(&str,"\n"); }
  }
  if (!((par->n_species)  > 0)) {
    err=SCAMAC_EINVALID;
    if (desc) { scamac_string_append(&str,"[Parameter range] n_species  > 0\n"); }
  }
  if (!((par->n_sites)  > 0)) {
    err=SCAMAC_EINVALID;
    if (desc) { scamac_string_append(&str,"[Parameter range] n_sites  > 0\n"); }
  }
  if (!((par->n_fermions)  > 0)) {
    err=SCAMAC_EINVALID;
    if (desc) { scamac_string_append(&str,"[Parameter range] n_fermions  > 0\n"); }
  }
  if (desc) { *desc = scamac_string_get(&str); }
  return err;
  }
