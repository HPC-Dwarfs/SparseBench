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
#include "kernel_dispatch.h"
#include "nvtx_marker.h"
#include "profiler.h"
#include "solver.h"
#include "timing.h"
#include "util.h"
#include "vtype.h"

#if PRECISION == 1
#define GMRES_EPS FLT_EPSILON
#else
#define GMRES_EPS DBL_EPSILON
#endif

/* r = b - A*x, returns ||r||. r must span the SpMV input size, tmp the output
 * size. */
static CG_FLOAT residual(
    CommType *comm, Matrix *A, V_ELE *x, V_ELE *b, V_ELE *r, V_ELE *tmp)
{
  double ts;
  CG_UINT nrow = A->nr;
  V_ELE rtmp;

  PROFILE(WAXPBY, WAXBYFUNC(nrow, 1.0, x, 0.0, x, r));
  solverApplyA(comm, A, r, tmp);
  PROFILE(WAXPBY, WAXBYFUNC(nrow, 1.0, b, -1.0, tmp, r));
  PROFILE(DDOT, DDOTFUNC(nrow, r, r, &rtmp));

  return sqrt(rtmp);
}

/* Restarted GMRES(m).
 *
 * param->restart controls the Krylov subspace size per restart cycle.
 * param->itermax is the maximum total number of Arnoldi steps across all
 * restarts, consistent with CG's itermax semantics.
 *
 * The Hessenberg matrix and Givens rotations are real, so complex builds are
 * rejected. */
