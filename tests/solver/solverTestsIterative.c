/* Copyright (C) NHR@FAU, University Erlangen-Nuremberg.
 * All rights reserved. This file is part of SparseBench.
 * Use of this source code is governed by a MIT style
 * license that can be found in the LICENSE file. */

/* End-to-end tests of the iterative solvers. Every registered CG and GMRES
 * variant (and every GMRES orthogonalization scheme) is run through its
 * SolveFn on a small generated stencil, and the returned SolverResult is
 * checked against the stopping-criteria and result-reporting specs. Variants
 * added later get these checks without touching this file. */

#include "solverTestsIterative.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/cli.h"
#include "../../src/comm.h"
#include "../../src/matrix.h"
#include "../../src/parameter.h"
#include "../../src/solver.h"
#include "../../src/vtype.h"

#define CHECK(cond, msg, ...)                                                            \
  do {                                                                                   \
    if (!(cond)) {                                                                       \
      printf("    FAIL: " msg "\n", ##__VA_ARGS__);                                      \
      ok = 0;                                                                            \
    }                                                                                    \
  } while (0)

#if PRECISION == 1
#define TEST_EPS 1e-4
#define ROUNDING_TOL 1e-4
#else
#define TEST_EPS 1e-8
#define ROUNDING_TOL 1e-12
#endif

/* Problem setup for one run */
typedef struct {
  const char *matrix; // "generate" or "generate7P"
  int nx, ny, nz;
  int itermax;
  double eps;
  int restart;
  CG_FLOAT rhsScale;
} Problem;

/* One registered solver/orthogonalization combination */
typedef struct {
  int benchType;
  const SolverVariant *variant;
  const OrthoVariant *ortho; // NULL for CG
} Combo;

#define MAX_COMBOS 64

static int collectCombos(Combo *combos, int max)
{
  int n = 0;
  int nv, no;
  const SolverVariant *cg = solverVariants(CG, &nv);
  for (int i = 0; i < nv && n < max; i++) {
    combos[n++] = (Combo) { CG, &cg[i], NULL };
  }
  const SolverVariant *gmres = solverVariants(GMRES, &nv);
  const OrthoVariant *orthos = solverOrthoVariants(&no);
  for (int i = 0; i < nv; i++) {
    for (int j = 0; j < no && n < max; j++) {
      combos[n++] = (Combo) { GMRES, &gmres[i], &orthos[j] };
    }
  }
  return n;
}

static void comboLabel(const Combo *c, char *buf, size_t len)
{
  if (c->ortho != NULL) {
    snprintf(buf, len, "gmres:%s/%s", c->variant->name, c->ortho->name);
  } else {
    snprintf(buf, len, "cg:%s", c->variant->name);
  }
}

static void initProblemParameter(Parameter *param, const Combo *c, const Problem *pb)
{
  initParameter(param);
  setParameterFilename(param, pb->matrix);
  setParameterString(&param->solverVariant, c->variant->name);
  if (c->ortho != NULL) {
    setParameterString(&param->gmresOrtho, c->ortho->name);
  }
  param->nx      = pb->nx;
  param->ny      = pb->ny;
  param->nz      = pb->nz;
  param->itermax = pb->itermax;
  param->eps     = pb->eps;
  param->restart = pb->restart;
}

/* Build the matrix the way main.c does and run the combination through the
 * registry. Returns 0 if the combination is not supported by this build. */
static int runCombo(const Combo *c, const Problem *pb, SolverResult *res)
{
  CommType comm;
  memset(&comm, 0, sizeof(comm));
  comm.rank = 0;
  comm.size = 1;

  Parameter param;
  initProblemParameter(&param, c, pb);

  SolverSelection sel;
  char msg[512];
  if (!solverResolve(c->benchType, &param, &sel, msg, sizeof(msg))) {
    printf("    skipped: %s", msg);
    freeParameter(&param);
    return 0;
  }
  if (sel.ortho != NULL) {
    gmresSetOrtho(sel.ortho->ortho);
  }

  GMatrix gm;
  matrixGenerate(&gm, &param, 0, 1, strcmp(pb->matrix, "generate7P") == 0);
  commLocalization(&comm, &gm);

  Matrix A;
  memset(&A, 0, sizeof(A));
#ifdef SCS
  A.C     = param.C;
  A.sigma = param.Sigma;
#endif
  convertMatrix(&A, &gm);
#ifdef SCS
  commRemapSendIndices(&comm, A.oldToNewPerm);
#endif

  memset(res, 0, sizeof(*res));
  solverSetRhsScale(pb->rhsScale);
  sel.variant->solve(&comm, &param, &A, res);
  solverSetRhsScale(1.0);

  freeMatrix(&A);
  freeGMatrix(&gm);
  freeParameter(&param);
  return 1;
}

/* Tolerance mode converges, and the iteration count does not depend on the
 * scaling of b. */
static int testTolerance(const Combo *c)
{
  int ok     = 1;
  Problem pb = { "generate7P", 8, 8, 8, 1000, TEST_EPS, 30, 1.0 };
  SolverResult res;

  if (!runCombo(c, &pb, &res)) {
    return 1;
  }
  CHECK(res.reason == STOP_CONVERGED, "reason %d, expected converged", res.reason);
  CHECK(res.iterations > 0 && res.iterations < pb.itermax,
      "iterations %d not in (0, %d)",
      res.iterations,
      pb.itermax);
  CHECK(res.relResEstimate <= pb.eps,
      "estimated residual %e > eps %e",
      (double)res.relResEstimate,
      pb.eps);
  CHECK(res.relResTrue <= 10.0 * pb.eps,
      "true residual %e > 10*eps",
      (double)res.relResTrue);
  CHECK(res.errMax >= 0.0 && res.errMax <= 1e3 * pb.eps,
      "max error %e",
      (double)res.errMax);

  SolverResult scaled;
  pb.rhsScale = 1e3;
  runCombo(c, &pb, &scaled);
  CHECK(scaled.reason == STOP_CONVERGED, "scaled rhs: reason %d", scaled.reason);
  CHECK(abs(scaled.iterations - res.iterations) <= 1,
      "scaled rhs: %d iterations, unscaled %d",
      scaled.iterations,
      res.iterations);
  return ok;
}

/* Fixed mode performs exactly itermax iterations, across GMRES restarts. */
static int testFixed(const Combo *c)
{
  int ok     = 1;
  Problem pb = { "generate", 6, 6, 6, 17, 0.0, 5, 1.0 };
  SolverResult res;

  if (!runCombo(c, &pb, &res)) {
    return 1;
  }
  CHECK(res.reason == STOP_ITERMAX, "reason %d, expected itermax", res.reason);
  CHECK(res.iterations == 17, "iterations %d, expected 17", res.iterations);
  CHECK(res.errMax >= 0.0, "exact-solution error missing for a generated matrix");
  return ok;
}

/* b = 0 returns x = 0 without iterating. The exact solution scales with b, so
 * errMax = max|x - 0| checks x = 0. */
static int testZeroRhs(const Combo *c)
{
  int ok     = 1;
  Problem pb = { "generate", 4, 4, 4, 50, 0.0, 30, 0.0 };
  SolverResult res;

  if (!runCombo(c, &pb, &res)) {
    return 1;
  }
  CHECK(res.reason == STOP_ZERO_RHS, "reason %d, expected zero rhs", res.reason);
  CHECK(res.iterations == 0, "iterations %d, expected 0", res.iterations);
  CHECK(res.errMax == 0.0, "x != 0: max|x| = %e", (double)res.errMax);
  CHECK(res.relResTrue == 0.0 && res.relResEstimate == 0.0,
      "residuals %e / %e, expected 0",
      (double)res.relResEstimate,
      (double)res.relResTrue);
  return ok;
}

/* restart >= n on a tiny system: the Krylov space becomes invariant, GMRES
 * breaks down and stops even in fixed mode. */
static int testBreakdown(const Combo *c)
{
  int ok     = 1;
  Problem pb = { "generate7P", 3, 2, 2, 50, 0.0, 20, 1.0 };
  SolverResult res;

  if (c->benchType != GMRES) {
    printf("    n/a (GMRES only)\n");
    return 1;
  }
  if (!runCombo(c, &pb, &res)) {
    return 1;
  }
  CHECK(res.reason == STOP_BREAKDOWN, "reason %d, expected breakdown", res.reason);
  CHECK(res.iterations >= 1 && res.iterations <= 12,
      "iterations %d, expected 1..12 (n = 12)",
      res.iterations);
  CHECK(res.relResTrue <= ROUNDING_TOL,
      "true residual %e not at rounding level",
      (double)res.relResTrue);
  return ok;
}

static int resolveOk(int benchType,
    const char *variant,
    const char *ortho,
    double eps,
    int itermax,
    char *msg,
    size_t len)
{
  Parameter param;
  SolverSelection sel;
  initParameter(&param);
  if (variant != NULL) {
    setParameterString(&param.solverVariant, variant);
  }
  if (ortho != NULL) {
    setParameterString(&param.gmresOrtho, ortho);
  }
  param.eps     = eps;
  param.itermax = itermax;
  msg[0]        = '\0';
  int r         = solverResolve(benchType, &param, &sel, msg, len);
  freeParameter(&param);
  return r;
}

/* The resolver rejects unknown names, invalid stop parameters, and variants
 * the build does not support. */
static int testResolver(void)
{
  int ok = 1;
  char msg[512];

  printf("  resolver:\n");
  CHECK(resolveOk(CG, NULL, NULL, 0.0, 10, msg, sizeof(msg)), "default CG: %s", msg);
  CHECK(!resolveOk(CG, "nope", NULL, 0.0, 10, msg, sizeof(msg)),
      "unknown CG variant accepted");
  CHECK(strstr(msg, "standard") != NULL, "CG message lists no valid names: %s", msg);
  CHECK(!resolveOk(GMRES, "nope", NULL, 0.0, 10, msg, sizeof(msg)),
      "unknown GMRES variant accepted");
  CHECK(!resolveOk(GMRES, NULL, "nope", 0.0, 10, msg, sizeof(msg)),
      "unknown orthogonalization accepted");
#ifndef USE_COMPLEX
  /* In complex builds the variant itself is rejected first */
  CHECK(strstr(msg, "mgs") != NULL, "ortho message lists no valid names: %s", msg);
#endif
  CHECK(resolveOk(CG, NULL, "nope", 0.0, 10, msg, sizeof(msg)),
      "CG rejected the ignored ortho setting: %s",
      msg);
  CHECK(!resolveOk(CG, NULL, NULL, -1e-6, 10, msg, sizeof(msg)), "negative eps accepted");
  CHECK(!resolveOk(GMRES, NULL, NULL, 0.0, 0, msg, sizeof(msg)), "itermax 0 accepted");

  CHECK(!solverCheckSupport("dummy", 0, msg, sizeof(msg)), "empty support mask accepted");
  CHECK(strstr(msg, "dummy") != NULL, "support message does not name the variant");
#ifdef USE_COMPLEX
  CHECK(!resolveOk(GMRES, NULL, NULL, 0.0, 10, msg, sizeof(msg)),
      "GMRES accepted in a complex build");
  CHECK(strstr(msg, "complex") != NULL, "message does not name complex: %s", msg);
#else
  CHECK(
      resolveOk(GMRES, NULL, NULL, 0.0, 10, msg, sizeof(msg)), "default GMRES: %s", msg);
#endif

  return ok;
}

int solverTestsIterative(int argc, char **argv)
{
  (void)argc;
  (void)argv;

  static const struct {
    const char *name;
    int (*fn)(const Combo *);
  } cases[] = {
    { "tolerance", testTolerance },
    { "fixed",     testFixed     },
    { "zero rhs",  testZeroRhs   },
    { "breakdown", testBreakdown },
  };
  const int numCases = (int)(sizeof(cases) / sizeof(cases[0]));

  Combo combos[MAX_COMBOS];
  int numCombos = collectCombos(combos, MAX_COMBOS);

  printf("Running iterative solver tests over %d variant(s):\n", numCombos);

  int total  = 0;
  int passed = 0;
  for (int i = 0; i < numCombos; i++) {
    char label[128];
    comboLabel(&combos[i], label, sizeof(label));
    for (int t = 0; t < numCases; t++) {
      printf("  %s %s:\n", label, cases[t].name);
      fflush(stdout);
      int ok = cases[t].fn(&combos[i]);
      printf(ok ? "    PASS\n" : "    FAIL\n");
      passed += ok;
      total++;
    }
  }

  int ok = testResolver();
  printf(ok ? "    PASS\n" : "    FAIL\n");
  passed += ok;
  total++;

  printf("\nSummary: %d/%d iterative solver tests passed.\n", passed, total);
  return (passed == total) ? 0 : 1;
}
