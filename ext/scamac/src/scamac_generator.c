#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

#include "scamac_generator.h"

#include "scamac_internal.h"

ScamacErrorCode scamac_workspace_alloc(const ScamacGenerator *gen, ScamacWorkspace **ws)
{

  if (!gen) {
    return SCAMAC_ENULL | 1 << SCAMAC_ESHIFT;
  }
  if (!ws) {
    return SCAMAC_ENULL | 2 << SCAMAC_ESHIFT;
  }

  if (gen->needs_finalization) {
    return SCAMAC_EINVALID | 1 << SCAMAC_ESHIFT;
  }

  ScamacErrorCode err;

  ScamacWorkspace *myws;
  myws = malloc(sizeof *myws);
  if (!myws) {
    return SCAMAC_EMALLOCFAIL;
  }

  if (gen->fct_work_alloc) {
    err = gen->fct_work_alloc(gen->par, gen->tables, &(myws->ws));
    if (err) {
      return scamac_error_set_internal(err);
    }
    myws->fct_work_free = gen->fct_work_free;
  } else {
    myws->fct_work_free = NULL;
    myws->ws            = NULL;
  }

  if (gen->info.valtype == SCAMAC_VAL_REAL) {
    err            = scamac_sparserow_real_alloc(gen->info.maxnzgen, &(myws->row_real));
    myws->row_cplx = NULL;
    if (err) {
      return scamac_error_set_internal(err);
    }
  } else if (gen->info.valtype == SCAMAC_VAL_COMPLEX) {
    myws->row_real = NULL;
    err            = scamac_sparserow_cplx_alloc(gen->info.maxnzgen, &(myws->row_cplx));
    if (err) {
      return scamac_error_set_internal(err);
    }
  } else {
    return scamac_error_set_internal(SCAMAC_EFAIL);
  }

  *ws = myws;
  return SCAMAC_EOK;
}

ScamacErrorCode scamac_workspace_free(ScamacWorkspace *ws)
{
  ScamacErrorCode err;
  if (ws) {
    if (ws->fct_work_free && ws->ws) {
      err = ws->fct_work_free(ws->ws);
      if (err) {
        return scamac_error_set_internal(err);
      }
    }
    if (ws->row_real) {
      err = scamac_sparserow_real_free(ws->row_real);
      if (err) {
        return scamac_error_set_internal(err);
      }
    }
    if (ws->row_cplx) {
      err = scamac_sparserow_cplx_free(ws->row_cplx);
      if (err) {
        return scamac_error_set_internal(err);
      }
    }
    free(ws);
  }
  return SCAMAC_EOK;
}

ScamacErrorCode scamac_generator_check(const ScamacGenerator *gen, char **desc)
{
  ScamacErrorCode err;

  if (!gen) {
    return SCAMAC_ENULL | 1 << SCAMAC_ESHIFT;
  }

  if (gen->wrapped_par) {
    if (gen->fct_check_wrapped) {
      if (desc) {
        *desc = NULL;
      }
      err = gen->fct_check_wrapped(gen->wrapped_par, desc);
      if (err) {
        return err;
      }
    }
    if (!gen->needs_unwrapping) {
      err = gen->fct_check(gen->par, NULL);
      if (err) {
        return scamac_error_set_internal(err);
      }
    }
    return SCAMAC_EOK;
  }

  if (gen->par) {
    if (gen->fct_check) {
      return gen->fct_check(gen->par, desc);
    } else {
      if (*desc) {
        *desc = malloc(100 * sizeof **desc);
        snprintf(*desc, 100, "Oops - internal error in: scamac_generator_check");
      }
      return scamac_error_set_internal(SCAMAC_EFAIL);
    }
  } else {
    if (desc) {
      *desc = NULL;
    }
    return SCAMAC_EOK;
  }
}

