/* Copyright (C) NHR@FAU, University Erlangen-Nuremberg.
 * All rights reserved. This file is part of SparseBench.
 * Use of this source code is governed by a MIT style
 * license that can be found in the LICENSE file. */
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "allocate.h"
#include "comm.h"
#include "cuda/kernel_dispatch.h"
#include "cuda/nvtx_marker.h"
#include "profiler.h"
#include "solver.h"
#include "timing.h"
#include "vtype.h"

static void initVectors(Matrix *m, V_ELE *x, V_ELE *b, V_ELE *xexact)
{
#if defined(CRS) || defined(CCRS)
  CG_UINT numRows = m->nr;
  CG_UINT *rowPtr = m->rowPtr;

  // Parallel init for NUMA first-touch — must use the same schedule the
  // kernels use (OMP_SCHEDULE, kept as static for first-touch correctness).
#pragma omp parallel for schedule(OMP_SCHEDULE)
  for (CG_UINT rowID = 0; rowID < numRows; rowID++) {

    CG_UINT nnzrow = rowPtr[rowID + 1] - rowPtr[rowID];
    x[rowID]       = 0.0;

    if (xexact != NULL) {
      b[rowID]      = 27.0 - ((CG_FLOAT)(nnzrow - 1));
      xexact[rowID] = 1.0;
    } else {
      b[rowID] = 1.0;
    }
  }
#elif SCS
  CG_UINT numRows       = m->nr;
  CG_UINT c             = m->C;
  CG_UINT *chunkPtr     = m->chunkPtr;
  CG_UINT *chunkLens    = m->chunkLens;
  CG_UINT *colInd       = m->colInd;
  V_ELE *val            = m->val;
  CG_UINT *oldToNewPerm = m->oldToNewPerm;

  // Parallel init for NUMA first-touch — see CRS branch above.
#pragma omp parallel for schedule(OMP_SCHEDULE)
  for (CG_UINT rowID = 0; rowID < numRows; rowID++) {
    x[rowID] = 0.0;

    // Map original row to new row position in SCS format
    CG_UINT newRow     = oldToNewPerm[rowID];
    CG_UINT chunkIdx   = newRow / c;
    CG_UINT chunkRow   = newRow % c;
    CG_UINT chunkStart = chunkPtr[chunkIdx];
    CG_UINT rowLen     = chunkLens[chunkIdx];

    // Count actual non-zero values in this row
    int nnzrow = 0;
    for (CG_UINT j = 0; j < rowLen; ++j) {
      CG_UINT idx = chunkStart + j * c + chunkRow;
#ifdef USE_COMPLEX
      if (VREAL(val[idx]) != 0.0 || VIMAG(val[idx]) != 0.0) {
#else
      if (val[idx] != 0.0) {
#endif
        nnzrow++;
      }
    }

    if (xexact != NULL) {
      b[rowID]      = 27.0 - ((CG_FLOAT)(nnzrow - 1));
      xexact[rowID] = 1.0;
    } else {
      b[rowID] = 1.0;
    }
  }
#endif
}

/* End-to-end sanity check of the solution. Cheap, and the only thing that
 * catches a halo contribution that silently went missing - which is exactly the
 * failure mode of a broken communication/computation overlap. */
static void solverCheckResidual(CommType *c, V_ELE *x, V_ELE *xexact, CG_UINT n)
{
  if (xexact == NULL) {
    return;
  }

  CG_FLOAT residual = 0.0;
  V_ELE *v1         = x;
  V_ELE *v2         = xexact;

  for (CG_UINT i = 0; i < n; i++) {
#ifdef USE_COMPLEX
    double diff = VABS(v1[i] - v2[i]);
#else
    double diff = fabs(v1[i] - v2[i]);
#endif
    if (diff > residual) {
      residual = diff;
    }
  }

  commReduction(&residual, MAX);

  if (commIsMaster(c)) {
    printf("Difference between computed and exact  = %E\n", residual);
  }
}

#ifdef USE_COMPLEX
#define CAST(v) VREAL((v))
#else
#define CAST(v) v
#endif

/* USE_OVERLAP_SPMVM and OVERLAP_NUDGE_CHUNKS come from solver.h so that the
 * profiler configuration in main.c always matches the path taken here. */

#ifdef USE_OVERLAP_SPMVM
/**
 * @brief ap = A * p with the halo exchange hidden behind the local SpMV.
 *
 * Posts the exchange, then computes the part of the SpMV that only reads local
 * vector entries. That local phase is walked in OVERLAP_NUDGE_CHUNKS row chunks
 * with an MPI_Test in between: most MPI implementations make no progress on a
 * non-blocking collective unless the application re-enters the library, so
 * without those calls the whole transfer would happen inside commExchangeEnd
 * and nothing would actually be overlapped.
 *
 * COMM_WAIT therefore measures the communication that could NOT be hidden.
 */
static void applyA(CommType *comm, Matrix *A, V_ELE *p, V_ELE *ap)
{
  double ts;
  MPI_Request req;
  const CG_UINT numRows = A->nr;

  PROFILE(COMM, commExchangeBegin(comm, numRows, p, &req));

  const CG_UINT chunks = (OVERLAP_NUDGE_CHUNKS > 1) ? (CG_UINT)OVERLAP_NUDGE_CHUNKS : 1;
  const CG_UINT chunkSize = (numRows + chunks - 1) / chunks;

  for (CG_UINT rowStart = 0; rowStart < numRows; rowStart += chunkSize) {
    CG_UINT rowEnd = rowStart + chunkSize;
    if (rowEnd > numRows) {
      rowEnd = numRows;
    }

    PROFILE(SPMVM_LOCAL, spMVM_local_range(A, p, ap, rowStart, rowEnd));
    PROFILE(COMM, commExchangeTest(&req));
  }

  PROFILE(COMM_WAIT, commExchangeEnd(&req));
  PROFILE(SPMVM_EXT, spMVM_external(A, p, ap));
}
#else
/**
 * @brief ap = A * p with a blocking halo exchange followed by a full SpMV.
 */
static void applyA(CommType *comm, Matrix *A, V_ELE *p, V_ELE *ap)
{
  double ts;

  PROFILE(COMM, commExchange(comm, A->nr, p));
  PROFILE(SPMVM, SPMVMFUNC(A, p, ap));
}
#endif

int solveCG(CommType *comm, Parameter *param, Matrix *A)
{
  NVTX_RANGE_PUSH_C("CG.solve", NVTX_C_CG);
  CG_FLOAT eps   = (CG_FLOAT)param->eps;
  int itermax    = param->itermax;

  CG_UINT nrow   = A->nr;

  bool useXexact = (strcmp(param->filename, "generate") == 0 ||
                    strcmp(param->filename, "generate7P") == 0);

  CGData d;
  allocCGData(&d, A, useXexact);

  V_ELE *r      = d.r;
  V_ELE *p      = d.p;
  V_ELE *ap     = d.ap;
  V_ELE *x      = d.x;
  V_ELE *b      = d.b;
  V_ELE *xexact = d.xexact;

  NVTX_RANGE_PUSH_C("CG.setup", NVTX_C_CG);
  initVectors(A, x, b, xexact);

  // Permute colInd and vectors to SCS ordering so no per-iteration
  // permute_vector is needed inside the CG loop.
#ifdef SCS
  CG_UINT *oldToNewPerm = A->oldToNewPerm;
  CG_UINT *newToOldPerm = A->newToOldPerm;
  V_ELE *permTmp        = d.permTmp;

  // Permute b, x (and xexact) from original to SCS ordering
  permute_vector(oldToNewPerm, b, permTmp, nrow);
  memcpy(b, permTmp, nrow * sizeof(V_ELE));

  permute_vector(oldToNewPerm, x, permTmp, nrow);
  memcpy(x, permTmp, nrow * sizeof(V_ELE));

  if (xexact != NULL) {
    permute_vector(oldToNewPerm, xexact, permTmp, nrow);
    memcpy(xexact, permTmp, nrow * sizeof(V_ELE));
  }
#endif
  NVTX_RANGE_POP();

  CG_FLOAT normr  = 0.0;
  V_ELE rtrans    = 0.0;
  V_ELE oldrtrans = 0.0;

  int printFreq   = itermax / 10;
  if (printFreq > 50) {
    printFreq = 50;
  }
  if (printFreq < 1) {
    printFreq = 1;
  }
  double timeStart, timeStop, ts;

  NVTX_RANGE_PUSH_C("CG.initResidual", NVTX_C_CG);
  PROFILE(WAXPBY, WAXBYFUNC(nrow, 1.0, x, 0.0, x, p));
  applyA(comm, A, p, ap);
  PROFILE(WAXPBY, WAXBYFUNC(nrow, 1.0, b, -1.0, ap, r));
  PROFILE(DDOT, DDOTFUNC(nrow, r, r, &rtrans));
  NVTX_RANGE_POP();

  normr = sqrt(CAST(rtrans));
  if (commIsMaster(comm)) {
    printf("Initial Residual = %E\n", normr);
  }

  int k;
  timeStart = getTimeStamp();
  NVTX_RANGE_PUSH_C("CG.iterations", NVTX_C_CG);
  for (k = 1; k < itermax && normr > eps; k++) {
    if (k == 1) {
      PROFILE(WAXPBY, WAXBYFUNC(nrow, 1.0, r, 0.0, r, p));
    } else {
      oldrtrans = rtrans;
      PROFILE(DDOT, DDOTFUNC(nrow, r, r, &rtrans));
      V_ELE beta = rtrans / oldrtrans;
      PROFILE(WAXPBY, WAXBYFUNC(nrow, 1.0, r, beta, p, p));
    }
    normr = sqrt(CAST(rtrans));

    if (commIsMaster(comm) && (k % printFreq == 0 || k + 1 == itermax)) {
      printf("Iteration = %d Residual = %E\n", k, normr);
    }

    applyA(comm, A, p, ap);

    V_ELE alpha = 0.0;
    PROFILE(DDOT, DDOTFUNC(nrow, p, ap, &alpha));
    alpha = rtrans / alpha;
    PROFILE(WAXPBY, WAXBYFUNC(nrow, 1.0, x, alpha, p, x));
    PROFILE(WAXPBY, WAXBYFUNC(nrow, 1.0, r, -alpha, ap, r));
  }
  NVTX_RANGE_POP();
  timeStop = getTimeStamp();

  if (commIsMaster(comm)) {
    printf("Solution performed %d iterations and took %.2fs\n", k, timeStop - timeStart);
  }

#ifdef SCS
  permute_vector(newToOldPerm, x, permTmp, nrow);
  memcpy(x, permTmp, nrow * sizeof(V_ELE));

  if (xexact != NULL) {
    permute_vector(newToOldPerm, xexact, permTmp, nrow);
    memcpy(xexact, permTmp, nrow * sizeof(V_ELE));
  }
#endif

  solverCheckResidual(comm, x, xexact, nrow);

  freeCGData(&d);

  NVTX_RANGE_POP();
  return k;
}

// NTS : makes allocation and dellocation centralized so that we dont
// allocate any data during the iterations
void allocCGData(CGData *d, Matrix *m, bool useXexact)
{
  CG_UINT nrow = m->nr;
  CG_UINT ncol = m->nc;

  d->r         = (V_ELE *)allocate(ARRAY_ALIGNMENT, nrow * sizeof(V_ELE));
  d->p         = (V_ELE *)allocate(ARRAY_ALIGNMENT, ncol * sizeof(V_ELE));
#ifdef SCS
  d->ap = (V_ELE *)allocate(ARRAY_ALIGNMENT, m->nrPadded * sizeof(V_ELE));
#else
  d->ap = (V_ELE *)allocate(ARRAY_ALIGNMENT, nrow * sizeof(V_ELE));
#endif
  d->x      = (V_ELE *)allocate(ARRAY_ALIGNMENT, nrow * sizeof(V_ELE));
  d->b      = (V_ELE *)allocate(ARRAY_ALIGNMENT, nrow * sizeof(V_ELE));

  d->xexact = NULL;
  if (useXexact) {
    d->xexact = (V_ELE *)allocate(ARRAY_ALIGNMENT, nrow * sizeof(V_ELE));
  }

#ifdef SCS
  d->permTmp = (V_ELE *)allocate(ARRAY_ALIGNMENT, nrow * sizeof(V_ELE));
#else
  d->permTmp = NULL;
#endif
}

void freeCGData(CGData *d)
{
  deallocate(d->r);
  deallocate(d->p);
  deallocate(d->ap);
  deallocate(d->x);
  deallocate(d->b);
  deallocate(d->xexact);
  deallocate(d->permTmp);
}
