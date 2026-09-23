if (!(strcmp(gen->name,"Anderson"))) {
  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"DiagonalReal"))) {
  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"Exciton"))) {
  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"FreeBosonChain"))) {
  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"FreeFermionChain"))) {
  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"Harmonic"))) {
  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"Hubbard"))) {
  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"Membrane1"))) {
  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"OneFermion"))) {
  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"SpinChainXXZ"))) {
  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"TightBinding"))) {
  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"TopIns"))) {
  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"Tridiagonal"))) {
  if (!strcmp(parname,"n")) {
    if (!(val > 0)) { return scamac_error_set_par(SCAMAC_ERANGE,3); }
    ( (scamac_matrix_Tridiagonal_params_st *) gen->par)->n = val;
    return SCAMAC_EOK;
  }

  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"TridiagonalComplex"))) {
  if (!strcmp(parname,"n")) {
    if (!(val > 0)) { return scamac_error_set_par(SCAMAC_ERANGE,3); }
    ( (scamac_matrix_TridiagonalComplex_params_st *) gen->par)->n = val;
    return SCAMAC_EOK;
  }

  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"TridiagonalReal"))) {
  if (!strcmp(parname,"n")) {
    if (!(val > 0)) { return scamac_error_set_par(SCAMAC_ERANGE,3); }
    ( (scamac_matrix_TridiagonalReal_params_st *) gen->par)->n = val;
    return SCAMAC_EOK;
  }

  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

