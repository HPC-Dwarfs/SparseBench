/** \file
 *  \author Andreas Alvermann (University of Greifswald)
 *  \date   October 2017 --- today
 *  \brief  ...
 *  \ingroup internal
 */

#ifndef SCAMAC_SPARSEROW_H
#define SCAMAC_SPARSEROW_H

#include <complex.h>
#include <stdbool.h>

#include "scamac_error.h"
#include "scamac_inttypes.h"

typedef struct {
  ScamacIdx idx;
  double val;
} scamac_idxreal_st;

typedef struct {
  int nalloc;
  int nrow;
  scamac_idxreal_st *row;
} scamac_sparserow_real_st;

typedef struct {
  ScamacIdx idx;
  double complex val;
} scamac_idxcplx_st;

typedef struct {
  int nalloc;
  int nrow;
  scamac_idxcplx_st *row;
} scamac_sparserow_cplx_st;

/* sparse row routines */

ScamacErrorCode scamac_sparserow_real_alloc(ScamacIdx n, scamac_sparserow_real_st **srow);
ScamacErrorCode scamac_sparserow_cplx_alloc(ScamacIdx n, scamac_sparserow_cplx_st **srow);

ScamacErrorCode scamac_sparserow_real_free(scamac_sparserow_real_st *row);
ScamacErrorCode scamac_sparserow_cplx_free(scamac_sparserow_cplx_st *row);

ScamacErrorCode scamac_sparserow_real_zero(scamac_sparserow_real_st *row);
ScamacErrorCode scamac_sparserow_cplx_zero(scamac_sparserow_cplx_st *row);

/* if idx < 0, row is unchanged. This convention interacts nicely with scamac_multidx_upd() */
ScamacErrorCode scamac_sparserow_real_add(
    scamac_sparserow_real_st *row, double alpha, ScamacIdx idx);
ScamacErrorCode scamac_sparserow_cplx_add(
    scamac_sparserow_cplx_st *row, double complex alpha, ScamacIdx idx);

/* Sort and compress sparse row */
ScamacErrorCode scamac_sparserow_real_normalize(
    scamac_sparserow_real_st *row, bool keep_zeros);
ScamacErrorCode scamac_sparserow_cplx_normalize(
    scamac_sparserow_cplx_st *row, bool keep_zeros);

/* convert sparse row to idx/val pair of vectors. */
ScamacErrorCode scamac_sparserow_real_to_idxval(scamac_sparserow_real_st *row,
    ScamacIdx maxrowlength,
    ScamacIdx *nz,
    ScamacIdx *idx,
    double *val);
ScamacErrorCode scamac_sparserow_cplx_to_idxval(scamac_sparserow_cplx_st *row,
    ScamacIdx maxrowlength,
    ScamacIdx *nz,
    ScamacIdx *idx,
    double complex *val);

/* convert sparse row to idx [type:int]/val pair of vectors. */
ScamacErrorCode scamac_sparserow_real_to_idxval_int(scamac_sparserow_real_st *row,
    ScamacIdx maxrowlength,
    int *nz,
    int *idx,
    double *val);
ScamacErrorCode scamac_sparserow_cplx_to_idxval_int(scamac_sparserow_cplx_st *row,
    ScamacIdx maxrowlength,
    int *nz,
    int *idx,
    double complex *val);

#endif /* SCAMAC_SPARSEROW_H */
