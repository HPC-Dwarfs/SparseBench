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
 * scalars (nr, nnz) are already set on m. Pairs with freeMatrix. */
void allocMatrix(Matrix *m)
{
  m->rowPtr  = (CG_UINT *)allocate(ARRAY_ALIGNMENT, (m->nr + 1) * sizeof(CG_UINT));
  m->entries = (mEntry *)allocate(ARRAY_ALIGNMENT, m->nnz * sizeof(mEntry));
}

/* Free the arrays allocated by allocMatrix. */
void freeMatrix(Matrix *m)
{
  deallocate(m->rowPtr);
  deallocate(m->entries);
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

  for (CG_UINT rowID = 0; rowID < numRows; rowID++) {
    sm->rowPtr[rowID] = m->rowPtr[rowID];

    for (CG_UINT id = m->rowPtr[rowID]; id < m->rowPtr[rowID + 1]; id++) {
      sm->entries[id].val = entries[id].val;
      sm->entries[id].col = entries[id].col;
    }
  }

  sm->rowPtr[numRows] = m->rowPtr[numRows];
}

void spMVM(Matrix *m, const V_ELE *restrict x, V_ELE *restrict y)
{
  CG_UINT numRows = m->nr;
  CG_UINT *rowPtr = m->rowPtr;
  mEntry *entries = m->entries;

  OMP_PARFOR
  for (CG_UINT i = 0; i < numRows; i++) {
    V_ELE sum = 0.0;

    // loop over all elements in row
    for (CG_UINT j = rowPtr[i]; j < rowPtr[i + 1]; j++) {
      sum += entries[j].val * x[entries[j].col];
    }

    y[i] = sum;
  }
}

void spMMVM(Matrix *m, const DMatrix *x, DMatrix *y)
{
  CG_UINT numRows = m->nr;
  CG_UINT *rowPtr = m->rowPtr;
  mEntry *entries = m->entries;

  OMP_PARFOR
  for (CG_UINT row = 0; row < numRows; row++) {
    V_ELE *y_row = &y->entries[row * y->nc];

    /* initialize output row before accumulation */
    for (size_t c = 0; c < y->nc; c++)
      y_row[c] = 0.0;

    /* loop over all elements in row and accumulate the scaled x[col] row */
    for (CG_UINT j = rowPtr[row]; j < rowPtr[row + 1]; j++) {
      CG_UINT col  = entries[j].col;
      V_ELE *x_col = &x->entries[col * x->nc];
      V_ELE a      = entries[j].val;
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
  CG_UINT numRows = m->nr;
  CG_UINT *rowPtr = m->rowPtr;
  mEntry *entries = m->entries;
  CG_UINT nc      = x->nc;

  OMP_PARFOR
  for (CG_UINT row = 0; row < numRows; row++) {
    V_ELE acc[nc];
    for (size_t c = 0; c < nc; c++)
      acc[c] = 0.0;

    for (CG_UINT j = rowPtr[row]; j < rowPtr[row + 1]; j++) {
      V_ELE *x_col = &x->entries[entries[j].col * nc];
      V_ELE a      = entries[j].val;
      for (size_t c = 0; c < nc; c++)
        acc[c] += a * x_col[c];
    }

    V_ELE *y_row = &y->entries[row * nc];
    V_ELE *p_row = &p->entries[row * nc];
    if (q != NULL) {
      V_ELE *q_row = &q->entries[row * nc];
      for (size_t c = 0; c < nc; c++)
        y_row[c] = cA * acc[c] + cP * p_row[c] + cQ * q_row[c];
    } else {
      for (size_t c = 0; c < nc; c++)
        y_row[c] = cA * acc[c] + cP * p_row[c];
    }
  }
}

/* ChebFD recurrence step: y = cA*(m*w) + cP*w + cQ*q, fused with the
 * accumulate x += gc*y in the same row pass. See matrix-CRS.c. */
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
  CG_UINT numRows = m->nr;
  CG_UINT *rowPtr = m->rowPtr;
  mEntry *entries = m->entries;
  CG_UINT nc      = w->nc;

  OMP_PARFOR
  for (CG_UINT row = 0; row < numRows; row++) {
    V_ELE acc[nc];
    for (size_t c = 0; c < nc; c++)
      acc[c] = 0.0;

    for (CG_UINT j = rowPtr[row]; j < rowPtr[row + 1]; j++) {
      V_ELE *w_col = &w->entries[entries[j].col * nc];
      V_ELE a      = entries[j].val;
      for (size_t c = 0; c < nc; c++)
        acc[c] += a * w_col[c];
    }

    V_ELE *w_row = &w->entries[row * nc];
    V_ELE *y_row = &y->entries[row * nc];
    V_ELE *x_row = &x->entries[row * nc];
    for (size_t c = 0; c < nc; c++) {
      V_ELE t = cA * acc[c] + cP * w_row[c];
      if (q != NULL) {
        t += cQ * q->entries[row * nc + c];
      }
      y_row[c] = t;
      x_row[c] += gc * t;
    }
  }
}
