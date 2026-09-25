/* Copyright (C) NHR@FAU, University Erlangen-Nuremberg.
 * All rights reserved. This file is part of SparseBench.
 * Use of this source code is governed by a MIT style
 * license that can be found in the LICENSE file. */
#ifndef __SOLVER_H_
#define __SOLVER_H_
#include <stdbool.h>
#include <stddef.h>

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

/* Why an iterative solver stopped. The last three are the exact-solution guards
 * and can end a run early in either stopping mode. */
typedef enum {
  STOP_CONVERGED = 0, // ||r||/||b|| <= eps (tolerance mode)
  STOP_ITERMAX,       // itermax iterations performed
  STOP_BREAKDOWN,     // GMRES lucky breakdown: the Krylov space is invariant
  STOP_EXACT,         // residual became exactly zero
  STOP_ZERO_RHS       // ||b|| == 0, x = 0 returned without iterating
} StopReason;

/* Filled by every solver variant, printed by solverPrintResult. */
typedef struct {
  int iterations;
  StopReason reason;
  double solveTime;        // iteration loop only
  CG_FLOAT relResEstimate; // recurrence / Givens estimate, relative to ||b||
  CG_FLOAT relResTrue;     // ||b - Ax|| / ||b||, computed after the timer
  CG_FLOAT errMax;         // max|x - xexact|, < 0 if there is no exact solution
} SolverResult;

typedef int (*SolveFn)(CommType *, Parameter *, Matrix *, SolverResult *);

/* Build configurations a variant supports. The configuration of the current
 * build is derived from the build macros in solverRegistry.c. */
enum {
  SUPPORT_REAL    = 1u << 0,
  SUPPORT_COMPLEX = 1u << 1,
  SUPPORT_CRS     = 1u << 2,
  SUPPORT_SCS     = 1u << 3,
  SUPPORT_CCRS    = 1u << 4,
  SUPPORT_CPU     = 1u << 5,
  SUPPORT_GPU     = 1u << 6,
};
#define SUPPORT_ALL_FORMATS (SUPPORT_CRS | SUPPORT_SCS | SUPPORT_CCRS)
#define SUPPORT_ALL_BACKENDS (SUPPORT_CPU | SUPPORT_GPU)

typedef struct {
  const char *name;
  SolveFn solve;
  unsigned supports;  // SUPPORT_* bitmask
  const int *profSeq; // profiler regions the variant executes
  int numProfSeq;
} SolverVariant;

/* GMRES orthogonalization of V[j+1] against V[0..j]. Writes column j of the
 * Hessenberg matrix to h[0..j+1], stores the norm of the unorthogonalized
 * column in *colNorm (for the breakdown test) and returns h_{j+1,j}. V[j+1] is
 * left unnormalized. */
typedef CG_FLOAT (*OrthoFn)(
    CG_UINT nrow, V_ELE **V, int j, CG_FLOAT *h, CG_FLOAT *colNorm);

typedef struct {
  const char *name;
  OrthoFn ortho;
  unsigned supports;
} OrthoVariant;

/* Resolved selection for one run. ortho is NULL for solvers other than GMRES. */
typedef struct {
  const char *solverName;
  const SolverVariant *variant;
  const OrthoVariant *ortho;
} SolverSelection;

// solver variant registry (solverRegistry.c)
extern const SolverVariant *solverVariants(int benchType, int *count);
extern const OrthoVariant *solverOrthoVariants(int *count);
extern const SolverVariant *solverFindVariant(int benchType, const char *name);
extern const OrthoVariant *solverFindOrtho(const char *name);
extern void solverVariantNames(int benchType, char *buf, size_t len);
extern void solverOrthoNames(char *buf, size_t len);
extern bool solverCheckSupport(
    const char *what, unsigned supports, char *msg, size_t len);
extern bool solverResolve(
    int benchType, const Parameter *param, SolverSelection *sel, char *msg, size_t len);
extern void solverPrintSelection(
    CommType *comm, const SolverSelection *sel, const Parameter *param);
extern void solverPrintResult(CommType *comm,
    const SolverSelection *sel,
    const Parameter *param,
    const SolverResult *res);

extern int solveCG(CommType *comm, Parameter *param, Matrix *m, SolverResult *res);
extern int solveGMRES(CommType *comm, Parameter *param, Matrix *m, SolverResult *res);
extern void gmresSetOrtho(OrthoFn ortho);
extern CG_FLOAT gmresOrthoMGS(
    CG_UINT nrow, V_ELE **V, int j, CG_FLOAT *h, CG_FLOAT *colNorm);

