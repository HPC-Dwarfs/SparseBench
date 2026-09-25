/* Copyright (C) NHR@FAU, University Erlangen-Nuremberg.
 * All rights reserved. This file is part of SparseBench.
 * Use of this source code is governed by a MIT style
 * license that can be found in the LICENSE file. */
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "cli.h"
#include "comm.h"
#include "parameter.h"
#include "profiler.h"
#include "solver.h"
#include "util.h"

/* Registry of the iterative solver variants. Adding a variant means adding its
 * solve function and one row to the table of its solver type. */

#define NELEMS(a) ((int)(sizeof(a) / sizeof((a)[0])))

/* Profiler regions of the solvers built on solverApplyA/waxpby/ddot. Selected
 * at compile time with the same macro solverApplyA uses, so the table cannot
 * disagree with the SpMV path that actually runs. */
#ifdef USE_OVERLAP_SPMVM
static const int seqKrylov[] = { DDOT, WAXPBY, SPMVM_LOCAL, SPMVM_EXT, COMM_WAIT };
#else
static const int seqKrylov[] = { DDOT, WAXPBY, SPMVM };
#endif

#define SUPPORT_ANY_REAL (SUPPORT_REAL | SUPPORT_ALL_FORMATS | SUPPORT_ALL_BACKENDS)

static const SolverVariant cgVariants[] = {
  { "standard",
   solveCG, SUPPORT_REAL | SUPPORT_COMPLEX | SUPPORT_ALL_FORMATS | SUPPORT_ALL_BACKENDS,
   seqKrylov, NELEMS(seqKrylov) },
};

/* The Hessenberg matrix and the Givens rotations are real. */
static const SolverVariant gmresVariants[] = {
  { "standard", solveGMRES, SUPPORT_ANY_REAL, seqKrylov, NELEMS(seqKrylov) },
};

static const OrthoVariant orthoVariants[] = {
  { "mgs", gmresOrthoMGS, SUPPORT_ANY_REAL },
};

/* The configuration of this build, one bit per axis. */
static unsigned buildConfig(void)
{
  unsigned c = 0;
#ifdef USE_COMPLEX
  c |= SUPPORT_COMPLEX;
#else
  c |= SUPPORT_REAL;
#endif
#if defined(CRS)
  c |= SUPPORT_CRS;
#elif defined(SCS)
  c |= SUPPORT_SCS;
#elif defined(CCRS)
  c |= SUPPORT_CCRS;
#endif
#if defined(RUNTIME_BACKEND_IS_CUDA) || defined(RUNTIME_BACKEND_IS_HIP)
  c |= SUPPORT_GPU;
#else
  c |= SUPPORT_CPU;
#endif
  return c;
}

static const struct {
  unsigned bit;
  const char *label;
} supportLabels[] = {
  { SUPPORT_REAL,    "real arithmetic"        },
  { SUPPORT_COMPLEX, "complex arithmetic"     },
  { SUPPORT_CRS,     "the CRS matrix format"  },
  { SUPPORT_SCS,     "the SCS matrix format"  },
  { SUPPORT_CCRS,    "the CCRS matrix format" },
  { SUPPORT_CPU,     "the CPU backend"        },
  { SUPPORT_GPU,     "GPU backends"           },
};

const SolverVariant *solverVariants(int benchType, int *count)
{
  switch (benchType) {
  case CG:
    *count = NELEMS(cgVariants);
    return cgVariants;
  case GMRES:
    *count = NELEMS(gmresVariants);
    return gmresVariants;
  default:
    *count = 0;
    return NULL;
  }
}

const OrthoVariant *solverOrthoVariants(int *count)
{
  *count = NELEMS(orthoVariants);
  return orthoVariants;
}

const SolverVariant *solverFindVariant(int benchType, const char *name)
{
  int n;
  const SolverVariant *v = solverVariants(benchType, &n);

  for (int i = 0; name != NULL && i < n; i++) {
    if (strcmp(v[i].name, name) == 0) {
      return &v[i];
    }
  }
  return NULL;
}

const OrthoVariant *solverFindOrtho(const char *name)
{
  for (int i = 0; name != NULL && i < NELEMS(orthoVariants); i++) {
    if (strcmp(orthoVariants[i].name, name) == 0) {
      return &orthoVariants[i];
    }
  }
  return NULL;
}

/* Append ", "-separated names to buf; snprintf truncates safely. */
static void appendName(char *buf, size_t len, const char *name)
{
  size_t used = strlen(buf);
  if (used < len) {
    snprintf(buf + used, len - used, "%s%s", used > 0 ? ", " : "", name);
  }
}

void solverVariantNames(int benchType, char *buf, size_t len)
{
  int n;
  const SolverVariant *v = solverVariants(benchType, &n);

  buf[0]                 = '\0';
  for (int i = 0; i < n; i++) {
    appendName(buf, len, v[i].name);
  }
}

void solverOrthoNames(char *buf, size_t len)
{
  buf[0] = '\0';
  for (int i = 0; i < NELEMS(orthoVariants); i++) {
    appendName(buf, len, orthoVariants[i].name);
  }
}

/* True if every axis of this build is in supports. Otherwise msg names what
 * (e.g. "GMRES variant 'standard'") and each unsupported configuration. */
