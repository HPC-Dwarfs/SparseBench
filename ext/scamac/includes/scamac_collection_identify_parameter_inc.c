if (!(strcmp(matname,"Anderson"))) {
  if (!strcmp(parname,"Lx")) {
    return SCAMAC_PAR_INT;
  }
  if (!strcmp(parname,"Ly")) {
    return SCAMAC_PAR_INT;
  }
  if (!strcmp(parname,"Lz")) {
    return SCAMAC_PAR_INT;
  }
  if (!strcmp(parname,"t")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"ranpot")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"boundary_conditions")) {
    return SCAMAC_PAR_OPTION;
  }
  if (!strcmp(parname,"seed")) {
    return SCAMAC_PAR_RNGSEED;
  }
  if (!strcmp(parname,"sweep")) {
    return SCAMAC_PAR_OPTION;
  }
return SCAMAC_PAR_NONE;
}

if (!(strcmp(matname,"DiagonalReal"))) {
  if (!strcmp(parname,"n")) {
    return SCAMAC_PAR_INT;
  }
  if (!strcmp(parname,"dmin")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"dmax")) {
    return SCAMAC_PAR_DOUBLE;
  }
return SCAMAC_PAR_NONE;
}

if (!(strcmp(matname,"Exciton"))) {
  if (!strcmp(parname,"so")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"ex")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"mlh")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"mhh")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"me")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"eps")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"lc")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"kx")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"ky")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"kz")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"a")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"L")) {
    return SCAMAC_PAR_INT;
  }
  if (!strcmp(parname,"symm")) {
    return SCAMAC_PAR_OPTION;
  }
return SCAMAC_PAR_NONE;
}

if (!(strcmp(matname,"FreeBosonChain"))) {
  if (!strcmp(parname,"t")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"n_species")) {
    return SCAMAC_PAR_INT;
  }
  if (!strcmp(parname,"n_sites")) {
    return SCAMAC_PAR_INT;
  }
  if (!strcmp(parname,"n_bosons")) {
    return SCAMAC_PAR_INT;
  }
  if (!strcmp(parname,"bc")) {
    return SCAMAC_PAR_OPTION;
  }
return SCAMAC_PAR_NONE;
}

if (!(strcmp(matname,"FreeFermionChain"))) {
  if (!strcmp(parname,"t")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"n_species")) {
    return SCAMAC_PAR_INT;
  }
  if (!strcmp(parname,"n_sites")) {
    return SCAMAC_PAR_INT;
  }
  if (!strcmp(parname,"n_fermions")) {
    return SCAMAC_PAR_INT;
  }
  if (!strcmp(parname,"bc")) {
    return SCAMAC_PAR_OPTION;
  }
return SCAMAC_PAR_NONE;
}

if (!(strcmp(matname,"Harmonic"))) {
  if (!strcmp(parname,"omega")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"lambda")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"n_bos")) {
    return SCAMAC_PAR_INT;
  }
return SCAMAC_PAR_NONE;
}

if (!(strcmp(matname,"Hubbard"))) {
  if (!strcmp(parname,"t")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"U")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"n_sites")) {
    return SCAMAC_PAR_INT;
  }
  if (!strcmp(parname,"n_fermions")) {
    return SCAMAC_PAR_INT;
  }
  if (!strcmp(parname,"boundary_conditions")) {
    return SCAMAC_PAR_OPTION;
  }
  if (!strcmp(parname,"ranpot")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"seed")) {
    return SCAMAC_PAR_RNGSEED;
  }
return SCAMAC_PAR_NONE;
}

if (!(strcmp(matname,"Membrane1"))) {
  if (!strcmp(parname,"lx")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"ly")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"rho")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"cwave")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"sigma")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"nx")) {
    return SCAMAC_PAR_INT;
  }
  if (!strcmp(parname,"ny")) {
    return SCAMAC_PAR_INT;
  }
  if (!strcmp(parname,"pack")) {
    return SCAMAC_PAR_OPTION;
  }
