/* Standalone unit tests for the individual steps of the ChebFD algorithm
 * (src/chebFDSolver.c, src/chebFilter.c, src/denseJacobi.c), each checked
 * against an independent reference computation rather than against each
 * other, so a bug shared between the code under test and the reference
 * can't hide. All blocks and matrices use V_ELE storage: real builds
 * exercise the real-symmetric path (phi = 0 fixtures), complex builds the
 * complex-Hermitian path with genuinely complex off-diagonals (phi != 0).
 * Eigenvectors are only ever checked through eigenvalues, residuals and
 * orthonormality -- never entrywise (phase conventions). */
#include "chebFDUnitTests.h"

#include "../../src/allocate.h"
#include "../../src/chebFDSolver.h"
#include "../../src/chebFilter.h"
#include "../../src/comm.h"
#include "../../src/denseJacobi.h"
#include "../../src/matrix.h"
#include "../../src/parameter.h"
#include "../../src/solver.h"
#include "../common.h"
#include "chebFDTestUtil.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Fixture phase for the shared phase-tridiag builders: real builds exercise
 * the plain real Laplacian (phi = 0), complex builds a genuinely
 * complex-Hermitian gauge. The spectrum is phi-independent either way. */
#ifdef USE_COMPLEX
static const double TEST_PHI = 1.0;
#else
static const double TEST_PHI = 0.0;
#endif

/* ---- Section 1: chebFilterInit (src/chebFilter.c) --------------------- */

/* Direct scalar evaluation of p(x) = sum_n gc[n] T_n(alpha x + beta), kept
 * independent of applyFilter's block/matrix-free recurrence. */
static double evalChebFilterAt(const ChebFilter *f, double x)
{
  double xi = f->alpha * x + f->beta;
  if (xi > 1.0)
    xi = 1.0;
  if (xi < -1.0)
    xi = -1.0;
  double T0 = 1.0, T1 = xi;
  double p = f->gc[0] * T0 + f->gc[1] * T1;
  for (int n = 2; n <= f->Np; n++) {
    double Tn = 2.0 * xi * T1 - T0;
    p += f->gc[n] * Tn;
    T0 = T1;
    T1 = Tn;
  }
  return p;
}

static int testChebFilterInit(void)
{
  int ok = 1;
  printf("  chebFilterInit:\n");

  ChebFilter f;
  CHECK(chebFilterInit(&f, 1.0, 1.0, 0.0, 0.0, 10, KERNEL_LANCZOS, 2) != 0,
      "a==b must be rejected");
  CHECK(chebFilterInit(&f, -1.0, 1.0, 0.2, 0.1, 10, KERNEL_LANCZOS, 2) != 0,
      "lam_lo>=lam_hi must be rejected");
  CHECK(chebFilterInit(&f, -1.0, 1.0, -0.1, 0.1, 1, KERNEL_LANCZOS, 2) != 0,
      "Np<2 must be rejected");

  double a = -1.0, b = 3.0, lamLo = 0.9, lamHi = 1.1;
  int Np = 300;
  CHECK(chebFilterInit(&f, a, b, lamLo, lamHi, Np, KERNEL_LANCZOS, 2) == 0,
      "valid construction should succeed");

  /* gc[0] closed form: kernelFactor(.,0,..) == 1 for every kernel, so
   * gc[0] = (theta_lo - theta_hi)/pi with theta = acos(alpha*x+beta). */
  double alpha   = 2.0 / (b - a);
  double beta    = -(a + b) / (b - a);
  double thetaLo = acos(alpha * lamLo + beta);
  double thetaHi = acos(alpha * lamHi + beta);
  double gc0Ref  = (thetaLo - thetaHi) / M_PI;
  CHECK(fabs(f.gc[0] - gc0Ref) < 1e-12, "gc[0]=%.10g expected %.10g", f.gc[0], gc0Ref);

  /* Deep inside the target interval, p(x) should be close to the window's
   * value of 1; well outside, close to 0. */
  double pIn   = evalChebFilterAt(&f, 1.0);
  double pOut1 = evalChebFilterAt(&f, -0.5);
  double pOut2 = evalChebFilterAt(&f, 2.5);
  CHECK(fabs(pIn - 1.0) < 0.05, "p(1.0)=%.6f should be close to 1", pIn);
  CHECK(fabs(pOut1) < 0.05, "p(-0.5)=%.6f should be close to 0", pOut1);
  CHECK(fabs(pOut2) < 0.05, "p(2.5)=%.6f should be close to 0", pOut2);

  chebFilterFree(&f);
  printf(ok ? "    PASS\n" : "");
  return ok;
}

