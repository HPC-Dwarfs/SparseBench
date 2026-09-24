/* Copyright (C) NHR@FAU, University Erlangen-Nuremberg.
 * All rights reserved. This file is part of SparseBench.
 * Use of this source code is governed by a MIT style
 * license that can be found in the LICENSE file. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "allocate.h"
#include "matrix.h"

#ifdef _OPENMP
#define OMP_PARFOR _Pragma("omp parallel for schedule(OMP_SCHEDULE)")
#else
#define OMP_PARFOR
#endif

/* Allocate every format-specific array of m in one place. Assumes the size
 * scalars (nr, nnz) are already set on m. Pairs with freeMatrix.
 * boundaryRows is sized by convertMatrix once nBoundaryRows is known. */
void allocMatrix(Matrix *m)
{
  m->rowPtr       = (CG_UINT *)allocate(ARRAY_ALIGNMENT, (m->nr + 1) * sizeof(CG_UINT));
  m->colInd       = (CG_UINT *)allocate(ARRAY_ALIGNMENT, m->nnz * sizeof(CG_UINT));
  m->val          = (V_ELE *)allocate(ARRAY_ALIGNMENT, m->nnz * sizeof(V_ELE));
  m->rowLocalEnd  = (CG_UINT *)allocate(ARRAY_ALIGNMENT, m->nr * sizeof(CG_UINT));
  m->boundaryRows = NULL;
}

/* Free the arrays allocated by allocMatrix and convertMatrix. */
void freeMatrix(Matrix *m)
{
  deallocate(m->rowPtr);
  deallocate(m->colInd);
  deallocate(m->val);
  deallocate(m->rowLocalEnd);
  if (m->boundaryRows != NULL) {
    deallocate(m->boundaryRows);
  }
}

void convertMatrix(Matrix *sm, GMatrix *m)
{
  sm->startRow = m->startRow;
  sm->stopRow  = m->stopRow;
  sm->totalNr  = m->totalNr;
  sm->totalNnz = m->totalNnz;
  sm->nr       = m->nr;
  sm->nc       = m->nc;
  sm->nnz      = m->nnz;

  allocMatrix(sm);

  Entry *entries  = m->entries;

  CG_UINT numRows = m->nr;

  // Convert to CRS format. Parallel row loop with the same schedule the
  // spMVM kernel uses, so val/colInd/rowPtr pages are first-touched on
  // the NUMA node of the thread that will later read them.
  OMP_PARFOR
  for (CG_UINT rowID = 0; rowID < numRows; rowID++) {
    sm->rowPtr[rowID] = m->rowPtr[rowID];

    // rowLocalEnd is set by reorderMatrixForOverlap during commLocalization
    sm->rowLocalEnd[rowID] =
        m->rowLocalEnd ? m->rowLocalEnd[rowID] : m->rowPtr[rowID + 1];

    // loop over all elements in Row
    for (CG_UINT id = m->rowPtr[rowID]; id < m->rowPtr[rowID + 1]; id++) {
      sm->val[id]    = entries[id].val;
      sm->colInd[id] = (CG_UINT)entries[id].col;
    }
  }

  sm->rowPtr[numRows] = m->rowPtr[numRows];

  // Boundary rows are also produced by reorderMatrixForOverlap. Without
  // localization there are no external entries at all, so the list is empty.
  sm->nBoundaryRows = m->boundaryRows ? m->nBoundaryRows : 0;
  sm->boundaryRows  = NULL;

  if (sm->nBoundaryRows > 0) {
    sm->boundaryRows =
        (CG_UINT *)allocate(ARRAY_ALIGNMENT, sm->nBoundaryRows * sizeof(CG_UINT));

    for (CG_UINT i = 0; i < sm->nBoundaryRows; i++) {
      sm->boundaryRows[i] = m->boundaryRows[i];
    }
  }
}

void spMVM(Matrix *m, const V_ELE *restrict x, V_ELE *restrict y)
{
  CG_UINT *colInd = m->colInd;
  V_ELE *val      = m->val;

  CG_UINT numRows = m->nr;
  CG_UINT *rowPtr = m->rowPtr;

  OMP_PARFOR
  for (CG_UINT i = 0; i < numRows; i++) {
    V_ELE sum = 0.0;

    // loop over all elements in row
    for (CG_UINT j = rowPtr[i]; j < rowPtr[i + 1]; j++) {
      sum += val[j] * x[colInd[j]];
    }

    y[i] = sum;
  }
}

