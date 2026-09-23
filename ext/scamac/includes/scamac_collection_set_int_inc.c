if (!(strcmp(gen->name,"Anderson"))) {
  if (!strcmp(parname,"Lx")) {
    if (!(val > 0)) { return scamac_error_set_par(SCAMAC_ERANGE,3); }
    ( (scamac_matrix_Anderson_params_st *) gen->par)->Lx = val;
    return SCAMAC_EOK;
  }

  if (!strcmp(parname,"Ly")) {
    if (!(val > 0)) { return scamac_error_set_par(SCAMAC_ERANGE,3); }
    ( (scamac_matrix_Anderson_params_st *) gen->par)->Ly = val;
    return SCAMAC_EOK;
  }

  if (!strcmp(parname,"Lz")) {
    if (!(val > 0)) { return scamac_error_set_par(SCAMAC_ERANGE,3); }
    ( (scamac_matrix_Anderson_params_st *) gen->par)->Lz = val;
    return SCAMAC_EOK;
  }

  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"DiagonalReal"))) {
  if (!strcmp(parname,"n")) {
    if (!(val > 0)) { return scamac_error_set_par(SCAMAC_ERANGE,3); }
    ( (scamac_matrix_DiagonalReal_params_st *) gen->par)->n = val;
    return SCAMAC_EOK;
  }

  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"Exciton"))) {
  if (!strcmp(parname,"L")) {
    if (!(val > 0)) { return scamac_error_set_par(SCAMAC_ERANGE,3); }
    ( (scamac_matrix_Exciton_params_st *) gen->par)->L = val;
    return SCAMAC_EOK;
  }

  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"FreeBosonChain"))) {
  if (!strcmp(parname,"n_species")) {
    if (!(val > 0)) { return scamac_error_set_par(SCAMAC_ERANGE,3); }
    ( (scamac_matrix_FreeBosonChain_params_st *) gen->par)->n_species = val;
    return SCAMAC_EOK;
  }

  if (!strcmp(parname,"n_sites")) {
    if (!(val > 0)) { return scamac_error_set_par(SCAMAC_ERANGE,3); }
    ( (scamac_matrix_FreeBosonChain_params_st *) gen->par)->n_sites = val;
    return SCAMAC_EOK;
  }

  if (!strcmp(parname,"n_bosons")) {
    if (!(val > 0)) { return scamac_error_set_par(SCAMAC_ERANGE,3); }
    ( (scamac_matrix_FreeBosonChain_params_st *) gen->par)->n_bosons = val;
    return SCAMAC_EOK;
  }

  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"FreeFermionChain"))) {
  if (!strcmp(parname,"n_species")) {
    if (!(val > 0)) { return scamac_error_set_par(SCAMAC_ERANGE,3); }
    ( (scamac_matrix_FreeFermionChain_params_st *) gen->par)->n_species = val;
    return SCAMAC_EOK;
  }

  if (!strcmp(parname,"n_sites")) {
    if (!(val > 0)) { return scamac_error_set_par(SCAMAC_ERANGE,3); }
    ( (scamac_matrix_FreeFermionChain_params_st *) gen->par)->n_sites = val;
    return SCAMAC_EOK;
  }

  if (!strcmp(parname,"n_fermions")) {
    if (!(val > 0)) { return scamac_error_set_par(SCAMAC_ERANGE,3); }
    ( (scamac_matrix_FreeFermionChain_params_st *) gen->par)->n_fermions = val;
    return SCAMAC_EOK;
  }

  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"Harmonic"))) {
  if (!strcmp(parname,"n_bos")) {
    if (!(val > 0)) { return scamac_error_set_par(SCAMAC_ERANGE,3); }
    ( (scamac_matrix_Harmonic_params_st *) gen->par)->n_bos = val;
    return SCAMAC_EOK;
  }

  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"Hubbard"))) {
  if (!strcmp(parname,"n_sites")) {
    if (!(val > 0)) { return scamac_error_set_par(SCAMAC_ERANGE,3); }
    ( (scamac_matrix_Hubbard_params_st *) gen->par)->n_sites = val;
    return SCAMAC_EOK;
  }

  if (!strcmp(parname,"n_fermions")) {
    if (!(val > 0)) { return scamac_error_set_par(SCAMAC_ERANGE,3); }
    ( (scamac_matrix_Hubbard_params_st *) gen->par)->n_fermions = val;
    return SCAMAC_EOK;
  }

  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"Membrane1"))) {
  if (!strcmp(parname,"nx")) {
    if (!(val > 0)) { return scamac_error_set_par(SCAMAC_ERANGE,3); }
    ( (scamac_matrix_Membrane1_params_st *) gen->par)->nx = val;
    return SCAMAC_EOK;
  }

  if (!strcmp(parname,"ny")) {
    if (!(val > 0)) { return scamac_error_set_par(SCAMAC_ERANGE,3); }
    ( (scamac_matrix_Membrane1_params_st *) gen->par)->ny = val;
    return SCAMAC_EOK;
  }

  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"OneFermion"))) {
  if (!strcmp(parname,"n_sites")) {
    ( (scamac_wrapper_OneFermion_params_st *) gen->wrapped_par)->n_sites = val;
    return SCAMAC_EOK;
  }

  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"SpinChainXXZ"))) {
  if (!strcmp(parname,"n_sites")) {
    if (!(val > 0)) { return scamac_error_set_par(SCAMAC_ERANGE,3); }
    ( (scamac_matrix_SpinChainXXZ_params_st *) gen->par)->n_sites = val;
    return SCAMAC_EOK;
  }

  if (!strcmp(parname,"n_up")) {
    if (!(val > 0)) { return scamac_error_set_par(SCAMAC_ERANGE,3); }
    ( (scamac_matrix_SpinChainXXZ_params_st *) gen->par)->n_up = val;
    return SCAMAC_EOK;
  }

  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"TightBinding"))) {
  if (!strcmp(parname,"Lx")) {
    if (!(val > 0)) { return scamac_error_set_par(SCAMAC_ERANGE,3); }
    ( (scamac_matrix_TightBinding_params_st *) gen->par)->Lx = val;
    return SCAMAC_EOK;
  }

  if (!strcmp(parname,"Ly")) {
    if (!(val > 0)) { return scamac_error_set_par(SCAMAC_ERANGE,3); }
    ( (scamac_matrix_TightBinding_params_st *) gen->par)->Ly = val;
    return SCAMAC_EOK;
  }

  if (!strcmp(parname,"Lz")) {
    if (!(val > 0)) { return scamac_error_set_par(SCAMAC_ERANGE,3); }
    ( (scamac_matrix_TightBinding_params_st *) gen->par)->Lz = val;
    return SCAMAC_EOK;
  }

  if (!strcmp(parname,"mx")) {
    if (!(val > 0)) { return scamac_error_set_par(SCAMAC_ERANGE,3); }
    ( (scamac_matrix_TightBinding_params_st *) gen->par)->mx = val;
    return SCAMAC_EOK;
  }

  if (!strcmp(parname,"my")) {
    if (!(val > 0)) { return scamac_error_set_par(SCAMAC_ERANGE,3); }
    ( (scamac_matrix_TightBinding_params_st *) gen->par)->my = val;
    return SCAMAC_EOK;
  }

  if (!strcmp(parname,"mz")) {
    if (!(val > 0)) { return scamac_error_set_par(SCAMAC_ERANGE,3); }
    ( (scamac_matrix_TightBinding_params_st *) gen->par)->mz = val;
    return SCAMAC_EOK;
  }

  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"TopIns"))) {
  if (!strcmp(parname,"Lx")) {
    if (!(val > 0)) { return scamac_error_set_par(SCAMAC_ERANGE,3); }
    ( (scamac_matrix_TopIns_params_st *) gen->par)->Lx = val;
    return SCAMAC_EOK;
  }

  if (!strcmp(parname,"Ly")) {
    if (!(val > 0)) { return scamac_error_set_par(SCAMAC_ERANGE,3); }
    ( (scamac_matrix_TopIns_params_st *) gen->par)->Ly = val;
    return SCAMAC_EOK;
  }

  if (!strcmp(parname,"Lz")) {
    if (!(val > 0)) { return scamac_error_set_par(SCAMAC_ERANGE,3); }
    ( (scamac_matrix_TopIns_params_st *) gen->par)->Lz = val;
    return SCAMAC_EOK;
  }

  if (!strcmp(parname,"mx")) {
    if (!(val > 0)) { return scamac_error_set_par(SCAMAC_ERANGE,3); }
    ( (scamac_matrix_TopIns_params_st *) gen->par)->mx = val;
    return SCAMAC_EOK;
  }

  if (!strcmp(parname,"my")) {
    if (!(val > 0)) { return scamac_error_set_par(SCAMAC_ERANGE,3); }
    ( (scamac_matrix_TopIns_params_st *) gen->par)->my = val;
    return SCAMAC_EOK;
  }

  if (!strcmp(parname,"mz")) {
    if (!(val > 0)) { return scamac_error_set_par(SCAMAC_ERANGE,3); }
    ( (scamac_matrix_TopIns_params_st *) gen->par)->mz = val;
    return SCAMAC_EOK;
  }

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