/* ---- Section 2: jacobiEigen (src/denseJacobi.c) ------------------------ */

/* ||A v - lambda v||_2 for column k of evec against row-major n x n A;
 * the residual is accumulated as sum |d|^2 so it is real for both the
 * real-symmetric and the complex-Hermitian path. */
static double jacobiResidual(
    const V_ELE *A, const V_ELE *evec, double lambda, int n, int k)
{
  V_ELE res2 = VCONST(0.0, 0.0);
  for (int i = 0; i < n; i++) {
    V_ELE avi = VCONST(0.0, 0.0);
    for (int j = 0; j < n; j++) {
      avi += A[i * n + j] * evec[j * n + (CG_UINT)k];
    }
    V_ELE d = avi - lambda * evec[i * n + (CG_UINT)k];
    res2 += d * VCONJ(d);
  }
  return sqrt(VREAL(res2));
}

/* Closed-form check of one jacobiEigen fixture: every eigenvalue must match
 * tridiagEigenvalue and every eigenvector satisfy A v = lambda v against the
 * untouched copy Aref. Returns 1 if all pairs pass (CHECK needs `ok`). */
static int checkTridiagPairs(const V_ELE *Aref,
    const double *eval,
    const V_ELE *evec,
    int n,
    const char *tag)
{
  int ok = 1;
  for (int k = 1; k <= n; k++) {
    double lambdaRef;
    tridiagEigenvalue(n, k, &lambdaRef);
    double got = eval[k - 1];
    CHECK(fabs(got - lambdaRef) < 1e-9,
        "%seval[%d]=%.10g expected %.10g",
        tag,
        k - 1,
        got,
        lambdaRef);

    /* Residual check against the untouched copy. */
    double res = jacobiResidual(Aref, evec, got, n, k - 1);
    CHECK(res < 1e-8,
        "%s||A v_%d - lambda v_%d|| = %.3e too large",
        tag,
        k - 1,
        k - 1,
        res);
  }
  return ok;
}

static int testJacobiEigen(void)
{
  int ok = 1;
  printf("  jacobiEigen:\n");

  const int n = 6;
  V_ELE Aref[36], Awork[36];
  phaseTridiagDense(Aref, n, 0.0);
  memcpy(Awork, Aref, sizeof(Aref));

  double eval[6];
  V_ELE evec[36];
  jacobiEigen(Awork, n, eval, evec);
  ok = checkTridiagPairs(Aref, eval, evec, n, "") && ok;

#ifdef USE_COMPLEX
  /* Complex Hermitian: the phase-gauged Laplacian with phi = pi/2, dense
   * (every off-diagonal purely imaginary). The spectrum is phi-independent
   * (tridiagEigenvalue); eigenvectors are checked via residuals and
   * orthonormality only. */
  V_ELE Bref[36], Bwork[36];
  phaseTridiagDense(Bref, n, M_PI / 2.0);
  memcpy(Bwork, Bref, sizeof(Bwork));

  double beval[6];
  V_ELE bevec[36];
  jacobiEigen(Bwork, n, beval, bevec);
  ok = checkTridiagPairs(Bref, beval, bevec, n, "complex ") && ok;

  /* Complex eigenvectors must be orthonormal under the Hermitian dot. */
  double ortho = gramOffDiagMax(bevec, n, n);
  CHECK(ortho < 1e-8, "complex eigenvector orthonormality residual %.3e", ortho);
#endif /* USE_COMPLEX */

  printf(ok ? "    PASS\n" : "");
  return ok;
}

/* ---- Section 3: fused spMMVM kernels (src/solver.h) -------------------- */