/**
 * @brief Sparse Matrix-Vector Multiply: LOCAL entries of a row range.
 *
 * Computes the SpMV contribution of rows [rowStart, rowEnd) from entries whose
 * column index is in the local range [0, numRows-1]. Entries from external
 * columns (col >= numRows) are excluded, so this may run while a halo exchange
 * is in flight.
 *
 * y[i] is overwritten (not accumulated) for every row in the range, which is
 * what makes the subsequent accumulating spMVM_external correct.
 *
 * Splitting the local phase into ranges lets the caller re-enter MPI between
 * chunks to drive progress on the in-flight exchange.
 *
 * @param m        Matrix in CRS format (must have rowLocalEnd set)
 * @param x        Input vector (size = ncol = numRows + numExternals)
 * @param y        Output vector (overwritten in [rowStart, rowEnd))
 * @param rowStart First row of the range
 * @param rowEnd   One past the last row of the range
 */
void spMVM_local_range(const Matrix *m,
    const V_ELE *restrict x,
    V_ELE *restrict y,
    CG_UINT rowStart,
    CG_UINT rowEnd)
{
  CG_UINT *colInd      = m->colInd;
  V_ELE *val           = m->val;
  CG_UINT *rowPtr      = m->rowPtr;
  CG_UINT *rowLocalEnd = m->rowLocalEnd;

  OMP_PARFOR
  for (CG_UINT i = rowStart; i < rowEnd; i++) {
    V_ELE sum = 0.0;

    // loop over LOCAL elements in row only (col < numRows)
    for (CG_UINT j = rowPtr[i]; j < rowLocalEnd[i]; j++) {
      sum += val[j] * x[colInd[j]];
    }

    y[i] = sum;
  }
}

/**
 * @brief Sparse Matrix-Vector Multiply: LOCAL entries only.
 *
 * Convenience wrapper around spMVM_local_range covering all local rows.
 *
 * @param m   Matrix in CRS format (must have rowLocalEnd set)
 * @param x   Input vector (size = ncol = numRows + numExternals)
 * @param y   Output vector (overwritten)
 */
void spMVM_local(const Matrix *m, const V_ELE *restrict x, V_ELE *restrict y)
{
  spMVM_local_range(m, x, y, 0, m->nr);
}

/**
 * @brief Sparse Matrix-Vector Multiply: EXTERNAL entries only.
 *
 * Computes SpMV contribution from entries whose column index is in the external
 * range [numRows, numRows+extCount-1]. Entries from local columns (col < numRows)
 * are excluded. Results accumulate onto existing y[i] values (must be called
 * AFTER spMVM_local in the overlapped CG iteration).
 *
 * Only rows that actually own external entries are visited; boundaryRows holds
 * that list, which is typically a small fraction of all local rows.
 *
 * @param m   Matrix in CRS format (must have rowLocalEnd and boundaryRows set)
 * @param x   Input vector (size = ncol = numRows + numExternals)
 * @param y   Output vector (accumulates onto existing values from spMVM_local)
 */
void spMVM_external(const Matrix *m, const V_ELE *restrict x, V_ELE *restrict y)
{
  CG_UINT *colInd       = m->colInd;
  V_ELE *val            = m->val;
  CG_UINT *rowPtr       = m->rowPtr;
  CG_UINT *rowLocalEnd  = m->rowLocalEnd;
  CG_UINT *boundaryRows = m->boundaryRows;
  CG_UINT nBoundaryRows = m->nBoundaryRows;

  OMP_PARFOR
  for (CG_UINT r = 0; r < nBoundaryRows; r++) {
    CG_UINT i = boundaryRows[r];
    V_ELE sum = 0.0;

    // loop over EXTERNAL elements in row only (col >= numRows)
    for (CG_UINT j = rowLocalEnd[i]; j < rowPtr[i + 1]; j++) {
      sum += val[j] * x[colInd[j]];
    }

    y[i] += sum;
  }
}

void spMMVM(Matrix *m, const DMatrix *x, DMatrix *y)
{
  CG_UINT *colInd = m->colInd;
  V_ELE *val      = m->val;

  CG_UINT numRows = m->nr;
  CG_UINT *rowPtr = m->rowPtr;

  OMP_PARFOR
  for (CG_UINT row = 0; row < numRows; row++) {
    V_ELE *y_row = &y->entries[row * y->nc];

    /* initialize output row before accumulation */
#pragma omp simd
    for (size_t c = 0; c < y->nc; c++)
      y_row[c] = 0.0;

    /* loop over all elements in row and accumulate the scaled x[col] row */
    for (CG_UINT j = rowPtr[row]; j < rowPtr[row + 1]; j++) {
      CG_UINT col  = colInd[j];
      V_ELE *x_col = &x->entries[col * x->nc];
      V_ELE a      = val[j];
#pragma omp simd
      for (size_t c = 0; c < x->nc; c++)
        y_row[c] += a * x_col[c];
    }
  }
}

