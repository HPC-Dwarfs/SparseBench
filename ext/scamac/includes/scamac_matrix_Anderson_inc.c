ScamacErrorCode scamac_matrix_Anderson_check(const scamac_matrix_Anderson_params_st * par, char ** desc) {
  if (!par)  { return scamac_error_set_par(SCAMAC_ENULL,1); }
  ScamacErrorCode err = SCAMAC_EOK;
  scamac_string_st str;
  if (desc) { scamac_string_empty(&str); }
  assert((par->boundary_conditions== option_open) || (par->boundary_conditions== option_periodic));
  assert((par->sweep== option_simple) || (par->sweep== option_backforth));
  if (!((par->Lx)  > 0)) {
    err=SCAMAC_EINVALID;
    if (desc) { scamac_string_append(&str,"[Parameter range] Lx  > 0\n"); }
  }
  if (!((par->Ly)  > 0)) {
    err=SCAMAC_EINVALID;
    if (desc) { scamac_string_append(&str,"[Parameter range] Ly  > 0\n"); }
  }
  if (!((par->Lz)  > 0)) {
    err=SCAMAC_EINVALID;
    if (desc) { scamac_string_append(&str,"[Parameter range] Lz  > 0\n"); }
  }
  if (!((par->ranpot)  >= 0)) {
    err=SCAMAC_EINVALID;
    if (desc) { scamac_string_append(&str,"[Parameter range] ranpot  >= 0\n"); }
  }
  if (desc) { *desc = scamac_string_get(&str); }
  return err;
  }
