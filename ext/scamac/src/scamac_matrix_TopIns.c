#include <stdlib.h>
#include <complex.h>

#include "scamac_safeint.h"
#include "scamac_string.h"

#include "scamac_matrix_TopIns.h"
#include "scamac_matrix_TopIns_inc.c"

ScamacErrorCode scamac_matrix_TopIns_tables_create(
    const scamac_matrix_TopIns_params_st *par,
    scamac_matrix_TopIns_tables_st **tab,
    scamac_info_st *info)
{
  if (!par || !tab) {
    return SCAMAC_ENULL;
  }

  scamac_matrix_TopIns_tables_st *my_tab = malloc(sizeof *my_tab);
  if (!my_tab) {
    return SCAMAC_EMALLOCFAIL;
  }

  my_tab->dof.Lx = par->Lx;
  my_tab->dof.Ly = par->Ly;
  my_tab->dof.Lz = par->Lz;
  my_tab->dof.mx = par->mx;
  my_tab->dof.my = par->my;
  my_tab->dof.mz = par->mz;

  my_tab->ns     = scamac_safeidx_mult(scamac_dof_lattice_cubic_ns(&(my_tab->dof)), 4);
  if (my_tab->ns < 0) {
    return SCAMAC_EOVERFLOW;
  }

  *tab = my_tab;

  if (info) {
    info->nrow     = my_tab->ns;
    info->ncol     = info->nrow;
    info->maxnzrow = 6 * 2 + 3 * 2 + 1;
    info->maxnzcol = info->maxnzrow;
    info->maxnzgen = 6 * 2 + 3 * 2 + 1;
    info->maxnz    = scamac_safeidx_mult(info->nrow, info->maxnzrow);
    if (info->maxnz < 0) {
      return SCAMAC_EOVERFLOW;
    }
    info->valtype  = SCAMAC_VAL_COMPLEX;
    info->symmetry = SCAMAC_HERMITIAN;
  }

  return SCAMAC_EOK;
}

ScamacErrorCode scamac_matrix_TopIns_tables_destroy(scamac_matrix_TopIns_tables_st *tab)
{
  if (tab) {
    free(tab);
  }

  return SCAMAC_EOK;
}

ScamacErrorCode scamac_matrix_TopIns_work_alloc(const scamac_matrix_TopIns_params_st *par,
    const scamac_matrix_TopIns_tables_st *tab,
    scamac_matrix_TopIns_work_st **ws)
{
  if ((!par) || (!tab) || (!ws)) {
    return SCAMAC_ENULL;
  }
  scamac_matrix_TopIns_work_st *my_ws = malloc(sizeof *my_ws);
  if (!my_ws) {
    return SCAMAC_EMALLOCFAIL;
  }

  ScamacErrorCode err;
  err = scamac_ransrc_alloc(par->seed, &(my_ws->rng));
  if (err) {
    free(my_ws);
    return err;
  }
  *ws = my_ws;

  return SCAMAC_EOK;
}

ScamacErrorCode scamac_matrix_TopIns_work_free(scamac_matrix_TopIns_work_st *ws)
{
  if (ws) {
    scamac_ransrc_free(ws->rng);
    free(ws);
  }
  return SCAMAC_EOK;
}

