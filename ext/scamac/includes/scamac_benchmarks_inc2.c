if (!strcmp(name,"Simple")) {
  if (m>2) {return scamac_error_set_par(SCAMAC_ERANGE,2); }
  if (m==1) {
    err=scamac_generator_obtain("Hubbard", gen);
    if (err) { return scamac_error_set_internal(err); }
    err=scamac_generator_set_int(*gen, "n_sites",2*x);
    if (err) { return scamac_error_set_internal(err); }
    err=scamac_generator_set_int(*gen, "n_fermions",x);
    if (err) { return scamac_error_set_internal(err); }
    return SCAMAC_EOK;
  }
  if (m==2) {
    err=scamac_generator_obtain("Anderson", gen);
    if (err) { return scamac_error_set_internal(err); }
    err=scamac_generator_set_double(*gen, "ranpot",0.5*x);
    if (err) { return scamac_error_set_internal(err); }
    return SCAMAC_EOK;
  }
  return scamac_error_set_par(SCAMAC_ERANGE,2);
}
