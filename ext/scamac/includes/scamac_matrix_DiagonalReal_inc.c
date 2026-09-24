ScamacErrorCode scamac_matrix_DiagonalReal_check(const scamac_matrix_DiagonalReal_params_st * par, char ** desc) {
  if (!par)  { return scamac_error_set_par(SCAMAC_ENULL,1); }
  ScamacErrorCode err = SCAMAC_EOK;
  scamac_string_st str;
  if (desc) { scamac_string_empty(&str); }
  if (!((par->dmin) < (par->dmax))) {
    err=SCAMAC_EINVALID;
    if (desc) { scamac_string_append(&str,"\n"); }
  }
  if (!((par->n)  > 0)) {
    err=SCAMAC_EINVALID;
    if (desc) { scamac_string_append(&str,"[Parameter range] n  > 0\n"); }
  }
  if (desc) { *desc = scamac_string_get(&str); }
  return err;
  }