ScamacErrorCode scamac_matrix_TopIns_generate_row(
    const scamac_matrix_TopIns_params_st *par,
    const scamac_matrix_TopIns_tables_st *tab,
    scamac_matrix_TopIns_work_st *ws,
    ScamacIdx irow,
    ScamacFlag flag,
    scamac_sparserow_cplx_st *row)
{

  if (!par || !tab || !ws || !row) {
    return SCAMAC_ENULL;
  }

  if (flag & ~SCAMAC_TRANSPOSE & ~SCAMAC_CONJUGATE & ~SCAMAC_KEEPZEROS) {
    return SCAMAC_EINVALID;
  }
  bool fl_transpose = (flag & SCAMAC_TRANSPOSE) != 0;
  bool fl_conjugate = (flag & SCAMAC_CONJUGATE) != 0;
  bool fl_keepzeros = (flag & SCAMAC_KEEPZEROS) != 0;

  if ((irow < 0) || (irow >= tab->ns)) {
    return SCAMAC_ERANGE;
  }

  static const double complex G1MIG2[4][4] = {
    { 1,  0,  0, 1  },
    { 0,  -1, 1, 0  },
    { 0,  -1, 1, 0  },
    { -1, 0,  0, -1 }
  };

  static const double complex G1MIG3[4][4] = {
    { 1,  0,  0,  -I },
    { 0,  -1, -I, 0  },
    { 0,  -I, 1,  0  },
    { -I, 0,  0,  -1 }
  };

  static const double complex G1MIG4[4][4] = {
    { -1, 1,  0,  0  },
    { -1, -1, 0,  0  },
    { 0,  0,  1,  1  },
    { 0,  0,  -1, -1 }
  };

  static const double complex G15[4][4] = {
    { 0,  I, 0, 0  },
    { -I, 0, 0, 0  },
    { 0,  0, 0, -I },
    { 0,  0, I, 0  }
  };

  static const double complex G1[4][4] = {
    { 1, 0,  0, 0  },
    { 0, -1, 0, 0  },
    { 0, 0,  1, 0  },
    { 0, 0,  0, -1 }
  };

  static const double complex G5[4][4] = {
    { 0,  -1, 0, 0 },
    { -1, 0,  0, 0 },
    { 0,  0,  0, 1 },
    { 0,  0,  1, 0 }
  };

  static const ScamacIdx nloc = 4;
  ScamacIdx iloc, isite;
  isite = irow / nloc;
  iloc  = irow % nloc;
  SCAMAC_ASSERT((0 <= iloc) && (iloc < 4));

  ScamacIdx x, y, z;
  scamac_dof_lattice_cubic_decode(&(tab->dof), isite, &x, &y, &z);

  int i;

  double complex cval;

  if (par->t != 0.0 || fl_keepzeros) {
    for (i = 0; i < nloc; i++) {
      if (par->Lx > 1) {
        if (x < par->Lx - 1 || par->boundary_conditions == option_periodic) {
          if (G1MIG2[i][iloc]) {
            cval = -0.5 * par->t * G1MIG2[i][iloc];
            if (fl_transpose != fl_conjugate) {
              cval = conj(cval);
            }
            SCAMAC_APPROVE(scamac_sparserow_cplx_add(row,
                cval,
                nloc * scamac_dof_lattice_cubic_encode(
                           &(tab->dof), (x + 1) % par->Lx, y, z) +
                    i));
          }
        }
        if (x > 0 || par->boundary_conditions == option_periodic) {
          if (G1MIG2[iloc][i]) {
            cval = -0.5 * par->t * conj(G1MIG2[iloc][i]);
            if (fl_transpose != fl_conjugate) {
              cval = conj(cval);
            }
            SCAMAC_APPROVE(scamac_sparserow_cplx_add(row,
                cval,
                nloc * scamac_dof_lattice_cubic_encode(
                           &(tab->dof), (x - 1 + par->Lx) % par->Lx, y, z) +
                    i));
          }
        }
      }
      if (par->Ly > 1) {
        if (y < par->Ly - 1 || par->boundary_conditions == option_periodic) {
          if (G1MIG3[i][iloc]) {
            cval = -0.5 * par->t * G1MIG3[i][iloc];
            if (fl_transpose != fl_conjugate) {
              cval = conj(cval);
            }
            SCAMAC_APPROVE(scamac_sparserow_cplx_add(row,
                cval,
                nloc * scamac_dof_lattice_cubic_encode(
                           &(tab->dof), x, (y + 1) % par->Ly, z) +
                    i));
          }
        }
        if (y > 0 || par->boundary_conditions == option_periodic) {
          if (G1MIG3[iloc][i]) {
            cval = -0.5 * par->t * conj(G1MIG3[iloc][i]);
            if (fl_transpose != fl_conjugate) {
              cval = conj(cval);
            }
            SCAMAC_APPROVE(scamac_sparserow_cplx_add(row,
                cval,
                nloc * scamac_dof_lattice_cubic_encode(
                           &(tab->dof), x, (y - 1 + par->Ly) % par->Ly, z) +
                    i));
          }
        }
      }
      if (par->Lz > 1) {
        if (z < par->Lz - 1 || par->boundary_conditions == option_periodic) {
          if (G1MIG4[i][iloc]) {
            cval = -0.5 * par->t * G1MIG4[i][iloc];
            if (fl_transpose != fl_conjugate) {
              cval = conj(cval);
            }
            SCAMAC_APPROVE(scamac_sparserow_cplx_add(row,
                cval,
                nloc * scamac_dof_lattice_cubic_encode(
                           &(tab->dof), x, y, (z + 1) % par->Lz) +
                    i));
          }
        }
        if (z > 0 || par->boundary_conditions == option_periodic) {
          if (G1MIG4[iloc][i]) {
            cval = -0.5 * par->t * conj(G1MIG4[iloc][i]);
            if (fl_transpose != fl_conjugate) {
              cval = conj(cval);
            }
            SCAMAC_APPROVE(scamac_sparserow_cplx_add(row,
                cval,
                nloc * scamac_dof_lattice_cubic_encode(
                           &(tab->dof), x, y, (z - 1 + par->Lz) % par->Lz) +
                    i));
          }
        }
      }
    }
  }

  if (par->m != 0.0 || fl_keepzeros) {
    for (i = 0; i < nloc; i++) {
      if (G1[i][iloc] != 0.0) {
        cval = par->m * G1[i][iloc];
        if (fl_transpose != fl_conjugate) {
          cval = conj(cval);
        }
        SCAMAC_APPROVE(scamac_sparserow_cplx_add(row, cval, irow - iloc + i));
      }
    }
  }
  if (par->D1 != 0.0 || fl_keepzeros) {
    for (i = 0; i < nloc; i++) {
      if (G5[i][iloc] != 0.0) {
        cval = par->m * G5[i][iloc];
        if (fl_transpose != fl_conjugate) {
          cval = conj(cval);
        }
        SCAMAC_APPROVE(scamac_sparserow_cplx_add(row, cval, irow - iloc + i));
      }
    }
  }
  if (par->D2 != 0.0 || fl_keepzeros) {
    for (i = 0; i < nloc; i++) {
      if (G15[i][iloc] != 0.0) {
        cval = par->m * G15[i][iloc];
        if (fl_transpose != fl_conjugate) {
          cval = conj(cval);
        }
        SCAMAC_APPROVE(scamac_sparserow_cplx_add(row, cval, irow - iloc + i));
      }
    }
  }

  double val = 0.0;
  if (par->ranpot != 0.0) {
    val = scamac_ransrc_double(ws->rng, -par->ranpot, par->ranpot, irow);
  }
  if (val != 0.0 || fl_keepzeros) {
    SCAMAC_APPROVE(scamac_sparserow_cplx_add(row, val, irow));
  }

  return SCAMAC_EOK;
}
