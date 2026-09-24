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
#include "vtype.h"

#ifdef USE_COMPLEX
#define CAST(v) VREAL((v))
#else
#define CAST(v) v
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
  solverInitVectors(A, x, b, xexact);

  // Permute colInd and vectors to SCS ordering so no per-iteration
  // permute_vector is needed inside the CG loop.
#ifdef SCS
  CG_UINT *oldToNewPerm = A->oldToNewPerm;
  CG_UINT *newToOldPerm = A->newToOldPerm;
  V_ELE *permTmp        = d.permTmp;

  // Permute b, x (and xexact) from original to SCS ordering
  solverPermuteVectors(oldToNewPerm, permTmp, nrow, x, b, xexact);
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
  solverApplyA(comm, A, p, ap);
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

    solverApplyA(comm, A, p, ap);

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
  solverPermuteVectors(newToOldPerm, permTmp, nrow, x, NULL, xexact);
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