static int testFusedKernels(void)
{
  int ok = 1;
  printf("  spMMVMFused / chebfdOp / waxpby3:\n");

  const int n = 10;
  Matrix A;
  GMatrix gm;
  buildTridiagMatrix(&A, &gm, n, 1, 1);
  CG_UINT vecRows = matrixVecRows(&A);

  int nc          = 4;
  CG_UINT sz      = vecRows * (CG_UINT)nc;
  DMatrix w, q, yRef, yFused, xRef, xFused;
  w.nr = q.nr = yRef.nr = yFused.nr = xRef.nr = xFused.nr = vecRows;
  w.nc = q.nc = yRef.nc = yFused.nc = xRef.nc = xFused.nc = nc;
  w.entries      = (V_ELE *)allocate(ARRAY_ALIGNMENT, (size_t)sz * sizeof(V_ELE));
  q.entries      = (V_ELE *)allocate(ARRAY_ALIGNMENT, (size_t)sz * sizeof(V_ELE));
  yRef.entries   = (V_ELE *)allocate(ARRAY_ALIGNMENT, (size_t)sz * sizeof(V_ELE));
  yFused.entries = (V_ELE *)allocate(ARRAY_ALIGNMENT, (size_t)sz * sizeof(V_ELE));
  xRef.entries   = (V_ELE *)allocate(ARRAY_ALIGNMENT, (size_t)sz * sizeof(V_ELE));
  xFused.entries = (V_ELE *)allocate(ARRAY_ALIGNMENT, (size_t)sz * sizeof(V_ELE));

  fillRandomBlock(w.entries, vecRows, vecRows, nc, 0x1234);
  fillRandomBlock(q.entries, vecRows, vecRows, nc, 0x5678);
  fillRandomBlock(xRef.entries, vecRows, vecRows, nc, 0x9abc);
  memcpy(xFused.entries, xRef.entries, (size_t)sz * sizeof(V_ELE));

  V_ELE cA = (V_ELE)0.7, cP = (V_ELE)(-1.3), cQ = (V_ELE)2.1, gc = (V_ELE)0.37;

  /* spMMVMFused vs. spMMVM+waxpby composition (2-term and 3-term forms). */
  DMatrix Aw;
  Aw.nr      = vecRows;
  Aw.nc      = nc;
  Aw.entries = (V_ELE *)allocate(ARRAY_ALIGNMENT, (size_t)sz * sizeof(V_ELE));
  spMMVM(&A, &w, &Aw);
  waxpby((CG_UINT)sz, cA, Aw.entries, cP, w.entries, yRef.entries);
  waxpby((CG_UINT)sz, (V_ELE)1.0, yRef.entries, cQ, q.entries, yRef.entries);
  spMMVMFused(&A, &w, cA, &w, cP, &q, cQ, &yFused);
  double maxd = maxAbsDiff(yRef.entries, yFused.entries, sz);
  CHECK(maxd < 1e-9, "spMMVMFused (3-term) max|diff|=%.3e", maxd);

  spMMVMFused(&A, &w, cA, &w, cP, NULL, (V_ELE)0.0, &yFused);
  waxpby((CG_UINT)sz, cA, Aw.entries, cP, w.entries, yRef.entries);
  maxd = maxAbsDiff(yRef.entries, yFused.entries, sz);
  CHECK(maxd < 1e-9, "spMMVMFused (2-term) max|diff|=%.3e", maxd);

  /* chebfdOp vs. spMMVMFused + waxpby accumulate, including the in-place
   * y==q aliasing pattern applyFilter's recurrence loop relies on. Recompute
   * the 3-term reference (yRef currently holds the 2-term result above). */
  waxpby((CG_UINT)sz, cA, Aw.entries, cP, w.entries, yRef.entries);
  waxpby((CG_UINT)sz, (V_ELE)1.0, yRef.entries, cQ, q.entries, yRef.entries);
  waxpby((CG_UINT)sz, (V_ELE)1.0, xRef.entries, gc, yRef.entries, xRef.entries);
  chebfdOp(&A, &w, cA, cP, &q, cQ, &yFused, gc, &xFused);
  maxd         = maxAbsDiff(yRef.entries, yFused.entries, sz);
  double maxdx = maxAbsDiff(xRef.entries, xFused.entries, sz);
  CHECK(maxd < 1e-9, "chebfdOp y max|diff|=%.3e", maxd);
  CHECK(maxdx < 1e-9, "chebfdOp x accumulate max|diff|=%.3e", maxdx);

  DMatrix qAlias;
  qAlias.nr      = vecRows;
  qAlias.nc      = nc;
  qAlias.entries = (V_ELE *)allocate(ARRAY_ALIGNMENT, (size_t)sz * sizeof(V_ELE));
  memcpy(qAlias.entries, q.entries, (size_t)sz * sizeof(V_ELE));
  for (CG_UINT i = 0; i < sz; i++) {
    xRef.entries[i] = xFused.entries[i] = (V_ELE)0.5;
  }
  spMMVMFused(&A, &w, cA, &w, cP, &qAlias, cQ, &yRef);
  waxpby((CG_UINT)sz, (V_ELE)1.0, xRef.entries, gc, yRef.entries, xRef.entries);
  chebfdOp(&A, &w, cA, cP, &qAlias, cQ, &qAlias, gc, &xFused);
  maxd  = maxAbsDiff(yRef.entries, qAlias.entries, sz);
  maxdx = maxAbsDiff(xRef.entries, xFused.entries, sz);
  CHECK(maxd < 1e-9, "chebfdOp (y==q aliased) y max|diff|=%.3e", maxd);
  CHECK(maxdx < 1e-9, "chebfdOp (y==q aliased) x max|diff|=%.3e", maxdx);

  /* waxpby3 vs. two waxpby calls. */
  V_ELE *w1 = (V_ELE *)allocate(ARRAY_ALIGNMENT, (size_t)sz * sizeof(V_ELE));
  V_ELE *w2 = (V_ELE *)allocate(ARRAY_ALIGNMENT, (size_t)sz * sizeof(V_ELE));
  waxpby((CG_UINT)sz, (V_ELE)1.1, xRef.entries, (V_ELE)(-0.4), w.entries, w1);
  waxpby((CG_UINT)sz, (V_ELE)1.0, w1, (V_ELE)0.9, q.entries, w1);
  waxpby3((CG_UINT)sz,
      (V_ELE)1.1,
      xRef.entries,
      (V_ELE)(-0.4),
      w.entries,
      (V_ELE)0.9,
      q.entries,
      w2);
  maxd = maxAbsDiff(w1, w2, sz);
  CHECK(maxd < 1e-9, "waxpby3 max|diff|=%.3e", maxd);

  deallocate(w.entries);
  deallocate(q.entries);
  deallocate(yRef.entries);
  deallocate(yFused.entries);
  deallocate(xRef.entries);
  deallocate(xFused.entries);
  deallocate(Aw.entries);
  deallocate(qAlias.entries);
  deallocate(w1);
  deallocate(w2);
  freeMatrix(&A);
  freeGMatrix(&gm);

  printf(ok ? "    PASS\n" : "");
  return ok;
}

