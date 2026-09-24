ScamacErrorCode scamac_matrix_TightBinding_check(const scamac_matrix_TightBinding_params_st * par, char ** desc) {
  if (!par)  { return scamac_error_set_par(SCAMAC_ENULL,1); }
  ScamacErrorCode err = SCAMAC_EOK;
  scamac_string_st str;
  if (desc) { scamac_string_empty(&str); }
  assert((par->boundary_conditions== option_open) || (par->boundary_conditions== option_periodic));
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
  if (!((par->mx)  > 0)) {
    err=SCAMAC_EINVALID;
    if (desc) { scamac_string_append(&str,"[Parameter range] mx  > 0\n"); }
  }
  if (!((par->my)  > 0)) {
    err=SCAMAC_EINVALID;
    if (desc) { scamac_string_append(&str,"[Parameter range] my  > 0\n"); }
  }
  if (!((par->mz)  > 0)) {
    err=SCAMAC_EINVALID;
    if (desc) { scamac_string_append(&str,"[Parameter range] mz  > 0\n"); }
  }
  if (desc) { *desc = scamac_string_get(&str); }
  return err;
  }
