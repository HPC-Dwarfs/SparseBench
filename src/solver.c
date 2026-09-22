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

#include "comm.h"
#include "solver.h"
#include "util.h"
#include "vtype.h"

#ifdef _OPENMP
#define OMP_PARFOR _Pragma("omp parallel for schedule(static)")
#define OMP_PARFOR_REDUCE _Pragma("omp parallel for reduction(+ : sum) schedule(static)")
#else
#define OMP_PARFOR
#define OMP_PARFOR_REDUCE
#endif

void waxpby(const CG_UINT n,
    const V_ELE alpha,
    const V_ELE *x,
    const V_ELE beta,
    const V_ELE *y,
    V_ELE *const w)
{
  if (alpha == 1.0) {
    OMP_PARFOR
    for (CG_UINT i = 0; i < n; i++) {
      w[i] = x[i] + beta * y[i];
    }
  } else if (beta == 1.0) {
    OMP_PARFOR
    for (CG_UINT i = 0; i < n; i++) {
      w[i] = alpha * x[i] + y[i];
    }
  } else {
    OMP_PARFOR
    for (CG_UINT i = 0; i < n; i++) {
      w[i] = alpha * x[i] + beta * y[i];
    }
  }
}

void ddot(const CG_UINT n, const V_ELE *x, const V_ELE *y, V_ELE *result)
{
  V_ELE sum = 0.0;

#ifdef USE_COMPLEX
  if (y == x) {
    OMP_PARFOR_REDUCE
    for (CG_UINT i = 0; i < n; i++) {
      sum += VCONJ(x[i]) * x[i];
    }
  } else {
    OMP_PARFOR_REDUCE
    for (CG_UINT i = 0; i < n; i++) {
      sum += VCONJ(x[i]) * y[i];
    }
  }
#else
  if (y == x) {
    OMP_PARFOR_REDUCE
    for (CG_UINT i = 0; i < n; i++) {
      sum += x[i] * x[i];
    }
  } else {
    OMP_PARFOR_REDUCE
    for (CG_UINT i = 0; i < n; i++) {
      sum += x[i] * y[i];
    }
  }
#endif

  commReductionV(&sum, SUM);
  *result = sum;
}