return SCAMAC_PAR_NONE;
}

if (!(strcmp(matname,"OneFermion"))) {
  if (!strcmp(parname,"t")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"n_sites")) {
    return SCAMAC_PAR_INT;
  }
  if (!strcmp(parname,"bc")) {
    return SCAMAC_PAR_OPTION;
  }
return SCAMAC_PAR_NONE;
}

if (!(strcmp(matname,"SpinChainXXZ"))) {
  if (!strcmp(parname,"Jxy")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"Jz")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"Bz")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"n_sites")) {
    return SCAMAC_PAR_INT;
  }
  if (!strcmp(parname,"n_up")) {
    return SCAMAC_PAR_INT;
  }
  if (!strcmp(parname,"boundary_conditions")) {
    return SCAMAC_PAR_OPTION;
  }
return SCAMAC_PAR_NONE;
}

if (!(strcmp(matname,"TightBinding"))) {
  if (!strcmp(parname,"Lx")) {
    return SCAMAC_PAR_INT;
  }
  if (!strcmp(parname,"Ly")) {
    return SCAMAC_PAR_INT;
  }
  if (!strcmp(parname,"Lz")) {
    return SCAMAC_PAR_INT;
  }
  if (!strcmp(parname,"t")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"boundary_conditions")) {
    return SCAMAC_PAR_OPTION;
  }
  if (!strcmp(parname,"mx")) {
    return SCAMAC_PAR_INT;
  }
  if (!strcmp(parname,"my")) {
    return SCAMAC_PAR_INT;
  }
  if (!strcmp(parname,"mz")) {
    return SCAMAC_PAR_INT;
  }
return SCAMAC_PAR_NONE;
}

if (!(strcmp(matname,"TopIns"))) {
  if (!strcmp(parname,"Lx")) {
    return SCAMAC_PAR_INT;
  }
  if (!strcmp(parname,"Ly")) {
    return SCAMAC_PAR_INT;
  }
  if (!strcmp(parname,"Lz")) {
    return SCAMAC_PAR_INT;
  }
  if (!strcmp(parname,"t")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"m")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"D1")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"D2")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"ranpot")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"boundary_conditions")) {
    return SCAMAC_PAR_OPTION;
  }
  if (!strcmp(parname,"seed")) {
    return SCAMAC_PAR_RNGSEED;
  }
  if (!strcmp(parname,"mx")) {
    return SCAMAC_PAR_INT;
  }
  if (!strcmp(parname,"my")) {
    return SCAMAC_PAR_INT;
  }
  if (!strcmp(parname,"mz")) {
    return SCAMAC_PAR_INT;
  }
return SCAMAC_PAR_NONE;
}

if (!(strcmp(matname,"Tridiagonal"))) {
  if (!strcmp(parname,"n")) {
    return SCAMAC_PAR_IDX;
  }
  if (!strcmp(parname,"diag")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"offdiag")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"phi")) {
    return SCAMAC_PAR_DOUBLE;
  }
return SCAMAC_PAR_NONE;
}

if (!(strcmp(matname,"TridiagonalComplex"))) {
  if (!strcmp(parname,"n")) {
    return SCAMAC_PAR_IDX;
  }
  if (!strcmp(parname,"diag_re")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"diag_im")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"subdiag")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"supdiag")) {
    return SCAMAC_PAR_DOUBLE;
  }
return SCAMAC_PAR_NONE;
}

if (!(strcmp(matname,"TridiagonalReal"))) {
  if (!strcmp(parname,"n")) {
    return SCAMAC_PAR_IDX;
  }
  if (!strcmp(parname,"diag")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"subdiag")) {
    return SCAMAC_PAR_DOUBLE;
  }
  if (!strcmp(parname,"supdiag")) {
    return SCAMAC_PAR_DOUBLE;
  }
return SCAMAC_PAR_NONE;
}