ScamacErrorCode scamac_generator_destroy(ScamacGenerator *gen)
{
  ScamacErrorCode err;
  if (gen) {
    if (gen->fct_tables_destroy && gen->tables) {
      err = gen->fct_tables_destroy(gen->tables);
      if (err) {
        return err;
      }
    }
    if (gen->par) {
      free(gen->par);
    }
    if (gen->wrapped_par) {
      free(gen->wrapped_par);
    }
    free(gen);
  }
  return SCAMAC_EOK;
}

ScamacErrorCode scamac_generator_finalize(ScamacGenerator *gen)
{
  ScamacErrorCode err;

  if (!gen) {
    return SCAMAC_ENULL | 1 << SCAMAC_ESHIFT;
  }

  if (gen->needs_finalization) {
    err = scamac_generator_check(gen, NULL);
    if (scamac_error_discard_warning(err)) {
      return err;
    }

    if (gen->fct_unwrap_par && gen->wrapped_par) {
      err = gen->fct_unwrap_par(gen->wrapped_par, gen->par);
      if (scamac_error_discard_warning(err)) {
        return err;
      }
      gen->needs_unwrapping = false;
      err                   = scamac_generator_check(gen, NULL);
      assert(err == SCAMAC_EOK);
      if (scamac_error_discard_warning(err)) {
        return scamac_error_set_internal(err);
      }
    }

    if (gen->fct_tables_destroy && gen->tables) {
      err = gen->fct_tables_destroy(gen->tables);
      if (err) {
        return err;
      }
      gen->tables = NULL;
    }
    if (gen->fct_tables_create) {
      err = gen->fct_tables_create(gen->par, &(gen->tables), &(gen->info));
      if (err) {
        return err;
      }
    }

    gen->needs_finalization = false;
    return SCAMAC_EOK;
  } else {
    return SCAMAC_EOK;
  }
}

ScamacIdx scamac_generator_query_nrow(const ScamacGenerator *gen)
{
  if (gen) {
    return gen->info.nrow;
  } else {
    return 0;
  }
}

ScamacIdx scamac_generator_query_ncol(const ScamacGenerator *gen)
{
  if (gen) {
    return gen->info.ncol;
  } else {
    return 0;
  }
}

ScamacIdx scamac_generator_query_maxnzrow(const ScamacGenerator *gen)
{
  if (gen) {
    return gen->info.maxnzrow;
  } else {
    return 0;
  }
}

ScamacIdx scamac_generator_query_maxnzcol(const ScamacGenerator *gen)
{
  if (gen) {
    return gen->info.maxnzcol;
  } else {
    return 0;
  }
}

ScamacIdx scamac_generator_query_maxnz(const ScamacGenerator *gen)
{
  if (gen) {
    return gen->info.maxnz;
  } else {
    return 0;
  }
}

ScamacIdx scamac_generator_query_valtype(const ScamacGenerator *gen)
{
  if (gen) {
    return gen->info.valtype;
  } else {
    return SCAMAC_NONE;
  }
}

ScamacIdx scamac_generator_query_symmetry(const ScamacGenerator *gen)
{
  if (gen) {
    return gen->info.symmetry;
  } else {
    return SCAMAC_NONE;
  }
}

const char *scamac_generator_query_name(const ScamacGenerator *gen)
{
  static const char msg[] = "Unknown example";
  if (gen) {
    if (gen->name) {
      return gen->name;
    } else {
      return msg;
    }
  } else {
    return msg;
  }
}

int scamac_generator_query_coorddim(const ScamacGenerator *gen)
{
  if (gen) {
    if (gen->fct_coord) {
      ScamacErrorCode err;
      int iil;
      err = gen->fct_coord(gen->par, gen->tables, NULL, -1, NULL, NULL, NULL, &iil, NULL);
      if (err) {
        return 0;
      } else {
        return iil;
      }
    } else {
      return 0;
    }
  } else {
    return 0;
  }
}