typedef struct {
  V_ELE *r;
  V_ELE *p;
  V_ELE *ap;
  V_ELE *x;
  V_ELE *b;
  V_ELE *xexact;
  V_ELE *permTmp;
} CGData;

// centralized methods to allocate and deallocate
extern void allocCGData(CGData *d, Matrix *m, bool useXexact);
extern void freeCGData(CGData *d);

// helpers shared by the iterative solvers (solverCommon.c)
extern void solverInitVectors(Matrix *m, V_ELE *x, V_ELE *b, V_ELE *xexact);
/* Scales the rhs (and the exact solution) solverInitVectors produces. Test
 * hook for the zero-rhs guard and the scaling invariance; default 1. */
extern void solverSetRhsScale(CG_FLOAT scale);
extern CG_FLOAT solverCheckResidual(CommType *c, V_ELE *x, V_ELE *xexact, CG_UINT n);
extern void solverApplyA(CommType *comm, Matrix *A, V_ELE *p, V_ELE *ap);
/* r = b - A*x, returns ||r||. r must span the SpMV input size, tmp the output
 * size. */
extern CG_FLOAT solverResidualNorm(
    CommType *comm, Matrix *A, V_ELE *x, const V_ELE *b, V_ELE *r, V_ELE *tmp);

typedef struct {
  CG_FLOAT normb;  // ||b|| over all ranks
  CG_FLOAT absTol; // eps*||b|| in tolerance mode, -1 in fixed mode
  bool zeroRhs;    // ||b|| == 0
} SolverStop;

extern SolverStop solverStopInit(
    CommType *comm, const V_ELE *b, CG_UINT nrow, double eps);
#ifdef SCS
extern void solverPermuteVectors(
    const CG_UINT *perm, V_ELE *tmp, CG_UINT n, V_ELE *x, V_ELE *b, V_ELE *xexact);
#endif

// extern void solverCheckResidual(Solver* s, Comm* c);
extern void spMVM(Matrix *m, const V_ELE *restrict x, V_ELE *restrict y);
extern void spMMVM(Matrix *m, const DMatrix *x, DMatrix *y);

/* Fused y = cA*(m*x) + cP*p + cQ*q, evaluated per output row without ever
 * materializing m*x. q may be NULL (with cQ ignored) for the 2-term form.
 * y may alias p and/or q (row-local, no cross-row dependency). */
extern void spMMVMFused(Matrix *m,
    const DMatrix *x,
    V_ELE cA,
    const DMatrix *p,
    V_ELE cP,
    const DMatrix *q,
    V_ELE cQ,
    DMatrix *y);

/* ChebFD recurrence step: y = cA*(m*w) + cP*w + cQ*q (q optional, NULL to
 * skip), fused with the accumulate x += gc*y in the same row/chunk pass.
 * y may alias q; x is a separate accumulator block. */
extern void chebfdOp(Matrix *m,
    const DMatrix *w,
    V_ELE cA,
    V_ELE cP,
    const DMatrix *q,
    V_ELE cQ,
    DMatrix *y,
    V_ELE gc,
    DMatrix *x);

extern void waxpby(const CG_UINT n,
    const V_ELE alpha,
    const V_ELE *x,
    const V_ELE beta,
    const V_ELE *y,
    V_ELE *const w);

/* Dense triple combine w = a*x + b*y + c*z in a single pass, stride-1.
 * Not restrict: w may alias x/y/z (same aliasing contract as waxpby). */
extern void waxpby3(const CG_UINT n,
    const V_ELE a,
    const V_ELE *x,
    const V_ELE b,
    const V_ELE *y,
    const V_ELE c,
    const V_ELE *z,
    V_ELE *const w);

/* x/y may alias (ddot branches on y == x); `result` must not alias either. */
extern void ddot(const CG_UINT n, const V_ELE *e, const V_ELE *y, V_ELE *restrict result);

/* Strided variants for dense-block ops (e.g. ChebFD columns of row-major DMatrix
 * blocks); the stride-1 versions above stay the hot path for contiguous vectors.
 * waxpby_stride: w may alias x/y only if the aliased vectors share a stride. */
extern void waxpby_stride(const CG_UINT n,
    const V_ELE alpha,
    const V_ELE *x,
    const CG_UINT incx,
    const V_ELE beta,
    const V_ELE *y,
    const CG_UINT incy,
    V_ELE *const w,
    const CG_UINT incw);

extern void ddot_stride(const CG_UINT n,
    const V_ELE *x,
    const CG_UINT incx,
    const V_ELE *y,
    const CG_UINT incy,
    V_ELE *restrict result);
#endif // __SOLVER_H_
