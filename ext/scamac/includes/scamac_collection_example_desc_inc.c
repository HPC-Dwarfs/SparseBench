if (!(strcmp(matname,"Anderson"))) {
if (desc) {
 char * my_desc = malloc(76 * sizeof *my_desc);
 strncpy(my_desc, "matrix type: Real Symmetric\n\nAnderson model of localization in 1D, 2D, 3D",75);
 my_desc[75]=0;
 *desc=my_desc;
}
if (valtype) {*valtype=SCAMAC_VAL_REAL;}
if (symmetry) {*symmetry=SCAMAC_SYMMETRIC;}
return SCAMAC_EOK;
}
if (!(strcmp(matname,"DiagonalReal"))) {
if (desc) {
 char * my_desc = malloc(54 * sizeof *my_desc);
 strncpy(my_desc, "matrix type: Real Symmetric\n\nDiagonal (test) matrix",53);
 my_desc[53]=0;
 *desc=my_desc;
}
if (valtype) {*valtype=SCAMAC_VAL_REAL;}
if (symmetry) {*symmetry=SCAMAC_SYMMETRIC;}
return SCAMAC_EOK;
}
if (!(strcmp(matname,"Exciton"))) {
if (desc) {
 char * my_desc = malloc(55 * sizeof *my_desc);
 strncpy(my_desc, "matrix type: Complex Hermitian\n\nExciton on a lattice",54);
 my_desc[54]=0;
 *desc=my_desc;
}
if (valtype) {*valtype=SCAMAC_VAL_COMPLEX;}
if (symmetry) {*symmetry=SCAMAC_HERMITIAN;}
return SCAMAC_EOK;
}
if (!(strcmp(matname,"FreeBosonChain"))) {
if (desc) {
 char * my_desc = malloc(54 * sizeof *my_desc);
 strncpy(my_desc, "matrix type: Real Symmetric\n\nFree bosons on a chain",53);
 my_desc[53]=0;
 *desc=my_desc;
}
if (valtype) {*valtype=SCAMAC_VAL_REAL;}
if (symmetry) {*symmetry=SCAMAC_SYMMETRIC;}
return SCAMAC_EOK;
}
if (!(strcmp(matname,"FreeFermionChain"))) {
if (desc) {
 char * my_desc = malloc(56 * sizeof *my_desc);
 strncpy(my_desc, "matrix type: Real Symmetric\n\nFree fermions on a chain",55);
 my_desc[55]=0;
 *desc=my_desc;
}
if (valtype) {*valtype=SCAMAC_VAL_REAL;}
if (symmetry) {*symmetry=SCAMAC_SYMMETRIC;}
return SCAMAC_EOK;
}
if (!(strcmp(matname,"Harmonic"))) {
if (desc) {
 char * my_desc = malloc(59 * sizeof *my_desc);
 strncpy(my_desc, "matrix type: Real Symmetric\n\nQuantum harmonic oscillator",58);
 my_desc[58]=0;
 *desc=my_desc;
}
if (valtype) {*valtype=SCAMAC_VAL_REAL;}
if (symmetry) {*symmetry=SCAMAC_SYMMETRIC;}
return SCAMAC_EOK;
}
if (!(strcmp(matname,"Hubbard"))) {
if (desc) {
 char * my_desc = malloc(73 * sizeof *my_desc);
 strncpy(my_desc, "matrix type: Real Symmetric\n\nThe 1D Hubbard model from quantum physics",72);
 my_desc[72]=0;
 *desc=my_desc;
}
if (valtype) {*valtype=SCAMAC_VAL_REAL;}
if (symmetry) {*symmetry=SCAMAC_SYMMETRIC;}
return SCAMAC_EOK;
}
if (!(strcmp(matname,"Membrane1"))) {
if (desc) {
 char * my_desc = malloc(96 * sizeof *my_desc);
 strncpy(my_desc, "matrix type: Real General\n\nDamped vibrations of a thin rectangular membrane (friction type 1)",95);
 my_desc[95]=0;
 *desc=my_desc;
}
if (valtype) {*valtype=SCAMAC_VAL_REAL;}
if (symmetry) {*symmetry=SCAMAC_GENERAL;}
return SCAMAC_EOK;
}
if (!(strcmp(matname,"OneFermion"))) {
if (desc) {
 char * my_desc = malloc(54 * sizeof *my_desc);
 strncpy(my_desc, "matrix type: Real Symmetric\n\none fermion on a chain",53);
 my_desc[53]=0;
 *desc=my_desc;
}
if (valtype) {*valtype=SCAMAC_VAL_REAL;}
if (symmetry) {*symmetry=SCAMAC_SYMMETRIC;}
return SCAMAC_EOK;
}
if (!(strcmp(matname,"SpinChainXXZ"))) {
if (desc) {
 char * my_desc = malloc(57 * sizeof *my_desc);
 strncpy(my_desc, "matrix type: Real Symmetric\n\nOne-dimensional XXZ model",56);
 my_desc[56]=0;
 *desc=my_desc;
}
if (valtype) {*valtype=SCAMAC_VAL_REAL;}
if (symmetry) {*symmetry=SCAMAC_SYMMETRIC;}
return SCAMAC_EOK;
}
if (!(strcmp(matname,"TightBinding"))) {
if (desc) {
 char * my_desc = malloc(97 * sizeof *my_desc);
 strncpy(my_desc, "matrix type: Real Symmetric\n\nsimple 1D (chain), 2D (square), or 3D (cubic) tight-binding model",96);
 my_desc[96]=0;
 *desc=my_desc;
}
if (valtype) {*valtype=SCAMAC_VAL_REAL;}
if (symmetry) {*symmetry=SCAMAC_SYMMETRIC;}
return SCAMAC_EOK;
}
if (!(strcmp(matname,"TopIns"))) {
if (desc) {
 char * my_desc = malloc(59 * sizeof *my_desc);
 strncpy(my_desc, "matrix type: Complex Hermitian\n\n3D topological insulator",58);
 my_desc[58]=0;
 *desc=my_desc;
}
if (valtype) {*valtype=SCAMAC_VAL_COMPLEX;}
if (symmetry) {*symmetry=SCAMAC_HERMITIAN;}
return SCAMAC_EOK;
}
if (!(strcmp(matname,"Tridiagonal"))) {
if (desc) {
 char * my_desc = malloc(53 * sizeof *my_desc);
 strncpy(my_desc, "matrix type: Complex Hermitian\n\nTridiagonal matrix",52);
 my_desc[52]=0;
 *desc=my_desc;
}
if (valtype) {*valtype=SCAMAC_VAL_COMPLEX;}
if (symmetry) {*symmetry=SCAMAC_HERMITIAN;}
return SCAMAC_EOK;
}
if (!(strcmp(matname,"TridiagonalComplex"))) {
if (desc) {
 char * my_desc = malloc(65 * sizeof *my_desc);
 strncpy(my_desc, "matrix type: Complex General\n\nNon-symmetric tridiagonal matrix",64);
 my_desc[64]=0;
 *desc=my_desc;
}
if (valtype) {*valtype=SCAMAC_VAL_COMPLEX;}
if (symmetry) {*symmetry=SCAMAC_GENERAL;}
return SCAMAC_EOK;
}
if (!(strcmp(matname,"TridiagonalReal"))) {
if (desc) {
 char * my_desc = malloc(62 * sizeof *my_desc);
 strncpy(my_desc, "matrix type: Real General\n\nNon-symmetric tridiagonal matrix",61);
 my_desc[61]=0;
 *desc=my_desc;
}
if (valtype) {*valtype=SCAMAC_VAL_REAL;}
if (symmetry) {*symmetry=SCAMAC_GENERAL;}
return SCAMAC_EOK;
}