bool scamac_generator_get_coord(const ScamacGenerator *gen,
    ScamacWorkspace *ws,
    ScamacIdx idx,
    double *x,
    double *y,
    double *z,
    int *ilayer,
    ScamacIdx *iblock)
{
  if (gen) {
    if (gen->fct_coord) {
      if ((idx < 0) || (idx >= gen->info.nrow)) {
        return false;
      }
      ScamacErrorCode err;
      double xx, yy, zz;
      int iil;
      ScamacIdx iib;
      err = gen->fct_coord(gen->par, gen->tables, ws->ws, idx, &xx, &yy, &zz, &iil, &iib);
      if (err) {
        return false;
      } else {
        if (x) {
          *x = xx;
        }
        if (y) {
          *y = yy;
        }
        if (z) {
          *z = zz;
        }
        if (ilayer) {
          *ilayer = iil;
        }
        if (iblock) {
          *iblock = iib;
        }
        return true;
      }
    } else {
      return false;
    }
  } else {
    return false;
  }
}

ScamacErrorCode scamac_generate_row(const ScamacGenerator *gen,
    ScamacWorkspace *ws,
    ScamacIdx irow,
    ScamacFlag flag,
    ScamacIdx *nzr,
    ScamacIdx *cind,
    double *val)
{
  ScamacErrorCode err;

  if (flag & ~SCAMAC_TRANSPOSE & ~SCAMAC_CONJUGATE & ~SCAMAC_KEEPZEROS) {
    return SCAMAC_ERANGE;
  }
  bool fl_transpose = (flag & SCAMAC_TRANSPOSE) != 0;
  bool fl_keepzeros = (flag & SCAMAC_KEEPZEROS) != 0;

  if (!gen) {
    return SCAMAC_ENULL;
  }
  if (gen->needs_finalization) {
    return SCAMAC_EFAIL;
  }
  if (!gen->fct_gen_row) {
    return scamac_error_set_internal(SCAMAC_ENULL);
  }

  if (!nzr) {
    return SCAMAC_ENULL;
  }
  if (val && (!cind)) {
    return SCAMAC_ENULL;
  }
  if (irow < 0) {
    return SCAMAC_ERANGE;
  }

  if (!ws) {
    return SCAMAC_ENULL;
  }
  if ((!ws->row_real) && (!ws->row_cplx)) {
    return scamac_error_set_internal(SCAMAC_ENULL);
  }
  if (ws->row_real && ws->row_cplx) {
    return scamac_error_set_internal(SCAMAC_EINVALID);
  }

  ScamacIdx maxnzr;
  if (fl_transpose) {
    if ((irow < 0) || (irow >= gen->info.ncol)) {
      return SCAMAC_ERANGE;
    }
    maxnzr = gen->info.maxnzcol;
  } else {
    if ((irow < 0) || (irow >= gen->info.nrow)) {
      return SCAMAC_ERANGE;
    }
    maxnzr = gen->info.maxnzrow;
  }

  if (ws->row_real) {
    err = scamac_sparserow_real_zero(ws->row_real);
    if (err) {
      return scamac_error_set_internal(err);
    }

    err = gen->fct_gen_row(gen->par, gen->tables, ws->ws, irow, flag, ws->row_real);
    if (err) {
      return scamac_error_set_internal(err);
    }

    err = scamac_sparserow_real_normalize(ws->row_real, fl_keepzeros);
    if (err) {
      return scamac_error_set_internal(err);
    }

    err = scamac_sparserow_real_to_idxval(ws->row_real, maxnzr, nzr, cind, val);
    if (err) {
      return scamac_error_set_internal(err);
    }

  } else if (ws->row_cplx) {
    err = scamac_sparserow_cplx_zero(ws->row_cplx);
    if (err) {
      return scamac_error_set_internal(err);
    }

    err = gen->fct_gen_row(gen->par, gen->tables, ws->ws, irow, flag, ws->row_cplx);
    if (err) {
      return scamac_error_set_internal(err);
    }

    err = scamac_sparserow_cplx_normalize(ws->row_cplx, fl_keepzeros);
    if (err) {
      return scamac_error_set_internal(err);
    }

    err = scamac_sparserow_cplx_to_idxval(
        ws->row_cplx, maxnzr, nzr, cind, (double complex *)val);
    if (err) {
      return scamac_error_set_internal(err);
    }
  }

  return SCAMAC_EOK;
}

