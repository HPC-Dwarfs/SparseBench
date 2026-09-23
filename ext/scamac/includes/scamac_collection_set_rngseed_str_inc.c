if (!(strcmp(gen->name,"Anderson"))) {
  if (!strcmp(parname,"seed")) {
    ( (scamac_matrix_Anderson_params_st *) gen->par)->seed = val;
    return SCAMAC_EOK;
  }

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
  if (!strcmp(parname,"seed")) {
    ( (scamac_matrix_Hubbard_params_st *) gen->par)->seed = val;
    return SCAMAC_EOK;
  }

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
  if (!strcmp(parname,"seed")) {
    ( (scamac_matrix_TopIns_params_st *) gen->par)->seed = val;
    return SCAMAC_EOK;
  }

  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"Tridiagonal"))) {
  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"TridiagonalComplex"))) {
  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"TridiagonalReal"))) {
  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