/* Fused y = cA*(m*x) + cP*p + cQ*q, evaluated per row without ever writing
 * m*x out to memory. q may be NULL (with cQ ignored). */
void spMMVMFused(Matrix *m,
    const DMatrix *x,
    V_ELE cA,
    const DMatrix *p,
    V_ELE cP,
    const DMatrix *q,
    V_ELE cQ,
    DMatrix *y)
{
  CG_UINT *colInd = m->colInd;
  V_ELE *val      = m->val;

  CG_UINT numRows = m->nr;
  CG_UINT *rowPtr = m->rowPtr;
  CG_UINT nc      = x->nc;

#pragma omp parallel for schedule(OMP_SCHEDULE)
  for (CG_UINT row = 0; row < numRows; row++) {
    V_ELE acc[nc];
#pragma omp simd
    for (size_t c = 0; c < nc; c++)
      acc[c] = 0.0;

    for (CG_UINT j = rowPtr[row]; j < rowPtr[row + 1]; j++) {
      CG_UINT col  = colInd[j];
      V_ELE *x_col = &x->entries[col * nc];
      V_ELE a      = val[j];
#pragma omp simd
      for (size_t c = 0; c < nc; c++)
        acc[c] += a * x_col[c];
    }

    V_ELE *y_row = &y->entries[(CG_UINT)row * nc];
    V_ELE *p_row = &p->entries[(CG_UINT)row * nc];
    if (q != NULL) {
      V_ELE *q_row = &q->entries[(CG_UINT)row * nc];
#pragma omp simd
      for (size_t c = 0; c < nc; c++)
        y_row[c] = cA * acc[c] + cP * p_row[c] + cQ * q_row[c];
    } else {
#pragma omp simd
      for (size_t c = 0; c < nc; c++)
        y_row[c] = cA * acc[c] + cP * p_row[c];
    }
  }
}

/* ChebFD recurrence step, fully fused: computes the new filter term
 * y = cA*(m*w) + cP*w + cQ*q (same shape as spMMVMFused with p=w), and in the
 * same row pass accumulates it into the running polynomial sum,
 * x += gc*y. y may alias q (row-local, in-place recurrence update); x is a
 * separate accumulator block. Saves the extra read of y that a follow-up
 * waxpby(x, gc, y, x) would otherwise need, since y is still local here. */
void chebfdOp(Matrix *m,
    const DMatrix *w,
    V_ELE cA,
    V_ELE cP,
    const DMatrix *q,
    V_ELE cQ,
    DMatrix *y,
    V_ELE gc,
    DMatrix *x)
{
  CG_UINT *colInd = m->colInd;
  V_ELE *val      = m->val;

  CG_UINT numRows = m->nr;
  CG_UINT *rowPtr = m->rowPtr;
  CG_UINT nc      = w->nc;

#pragma omp parallel for schedule(OMP_SCHEDULE)
  for (CG_UINT row = 0; row < numRows; row++) {
    V_ELE acc[nc];
#pragma omp simd
    for (size_t c = 0; c < nc; c++)
      acc[c] = 0.0;

    for (CG_UINT j = rowPtr[row]; j < rowPtr[row + 1]; j++) {
      CG_UINT col  = colInd[j];
      V_ELE *w_col = &w->entries[col * nc];
      V_ELE a      = val[j];
#pragma omp simd
      for (size_t c = 0; c < nc; c++)
        acc[c] += a * w_col[c];
    }

    V_ELE *w_row = &w->entries[(CG_UINT)row * nc];
    V_ELE *y_row = &y->entries[(CG_UINT)row * nc];
    V_ELE *x_row = &x->entries[(CG_UINT)row * nc];
    if (q != NULL) {
      V_ELE *q_row = &q->entries[(CG_UINT)row * nc];
#pragma omp simd
      for (size_t c = 0; c < nc; c++) {
        V_ELE t  = cA * acc[c] + cP * w_row[c] + cQ * q_row[c];
        y_row[c] = t;
        x_row[c] += gc * t;
      }
    } else {
#pragma omp simd
      for (size_t c = 0; c < nc; c++) {
        V_ELE t  = cA * acc[c] + cP * w_row[c];
        y_row[c] = t;
        x_row[c] += gc * t;
      }
    }
  }
}