ScamacErrorCode scamac_alloc_cind_val(
    const ScamacGenerator *gen, ScamacFlag flag, ScamacIdx **cind, double **val)
{
  if (!gen) {
    return SCAMAC_ENULL;
  }
  if (gen->needs_finalization) {
    return SCAMAC_EFAIL;
  }

  if (flag & ~SCAMAC_TRANSPOSE & ~SCAMAC_CONJUGATE & ~SCAMAC_KEEPZEROS) {
    return SCAMAC_ERANGE;
  }
  bool fl_transpose = (flag & SCAMAC_TRANSPOSE) != 0;

  ScamacIdx maxnzr;
  if (fl_transpose) {
    maxnzr = gen->info.maxnzcol;
  } else {
    maxnzr = gen->info.maxnzrow;
  }

  if (cind) {
    if (maxnzr > 0) {
      *cind = malloc(maxnzr * sizeof **cind);
      if (!*cind) {
        return SCAMAC_EMALLOCFAIL;
      }
    } else {
      *cind = NULL;
    }
  }
  if (val) {
    if (maxnzr > 0) {
      if (gen->info.valtype == SCAMAC_VAL_REAL) {
        *val = malloc(maxnzr * sizeof **val);
      } else if (gen->info.valtype == SCAMAC_VAL_COMPLEX) {
        *val = malloc(2 * maxnzr * sizeof **val);
      } else {
        return scamac_error_set_internal(SCAMAC_EFAIL);
      }
    } else {
      *val = NULL;
    }
    if (!*val) {
      return SCAMAC_EMALLOCFAIL;
    }
  }
  return SCAMAC_EOK;
}

ScamacErrorCode scamac_generate_row_real(const ScamacGenerator *gen,
    ScamacWorkspace *ws,
    ScamacIdx irow,
    ScamacFlag flag,
    ScamacIdx *nzr,
    ScamacIdx *cind,
    double *val)
{
  if (!gen) {
    return SCAMAC_ENULL;
  }
  if (gen->needs_finalization) {
    return SCAMAC_EFAIL;
  }
  if (gen->info.valtype != SCAMAC_VAL_REAL) {
    return SCAMAC_EFAIL;
  }
  ScamacErrorCode err;
  err = scamac_generate_row(gen, ws, irow, flag, nzr, cind, val);
  return err;
}

ScamacErrorCode scamac_generate_row_cplx(const ScamacGenerator *gen,
    ScamacWorkspace *ws,
    ScamacIdx irow,
    ScamacFlag flag,
    ScamacIdx *nzr,
    ScamacIdx *cind,
    double complex *val)
{
  if (!gen) {
    return SCAMAC_ENULL;
  }
  if (gen->needs_finalization) {
    return SCAMAC_EFAIL;
  }
  if (gen->info.valtype != SCAMAC_VAL_COMPLEX) {
    return SCAMAC_EFAIL;
  }
  ScamacErrorCode err;
  err = scamac_generate_row(gen, ws, irow, flag, nzr, cind, (double *)val);
  return err;
}

