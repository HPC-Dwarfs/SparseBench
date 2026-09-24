if (!(strcmp(gen->name,"Anderson"))) {
  if (!strcmp(parname,"boundary_conditions")) {
    if (!strcmp(option,"open")) {
      ( (scamac_matrix_Anderson_params_st *) gen->par)->boundary_conditions = option_open;
      return SCAMAC_EOK;
    }
    if (!strcmp(option,"periodic")) {
      ( (scamac_matrix_Anderson_params_st *) gen->par)->boundary_conditions = option_periodic;
      return SCAMAC_EOK;
    }
    return scamac_error_set_par(SCAMAC_EINVALID,3);
  }

  if (!strcmp(parname,"sweep")) {
    if (!strcmp(option,"simple")) {
      ( (scamac_matrix_Anderson_params_st *) gen->par)->sweep = option_simple;
      return SCAMAC_EOK;
    }
    if (!strcmp(option,"backforth")) {
      ( (scamac_matrix_Anderson_params_st *) gen->par)->sweep = option_backforth;
      return SCAMAC_EOK;
    }
    return scamac_error_set_par(SCAMAC_EINVALID,3);
  }

  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"DiagonalReal"))) {
  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"Exciton"))) {
  if (!strcmp(parname,"symm")) {
    if (!strcmp(option,"para")) {
      ( (scamac_matrix_Exciton_params_st *) gen->par)->symm = option_para;
      return SCAMAC_EOK;
    }
    if (!strcmp(option,"ortho")) {
      ( (scamac_matrix_Exciton_params_st *) gen->par)->symm = option_ortho;
      return SCAMAC_EOK;
    }
    return scamac_error_set_par(SCAMAC_EINVALID,3);
  }

  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"FreeBosonChain"))) {
  if (!strcmp(parname,"bc")) {
    if (!strcmp(option,"open")) {
      ( (scamac_matrix_FreeBosonChain_params_st *) gen->par)->bc = option_open;
      return SCAMAC_EOK;
    }
    if (!strcmp(option,"periodic")) {
      ( (scamac_matrix_FreeBosonChain_params_st *) gen->par)->bc = option_periodic;
      return SCAMAC_EOK;
    }
    return scamac_error_set_par(SCAMAC_EINVALID,3);
  }

  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"FreeFermionChain"))) {
  if (!strcmp(parname,"bc")) {
    if (!strcmp(option,"open")) {
      ( (scamac_matrix_FreeFermionChain_params_st *) gen->par)->bc = option_open;
      return SCAMAC_EOK;
    }
    if (!strcmp(option,"periodic")) {
      ( (scamac_matrix_FreeFermionChain_params_st *) gen->par)->bc = option_periodic;
      return SCAMAC_EOK;
    }
    return scamac_error_set_par(SCAMAC_EINVALID,3);
  }

  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"Harmonic"))) {
  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"Hubbard"))) {
  if (!strcmp(parname,"boundary_conditions")) {
    if (!strcmp(option,"open")) {
      ( (scamac_matrix_Hubbard_params_st *) gen->par)->boundary_conditions = option_open;
      return SCAMAC_EOK;
    }
    if (!strcmp(option,"periodic")) {
      ( (scamac_matrix_Hubbard_params_st *) gen->par)->boundary_conditions = option_periodic;
      return SCAMAC_EOK;
    }
    return scamac_error_set_par(SCAMAC_EINVALID,3);
  }

  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"Membrane1"))) {
  if (!strcmp(parname,"pack")) {
    if (!strcmp(option,"tight")) {
      ( (scamac_matrix_Membrane1_params_st *) gen->par)->pack = option_tight;
      return SCAMAC_EOK;
    }
    if (!strcmp(option,"loose")) {
      ( (scamac_matrix_Membrane1_params_st *) gen->par)->pack = option_loose;
      return SCAMAC_EOK;
    }
    return scamac_error_set_par(SCAMAC_EINVALID,3);
  }

  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"OneFermion"))) {
  if (!strcmp(parname,"bc")) {
    if (!strcmp(option,"open")) {
      ( (scamac_wrapper_OneFermion_params_st *) gen->wrapped_par)->bc = option_open;
      return SCAMAC_EOK;
    }
    if (!strcmp(option,"periodic")) {
      ( (scamac_wrapper_OneFermion_params_st *) gen->wrapped_par)->bc = option_periodic;
      return SCAMAC_EOK;
    }
    return scamac_error_set_par(SCAMAC_EINVALID,3);
  }

  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"SpinChainXXZ"))) {
  if (!strcmp(parname,"boundary_conditions")) {
    if (!strcmp(option,"open")) {
      ( (scamac_matrix_SpinChainXXZ_params_st *) gen->par)->boundary_conditions = option_open;
      return SCAMAC_EOK;
    }
    if (!strcmp(option,"periodic")) {
      ( (scamac_matrix_SpinChainXXZ_params_st *) gen->par)->boundary_conditions = option_periodic;
      return SCAMAC_EOK;
    }
    return scamac_error_set_par(SCAMAC_EINVALID,3);
  }

  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"TightBinding"))) {
  if (!strcmp(parname,"boundary_conditions")) {
    if (!strcmp(option,"open")) {
      ( (scamac_matrix_TightBinding_params_st *) gen->par)->boundary_conditions = option_open;
      return SCAMAC_EOK;
    }
    if (!strcmp(option,"periodic")) {
      ( (scamac_matrix_TightBinding_params_st *) gen->par)->boundary_conditions = option_periodic;
      return SCAMAC_EOK;
    }
    return scamac_error_set_par(SCAMAC_EINVALID,3);
  }

  return scamac_error_set_par(SCAMAC_EINVALID,2);
}

if (!(strcmp(gen->name,"TopIns"))) {
  if (!strcmp(parname,"boundary_conditions")) {
    if (!strcmp(option,"open")) {
      ( (scamac_matrix_TopIns_params_st *) gen->par)->boundary_conditions = option_open;
      return SCAMAC_EOK;
    }
    if (!strcmp(option,"periodic")) {
      ( (scamac_matrix_TopIns_params_st *) gen->par)->boundary_conditions = option_periodic;
      return SCAMAC_EOK;
    }
    return scamac_error_set_par(SCAMAC_EINVALID,3);
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

