ScamacErrorCode scamac_matrix_Hubbard_check(const scamac_matrix_Hubbard_params_st * par, char ** desc) {
  if (!par)  { return scamac_error_set_par(SCAMAC_ENULL,1); }
  ScamacErrorCode err = SCAMAC_EOK;
  scamac_string_st str;
  if (desc) { scamac_string_empty(&str); }
  assert((par->boundary_conditions== option_open) || (par->boundary_conditions== option_periodic));
  if (!((par->n_fermions) <= (par->n_sites))) {
    err=SCAMAC_EINVALID;
    if (desc) { scamac_string_append(&str,"\n"); }
  }
  if (!((par->n_sites) <= 1000)) {
    err=SCAMAC_EWARNING;
    if (desc) { scamac_string_append(&str,"[WARNING] very large n_sites (>1000)\n"); }
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