ScamacErrorCode scamac_generate_row_int(const ScamacGenerator *gen,
    ScamacWorkspace *ws,
    ScamacIdx irow,
    ScamacFlag flag,
    int *nzr,
    int *cind,
    double *val)
{

  ScamacErrorCode err;

  if (flag & ~SCAMAC_TRANSPOSE & ~SCAMAC_CONJUGATE & ~SCAMAC_KEEPZEROS) {
    return SCAMAC_ERANGE;
  }
  bool fl_transpose = (flag & SCAMAC_TRANSPOSE) != 0;
  bool fl_keepzeros = (flag & SCAMAC_KEEPZEROS) != 0;

  if (!gen) {
    return SCAMAC_ENULL;
  }
  if (gen->needs_finalization) {
    return SCAMAC_EFAIL;
  }
  if (!gen->fct_gen_row) {
    return scamac_error_set_internal(SCAMAC_ENULL);
  }

  if (!nzr) {
    return SCAMAC_ENULL;
  }
  if (val && (!cind)) {
    return SCAMAC_ENULL;
  }
  if (irow < 0) {
    return SCAMAC_ERANGE;
  }

  if (!ws) {
    return SCAMAC_ENULL;
  }
  if ((!ws->row_real) && (!ws->row_cplx)) {
    return scamac_error_set_internal(SCAMAC_ENULL);
  }
  if (ws->row_real && ws->row_cplx) {
    return scamac_error_set_internal(SCAMAC_EINVALID);
  }

  if ((gen->info.ncol > INT_MAX) || (gen->info.nrow > INT_MAX)) {
    return SCAMAC_EOVERFLOW;
  }

  ScamacIdx maxnzr;
  if (fl_transpose) {
    if ((irow < 0) || (irow >= gen->info.ncol)) {
      return SCAMAC_ERANGE;
    }
    maxnzr = gen->info.maxnzcol;
  } else {
    if ((irow < 0) || (irow >= gen->info.nrow)) {
      return SCAMAC_ERANGE;
    }
    maxnzr = gen->info.maxnzrow;
  }

  if (ws->row_real) {
    err = scamac_sparserow_real_zero(ws->row_real);
    if (err) {
      return scamac_error_set_internal(err);
    }

    err = gen->fct_gen_row(gen->par, gen->tables, ws->ws, irow, flag, ws->row_real);
    if (err) {
      return scamac_error_set_internal(err);
    }

    err = scamac_sparserow_real_normalize(ws->row_real, fl_keepzeros);
    if (err) {
      return scamac_error_set_internal(err);
    }

    err = scamac_sparserow_real_to_idxval_int(ws->row_real, maxnzr, nzr, cind, val);
    if (err) {
      return scamac_error_set_internal(err);
    }

  } else if (ws->row_cplx) {
    err = scamac_sparserow_cplx_zero(ws->row_cplx);
    if (err) {
      return scamac_error_set_internal(err);
    }

    err = gen->fct_gen_row(gen->par, gen->tables, ws->ws, irow, flag, ws->row_cplx);
    if (err) {
      return scamac_error_set_internal(err);
    }

    err = scamac_sparserow_cplx_normalize(ws->row_cplx, fl_keepzeros);
    if (err) {
      return scamac_error_set_internal(err);
    }

    err = scamac_sparserow_cplx_to_idxval_int(
        ws->row_cplx, maxnzr, nzr, cind, (double complex *)val);
    if (err) {
      return scamac_error_set_internal(err);
    }
  }

  return SCAMAC_EOK;
}

ScamacErrorCode scamac_generate_row_int_real(const ScamacGenerator *gen,
    ScamacWorkspace *ws,
    ScamacIdx irow,
    ScamacFlag flag,
    int *nzr,
    int *cind,
    double *val)
{
  if (!gen) {
    return SCAMAC_ENULL;
  }
  if (gen->needs_finalization) {
    return SCAMAC_EFAIL;
  }
  if (gen->info.valtype != SCAMAC_VAL_REAL) {
    return SCAMAC_EFAIL;
  }
  ScamacErrorCode err;
  err = scamac_generate_row_int(gen, ws, irow, flag, nzr, cind, val);
  return err;
}

ScamacErrorCode scamac_generate_row_int_cplx(const ScamacGenerator *gen,
    ScamacWorkspace *ws,
    ScamacIdx irow,
    ScamacFlag flag,
    int *nzr,
    int *cind,
    double complex *val)
{
  if (!gen) {
    return SCAMAC_ENULL;
  }
  if (gen->needs_finalization) {
    return SCAMAC_EFAIL;
  }
  if (gen->info.valtype != SCAMAC_VAL_COMPLEX) {
    return SCAMAC_EFAIL;
  }
  ScamacErrorCode err;
  err = scamac_generate_row_int(gen, ws, irow, flag, nzr, cind, (double *)val);
  return err;
}
