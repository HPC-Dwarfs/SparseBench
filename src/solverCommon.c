/* Copyright (C) NHR@FAU, University Erlangen-Nuremberg.
 * All rights reserved. This file is part of SparseBench.
 * Use of this source code is governed by a MIT style
 * license that can be found in the LICENSE file. */
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "comm.h"
#include "kernel_dispatch.h"
#include "matrix.h"
#include "profiler.h"
#include "solver.h"
#include "timing.h"
#include "vtype.h"

/* Helpers shared by the iterative solvers (CG, GMRES). */

void solverInitVectors(Matrix *m, V_ELE *x, V_ELE *b, V_ELE *xexact)
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

#ifdef SCS
/* Permute every non-NULL vector in place through tmp (length n). */
void solverPermuteVectors(
    const CG_UINT *perm, V_ELE *tmp, CG_UINT n, V_ELE *x, V_ELE *b, V_ELE *xexact)
{
  V_ELE *vecs[3] = { x, b, xexact };

  for (int i = 0; i < 3; i++) {
    if (vecs[i] != NULL) {
      permute_vector(perm, vecs[i], tmp, n);
      memcpy(vecs[i], tmp, n * sizeof(V_ELE));
    }
  }
}
#endif

/* End-to-end sanity check of the solution. Cheap, and the only thing that
 * catches a halo contribution that silently went missing - which is exactly the
 * failure mode of a broken communication/computation overlap. */
void solverCheckResidual(CommType *c, V_ELE *x, V_ELE *xexact, CG_UINT n)
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
void solverApplyA(CommType *comm, Matrix *A, V_ELE *p, V_ELE *ap)
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

    PROFILE_TIMED(SPMVM_LOCAL, spMVM_local_range(A, p, ap, rowStart, rowEnd));
    PROFILE_TIMED(COMM, commExchangeTest(&req));
  }
  NCalls[SPMVM_LOCAL]++;

  PROFILE(COMM_WAIT, commExchangeEnd(&req));
  PROFILE(SPMVM_EXT, spMVM_external(A, p, ap));
}
#else
/**
 * @brief ap = A * p with a blocking halo exchange followed by a full SpMV.
 */
void solverApplyA(CommType *comm, Matrix *A, V_ELE *p, V_ELE *ap)
{
  double ts;

  PROFILE(COMM, commExchange(comm, A->nr, p));
  PROFILE(SPMVM, SPMVMFUNC(A, p, ap));
}
#endif