bool solverCheckSupport(const char *what, unsigned supports, char *msg, size_t len)
{
  unsigned missing = buildConfig() & ~supports;
  if (missing == 0) {
    return true;
  }

  snprintf(msg, len, "%s does not support ", what);
  bool first = true;
  for (int i = 0; i < NELEMS(supportLabels); i++) {
    if (missing & supportLabels[i].bit) {
      size_t used = strlen(msg);
      snprintf(
          msg + used, len - used, "%s%s", first ? "" : " or ", supportLabels[i].label);
      first = false;
    }
  }
  size_t used = strlen(msg);
  snprintf(msg + used, len - used, "\n");
  return false;
}

/* Resolve the variant and orthogonalization names of param for benchType and
 * validate the stopping parameters. Returns false with a message listing the
 * valid names on failure. Benchmark types without a registry always pass. */
bool solverResolve(
    int benchType, const Parameter *param, SolverSelection *sel, char *msg, size_t len)
{
  char names[256];
  char what[128];

  memset(sel, 0, sizeof(*sel));
  if (benchType != CG && benchType != GMRES) {
    return true;
  }
  sel->solverName = (benchType == CG) ? "CG" : "GMRES";

  if (param->eps < 0.0) {
    snprintf(msg,
        len,
        "Invalid tolerance eps = %g: must be >= 0 (0 = fixed iterations)\n",
        param->eps);
    return false;
  }
  if (param->itermax < 1) {
    snprintf(msg, len, "Invalid itermax = %d: must be >= 1\n", param->itermax);
    return false;
  }

  sel->variant = solverFindVariant(benchType, param->solverVariant);
  if (sel->variant == NULL) {
    solverVariantNames(benchType, names, sizeof(names));
    snprintf(msg,
        len,
        "Unknown %s variant '%s'. Valid %s variants: %s\n",
        sel->solverName,
        param->solverVariant ? param->solverVariant : "(null)",
        sel->solverName,
        names);
    return false;
  }
  snprintf(what, sizeof(what), "%s variant '%s'", sel->solverName, sel->variant->name);
  if (!solverCheckSupport(what, sel->variant->supports, msg, len)) {
    return false;
  }

  if (benchType == GMRES) {
    sel->ortho = solverFindOrtho(param->gmresOrtho);
    if (sel->ortho == NULL) {
      solverOrthoNames(names, sizeof(names));
      snprintf(msg,
          len,
          "Unknown GMRES orthogonalization '%s'. Valid orthogonalization schemes: %s\n",
          param->gmresOrtho ? param->gmresOrtho : "(null)",
          names);
      return false;
    }
    snprintf(what, sizeof(what), "GMRES orthogonalization '%s'", sel->ortho->name);
    if (!solverCheckSupport(what, sel->ortho->supports, msg, len)) {
      return false;
    }
  }

  return true;
}

/* One-line-per-field echo of the selection, printed before the solve. */
void solverPrintSelection(
    CommType *comm, const SolverSelection *sel, const Parameter *param)
{
  if (!commIsMaster(comm) || sel->variant == NULL) {
    return;
  }

  char stopMode[128];
  formatStopMode(param, stopMode, sizeof(stopMode));
  printf("Variant: %s\n", sel->variant->name);
  if (sel->ortho != NULL) {
    printf("Ortho: %s\n", sel->ortho->name);
  }
  printf("Stop mode: %s\n", stopMode);
}

static const char *stopReasonName(StopReason r)
{
  switch (r) {
  case STOP_CONVERGED:
    return "converged";
  case STOP_ITERMAX:
    return "itermax";
  case STOP_BREAKDOWN:
    return "breakdown";
  case STOP_EXACT:
    return "exact solution";
  case STOP_ZERO_RHS:
    return "zero rhs";
  }
  return "unknown";
}

/* Common result summary. Field labels and order are fixed so scripts can grep
 * them; Ortho is omitted for CG, Max error when no exact solution is known. */
void solverPrintResult(CommType *comm,
    const SolverSelection *sel,
    const Parameter *param,
    const SolverResult *res)
{
  if (!commIsMaster(comm)) {
    return;
  }

  char stopMode[128];
  formatStopMode(param, stopMode, sizeof(stopMode));

  printf(HLINE);
  printf("%-22s %s\n", "Solver:", sel->solverName);
  printf("%-22s %s\n", "Variant:", sel->variant->name);
  if (sel->ortho != NULL) {
    printf("%-22s %s\n", "Ortho:", sel->ortho->name);
  }
  printf("%-22s %s\n", "Stop mode:", stopMode);
  printf("%-22s %s\n", "Stop reason:", stopReasonName(res->reason));
  printf("%-22s %d\n", "Iterations:", res->iterations);
  printf("%-22s %.6e s\n", "Solve time:", res->solveTime);
  if (res->iterations > 0) {
    printf("%-22s %.6e s\n", "Time/iter:", res->solveTime / res->iterations);
  } else {
    printf("%-22s n/a\n", "Time/iter:");
  }
  printf("%-22s %.6e\n", "Rel. residual (est.):", (double)res->relResEstimate);
  printf("%-22s %.6e\n", "Rel. residual (true):", (double)res->relResTrue);
  if (res->errMax >= 0.0) {
    printf("%-22s %.6e\n", "Max error:", (double)res->errMax);
  }
}