/* ---- Section 4: applyFilter (Step 5) ----------------------------------- */

static int testApplyFilter(void)
{
  int ok = 1;
  printf("  applyFilter:\n");

  const int n = 10;
  const double phi = TEST_PHI;
  Matrix A;
  GMatrix gm;
  buildPhaseTridiagMatrix(&A, &gm, n, 1, 1, phi);
  CG_UINT nr = A.nr;

  ChebFilter f;
  double a = 0.0, b = 4.0; /* spectrum lies in (0,4) for every phi */
  /* Target a window around one interior eigenvalue so the filter has real
   * shape to apply, without needing it to be sharp for this test. */
  double lamLo, lamHi;
  tridiagEigenvalue(n, n / 2, &lamLo);
  lamHi = lamLo + 0.3;
  lamLo -= 0.1;
  CHECK(chebFilterInit(&f, a, b, lamLo, lamHi, 60, KERNEL_LANCZOS, 2) == 0,
      "chebFilterInit should succeed");

  ChebData d;
  allocChebData(&d, &A, n); /* NS = n so d.Y holds exactly the eigenbasis */

  /* X = the exact (orthonormal) eigenbasis, column k-1 = u_k. Chebyshev
   * polynomials of A applied to an eigenvector reduce to a scalar multiple
   * of the same eigenvector: p(A) u_k = p(lambda_k) u_k -- this is the
   * independent ground truth applyFilter's block recurrence is checked
   * against. */
  for (CG_UINT r = 0; r < d.Y.nr; r++) {
    for (int k = 0; k < n; k++) {
      d.Y.entries[r * (CG_UINT)n + (CG_UINT)k] = VCONST(0.0, 0.0);
    }
  }
  V_ELE *v = (V_ELE *)malloc((size_t)n * sizeof(V_ELE));
  for (int k = 1; k <= n; k++) {
    phaseTridiagEigenvector(n, k, phi, v);
    for (int j = 0; j < n; j++) {
      d.Y.entries[(CG_UINT)j * (CG_UINT)n + (CG_UINT)(k - 1)] = v[j];
    }
  }

  d.Y.nc = n;
  applyFilter(&A, &f, &d.Y, &d.u, &d.w);

  for (int k = 1; k <= n; k++) {
    double lambdaK;
    tridiagEigenvalue(n, k, &lambdaK);
    double pLambda = evalChebFilterAt(&f, lambdaK);
    phaseTridiagEigenvector(n, k, phi, v);

    V_ELE diff2 = VCONST(0.0, 0.0);
    for (int j = 0; j < n; j++) {
      V_ELE got = d.Y.entries[(CG_UINT)j * (CG_UINT)n + (CG_UINT)(k - 1)];
      V_ELE dd  = got - (V_ELE)pLambda * v[j];
      diff2 += dd * VCONJ(dd);
    }
    CHECK(sqrt(VREAL(diff2)) < 1e-6,
        "column %d: ||Y[:,k] - p(lambda_k) u_k|| = %.3e too large",
        k - 1,
        sqrt(VREAL(diff2)));
  }

  free(v);
  freeChebData(&d);
  chebFilterFree(&f);
  freeMatrix(&A);
  freeGMatrix(&gm);
  (void)nr;

  printf(ok ? "    PASS\n" : "");
  return ok;
}

