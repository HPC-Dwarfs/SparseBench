/* Copyright (C) NHR@FAU, University Erlangen-Nuremberg.
 * All rights reserved. This file is part of SparseBench.
 * Use of this source code is governed by a MIT style
 * license that can be found in the LICENSE file. */
#ifndef __SOLVER_H_
#define __SOLVER_H_
#include "comm.h"
#include "parameter.h"
#include "util.h"
#include "vtype.h"

/* Communication/computation overlap is only available for the CPU CRS backend,
 * where the split kernels spMVM_local/spMVM_external exist. For the GPU backends
 * and the SCS format, fall back to a blocking halo exchange plus a full SpMV.
 * Defined here rather than in CGSolver.c so that the profiler configuration in
 * main.c cannot disagree with the path the solver actually takes. */
#if defined(ENABLE_OVERLAP) && defined(_MPI) && defined(CRS) &&                          \
    !defined(RUNTIME_BACKEND_IS_CUDA) && !defined(RUNTIME_BACKEND_IS_HIP)
#define USE_OVERLAP_SPMVM
#endif

/* Number of row chunks the local SpMV is split into so that MPI progress can be
 * nudged in between. Set from config.mk; 1 disables the chunking. */
#ifndef OVERLAP_NUDGE_CHUNKS
#define OVERLAP_NUDGE_CHUNKS 8
#endif

extern int solveCG(CommType *comm, Parameter *param, Matrix *m);
// extern void solverCheckResidual(Solver* s, Comm* c);
extern void spMVM(Matrix *m, const V_ELE *restrict x, V_ELE *restrict y);
extern void spMMVM(Matrix *m, const DMatrix *x, DMatrix *y);

/* waxpby and ddot deliberately support aliasing: the CG solver calls
 * waxpby(n, 1.0, v, s, w, v) style in-place updates and ddot(n, v, v, &r).
 * Declaring x/y/w restrict would make those calls undefined behavior (GCC
 * reports -Wrestrict), so no restrict qualifiers are used here. */
extern void waxpby(const CG_UINT n,
    const V_ELE alpha,
    const V_ELE *x,
    const V_ELE beta,
    const V_ELE *y,
    V_ELE *w);

extern void ddot(const CG_UINT n, const V_ELE *e, const V_ELE *y, V_ELE *result);
#endif // __SOLVER_H_