int solveGMRES(CommType *comm, Parameter *param, Matrix *A)
{
#ifdef USE_COMPLEX
  commAbort(comm, "GMRES does not support complex arithmetic\n");
  return 0;
#else
  NVTX_RANGE_PUSH_C("GMRES.solve", NVTX_C_CG);
  CG_FLOAT eps = (CG_FLOAT)param->eps;
  int itermax  = param->itermax;
  int m        = param->restart;

  if (m < 1) {
    commAbort(comm, "GMRES restart dimension must be >= 1\n");
  }

  CG_UINT nrow   = A->nr;

  bool useXexact = (strcmp(param->filename, "generate") == 0 ||
                    strcmp(param->filename, "generate7P") == 0);

  /* Every basis vector is an SpMV input (needs the halo region up to nc) and
   * an SpMV output (the SCS kernels also write the padded rows). */
#ifdef SCS
  CG_UINT vecSize = MAX(A->nc, A->nrPadded);
#else
  CG_UINT vecSize = A->nc;
#endif

  V_ELE **V = (V_ELE **)malloc((m + 1) * sizeof(V_ELE *));
  for (int i = 0; i <= m; i++) {
    V[i] = (V_ELE *)allocate(ARRAY_ALIGNMENT, vecSize * sizeof(V_ELE));
    memset(V[i], 0, vecSize * sizeof(V_ELE));
  }

  /* Small dense host-side data: upper Hessenberg (m+1) x m, column-major,
   * H[i + j*(m+1)] = H_{i,j}, plus Givens rotations and the rhs g. */
  CG_FLOAT *H   = (CG_FLOAT *)malloc((m + 1) * m * sizeof(CG_FLOAT));
  CG_FLOAT *cs  = (CG_FLOAT *)malloc(m * sizeof(CG_FLOAT));
  CG_FLOAT *sn  = (CG_FLOAT *)malloc(m * sizeof(CG_FLOAT));
  CG_FLOAT *g   = (CG_FLOAT *)malloc((m + 1) * sizeof(CG_FLOAT));
  CG_FLOAT *y   = (CG_FLOAT *)malloc(m * sizeof(CG_FLOAT));

  V_ELE *x      = (V_ELE *)allocate(ARRAY_ALIGNMENT, nrow * sizeof(V_ELE));
  V_ELE *b      = (V_ELE *)allocate(ARRAY_ALIGNMENT, nrow * sizeof(V_ELE));
  V_ELE *xexact = NULL;
  if (useXexact) {
    xexact = (V_ELE *)allocate(ARRAY_ALIGNMENT, nrow * sizeof(V_ELE));
  }

  NVTX_RANGE_PUSH_C("GMRES.setup", NVTX_C_CG);
  solverInitVectors(A, x, b, xexact);

#ifdef SCS
  CG_UINT *oldToNewPerm = A->oldToNewPerm;
  CG_UINT *newToOldPerm = A->newToOldPerm;
  V_ELE *permTmp        = (V_ELE *)allocate(ARRAY_ALIGNMENT, nrow * sizeof(V_ELE));

  // Permute b, x (and xexact) from original to SCS ordering
  solverPermuteVectors(oldToNewPerm, permTmp, nrow, x, b, xexact);
#endif
  NVTX_RANGE_POP();

  int printFreq = itermax / 10;
  if (printFreq > 50) {
    printFreq = 50;
  }
  if (printFreq < 1) {
    printFreq = 1;
  }

  double timeStart, timeStop, ts;
  V_ELE rtmp;

  /* Initial residual into V[0]; reused by the first restart cycle. */
  NVTX_RANGE_PUSH_C("GMRES.initResidual", NVTX_C_CG);
  CG_FLOAT normr = residual(comm, A, x, b, V[0], V[1]);
  NVTX_RANGE_POP();

  if (commIsMaster(comm)) {
    printf("Initial Residual = %E\n", normr);
  }

  int k     = 0;
  timeStart = getTimeStamp();
  NVTX_RANGE_PUSH_C("GMRES.iterations", NVTX_C_CG);

  while (k < itermax && normr > eps) {

    /* ---- r = b - A*x into V[0] (already there for the first cycle) ---- */
    if (k > 0) {
      normr = residual(comm, A, x, b, V[0], V[1]);
      if (normr <= eps) {
        break;
      }
    }
    CG_FLOAT beta = normr;
    PROFILE(WAXPBY, WAXBYFUNC(nrow, (V_ELE)(1.0 / beta), V[0], (V_ELE)0.0, V[0], V[0]));

    memset(g, 0, (m + 1) * sizeof(CG_FLOAT));
    g[0] = beta;

    /* Steps remaining before hitting itermax */
    int jlimit = (k + m <= itermax) ? m : (itermax - k);
    int jj     = 0;

    /* ---- Arnoldi inner loop ---- */
    for (int j = 0; j < jlimit; j++) {
      jj = j;

      /* w = A * V[j]; result written into V[j+1] */
      solverApplyA(comm, A, V[j], V[j + 1]);

      /* Modified Gram-Schmidt orthogonalisation against V[0..j] */
      CG_FLOAT colNorm2 = 0.0;
      for (int i = 0; i <= j; i++) {
        PROFILE(DDOT, DDOTFUNC(nrow, V[i], V[j + 1], &rtmp));
        CG_FLOAT hij       = rtmp;
        H[i + j * (m + 1)] = hij;
        colNorm2 += hij * hij;
        PROFILE(WAXPBY, WAXBYFUNC(nrow, 1.0, V[j + 1], (V_ELE)(-hij), V[i], V[j + 1]));
      }

      PROFILE(DDOT, DDOTFUNC(nrow, V[j + 1], V[j + 1], &rtmp));
      CG_FLOAT h_next          = sqrt(rtmp);
      H[(j + 1) + j * (m + 1)] = h_next;
      colNorm2 += h_next * h_next;

      /* Lucky breakdown: A*V[j] lies in span(V[0..j]), so the Krylov space is
       * invariant and this cycle's least-squares solution is exact. The
       * threshold is relative to ||A*V[j]|| (= column norm of H, since MGS
       * preserves it). Skip normalisation; the rotations below still finish
       * column j so the back substitution is valid. */
      bool breakdown = (h_next <= 10.0 * GMRES_EPS * sqrt(colNorm2));
      if (!breakdown) {
        PROFILE(WAXPBY,
            WAXBYFUNC(
                nrow, (V_ELE)(1.0 / h_next), V[j + 1], (V_ELE)0.0, V[j + 1], V[j + 1]));
      }

      /* Apply previous Givens rotations to column j of H */
      for (int i = 0; i < j; i++) {
        CG_FLOAT h_ij            = H[i + j * (m + 1)];
        CG_FLOAT h_i1j           = H[(i + 1) + j * (m + 1)];
        H[i + j * (m + 1)]       = cs[i] * h_ij + sn[i] * h_i1j;
        H[(i + 1) + j * (m + 1)] = -sn[i] * h_ij + cs[i] * h_i1j;
      }

      /* Compute new Givens rotation to zero H[j+1][j] */
      CG_FLOAT h_jj            = H[j + j * (m + 1)];
      CG_FLOAT h_jp1j          = H[(j + 1) + j * (m + 1)];
      CG_FLOAT rlen            = sqrt(h_jj * h_jj + h_jp1j * h_jp1j);
      cs[j]                    = h_jj / rlen;
      sn[j]                    = h_jp1j / rlen;
      H[j + j * (m + 1)]       = rlen;
      H[(j + 1) + j * (m + 1)] = 0.0;

      /* Update g and track residual norm estimate */
      CG_FLOAT g_jp1 = -sn[j] * g[j];
      g[j]           = cs[j] * g[j];
      g[j + 1]       = g_jp1;
      normr          = fabs(g_jp1);

      if (commIsMaster(comm) && ((k + j + 1) % printFreq == 0)) {
        printf("Iteration = %d   Residual = %E\n", k + j + 1, normr);
      }

      if (breakdown || normr <= eps) {
        break;
      }
    }

    /* ---- Back substitution: solve H[0..jj][0..jj] * y = g[0..jj] ---- */
    for (int i = jj; i >= 0; i--) {
      y[i] = g[i];
      for (int kk = i + 1; kk <= jj; kk++) {
        y[i] -= H[i + kk * (m + 1)] * y[kk];
      }
      y[i] /= H[i + i * (m + 1)];
    }

    /* ---- Solution update: x += sum_i y[i] * V[i] ---- */
    for (int i = 0; i <= jj; i++) {
      PROFILE(WAXPBY, WAXBYFUNC(nrow, 1.0, x, (V_ELE)y[i], V[i], x));
    }

    k += jj + 1;
  }

  NVTX_RANGE_POP();
  timeStop = getTimeStamp();

  if (commIsMaster(comm)) {
    printf("Solution performed %d iterations and took %.2fs\n", k, timeStop - timeStart);
  }

  /* normr is the Arnoldi estimate; report the true residual as well. */
  normr = residual(comm, A, x, b, V[0], V[1]);
  if (commIsMaster(comm)) {
    printf("Final Residual = %E\n", normr);
  }

#ifdef SCS
  solverPermuteVectors(newToOldPerm, permTmp, nrow, x, NULL, xexact);
  deallocate(permTmp);
#endif

  solverCheckResidual(comm, x, xexact, nrow);

  for (int i = 0; i <= m; i++) {
    deallocate(V[i]);
  }
  free(V);
  free(H);
  free(cs);
  free(sn);
  free(g);
  free(y);
  deallocate(x);
  deallocate(b);
  deallocate(xexact);

  NVTX_RANGE_POP();
  return k;
#endif
}