/* ---- Section 5: orthoMGS (Step 6) -------------------------------------- */

static int testOrthoMGS(void)
{
  int ok = 1;
  printf("  orthoMGS:\n");

  CG_UINT nr = 50;
  int nc     = 8;
  V_ELE *e = (V_ELE *)allocate(ARRAY_ALIGNMENT, (size_t)nr * (size_t)nc * sizeof(V_ELE));

  fillRandomBlock(e, nr, nr, nc, 0xdeadbeef);
  int m = orthoMGS(nr, e, nc, 1e-8);
  CHECK(m == nc, "generic random block: m=%d expected %d (full rank)", m, nc);
  double off = gramOffDiagMax(e, nr, m);
  CHECK(off < 1e-6, "orthonormality residual %.3e too large", off);

  /* Rank-deficient block: column 1 is an exact copy of column 0. */
  deallocate(e);
  e = (V_ELE *)allocate(ARRAY_ALIGNMENT, (size_t)nr * (size_t)nc * sizeof(V_ELE));
  fillRandomBlock(e, nr, nr, nc, 0xfeedface);
  for (CG_UINT r = 0; r < nr; r++) {
    e[r * (CG_UINT)nc + 1] = e[r * (CG_UINT)nc + 0];
  }
  m = orthoMGS(nr, e, nc, 1e-8);
  CHECK(m == nc - 1, "rank-deficient block: m=%d expected %d", m, nc - 1);
  off = gramOffDiagMax(e, nr, m);
  CHECK(off < 1e-6, "orthonormality residual %.3e too large after drop", off);

  deallocate(e);
  printf(ok ? "    PASS\n" : "");
  return ok;
}

/* ---- Section 6: rayleighRitz + residual (Steps 7-8) -------------------- */

static int testRayleighRitzAndResidual(void)
{
  int ok = 1;
  printf("  rayleighRitz / computeRitzResidual / residualNorm:\n");

  const int n = 8;
  const double phi = TEST_PHI;
  Matrix A;
  GMatrix gm;
  buildPhaseTridiagMatrix(&A, &gm, n, 1, 1, phi);
  CG_UINT nr = A.nr;

  ChebData d;
  allocChebData(&d, &A, n);

  /* Y = the exact orthonormal eigenbasis -> H = Y^H A Y should come out
   * exactly diagonal (up to FP error), so rayleighRitz's Ritz values must
   * reproduce the closed-form eigenvalues and the residuals must vanish. */
  V_ELE *v = (V_ELE *)malloc((size_t)n * sizeof(V_ELE));
  for (int k = 1; k <= n; k++) {
    phaseTridiagEigenvector(n, k, phi, v);
    for (int j = 0; j < n; j++) {
      d.Y.entries[(CG_UINT)j * (CG_UINT)n + (CG_UINT)(k - 1)] = v[j];
    }
  }
  d.Y.nc = n;

  rayleighRitz(&A, &d.Y, &d.AY, n, nr, d.H, d.eval, d.evec);

  for (int k = 1; k <= n; k++) {
    double lambdaRef;
    tridiagEigenvalue(n, k, &lambdaRef);
    double got = d.eval[k - 1];
    CHECK(fabs(got - lambdaRef) < 1e-8,
        "eval[%d]=%.10g expected %.10g",
        k - 1,
        got,
        lambdaRef);

    computeRitzResidual(&d.Y, &d.AY, n, nr, got, d.evec, k - 1, d.evk, d.avbuf);
    double res = residualNorm(nr, d.avbuf);
    CHECK(res < 1e-8, "residual for Ritz pair %d = %.3e too large", k - 1, res);
  }

  free(v);
  freeChebData(&d);
  freeMatrix(&A);
  freeGMatrix(&gm);

  printf(ok ? "    PASS\n" : "");
  return ok;
}

/* ---- Section 7: solveChebFD end-to-end sanity check --------------------- */

static int testSolveChebFDEndToEnd(void)
{
  int ok = 1;
  printf("  solveChebFD (end-to-end):\n");

  const int n = 40;
  const double phi = TEST_PHI; /* complex-Hermitian end-to-end run on complex builds */
  Matrix A;
  GMatrix gm;
  buildPhaseTridiagMatrix(&A, &gm, n, 1, 1, phi);

  /* Target a window around three consecutive interior eigenvalues (the
   * spectrum is phi-independent, so the same window serves both paths). */
  int kMid = n / 2;
  double lamLo, lamMid, lamHi;
  tridiagEigenvalue(n, kMid - 1, &lamLo);
  tridiagEigenvalue(n, kMid, &lamMid);
  tridiagEigenvalue(n, kMid + 1, &lamHi);
  (void)lamMid;
  lamLo -= 0.05;
  lamHi += 0.05;

  CommType comm;
  memset(&comm, 0, sizeof(comm));
  comm.rank = 0;
  comm.size = 1;

  Parameter param;
  memset(&param, 0, sizeof(param));
  param.eps              = 1e-10;
  param.itermax          = 60;
  param.verbose          = 0;
  param.cheb.lam_lo      = lamLo;
  param.cheb.lam_hi      = lamHi;
  param.cheb.Np          = 80;
  param.cheb.NS          = 10;
  param.cheb.kernel      = 3; /* Lanczos */
  param.cheb.mu          = 2;
  param.cheb.have_bounds = 0;
  param.cheb.have_target = 1;

  int found              = solveChebFD(&comm, &param, &A);
  CHECK(
      found == 3, "found %d eigenpairs in [%.4f, %.4f], expected 3", found, lamLo, lamHi);

  freeMatrix(&A);
  freeGMatrix(&gm);
  printf(ok ? "    PASS\n" : "");
  return ok;
}

int chebFDUnitTests(int argc, char **argv)
{
  (void)argc;
  (void)argv;

  printf("Running ChebFD unit tests:\n");

  int results[7];
  int i        = 0;
  results[i++] = testChebFilterInit();
  results[i++] = testJacobiEigen();
  results[i++] = testFusedKernels();
  results[i++] = testApplyFilter();
  results[i++] = testOrthoMGS();
  results[i++] = testRayleighRitzAndResidual();
  results[i++] = testSolveChebFDEndToEnd();

  int passed   = 0;
  for (int j = 0; j < i; j++) {
    passed += results[j] ? 1 : 0;
  }
  printf("\nSummary: %d/%d ChebFD unit test sections passed.\n", passed, i);
  return (passed == i) ? 0 : 1;
}
