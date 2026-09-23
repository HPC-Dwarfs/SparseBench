/* Helpers shared by the ChebFD test suites (chebFDUnitTests.c,
 * chebFDStreamTests.c): the 1-D Laplacian test matrix with its closed-form
 * eigenpairs, a deterministic block fill and small comparison utilities.
 * Everything is static inline so the header can be included by each suite. */
#ifndef __CHEBFD_TEST_UTIL_H_
#define __CHEBFD_TEST_UTIL_H_

#include "../../src/allocate.h"
#include "../../src/matrix.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Records a failure in the enclosing test's `ok` flag. */
#define CHECK(cond, msg, ...)                                                            \
  do {                                                                                   \
    if (!(cond)) {                                                                       \
      printf("    FAIL: " msg "\n", ##__VA_ARGS__);                                      \
      ok = 0;                                                                            \
    }                                                                                    \
  } while (0)

/* Symmetric tridiagonal matrix (2 on the diagonal, -1 on the off-diagonals),
 * the standard discrete 1-D Laplacian. Its eigenpairs have a closed form:
 *   lambda_k = 2 - 2*cos(k*pi/(n+1))          (ascending in k)
 *   v_k[j]   = sin((j+1)*k*pi/(n+1))          j = 0..n-1
 * for k = 1..n, which gives an independent ground truth for every ChebFD
 * step without relying on any of the code under test. */
static inline void tridiagEigenvalue(int n, int k, double *lambda)
{
  *lambda = 2.0 - 2.0 * cos((double)k * M_PI / (double)(n + 1));
}

/* Build the tridiagonal matrix above as a GMatrix, then convert it via the
 * format-specific convertMatrix() (CRS or SCS, whichever this test binary
 * was built with). C / sigma are the SCS chunk height and sort window and
 * are ignored on CRS. C=1, sigma=1 disables SCS's row/column permutation and
 * chunk padding, so indices stay in the original 0..n-1 numbering (row
 * lengths differ at the two endpoints, so any sigma>1 WOULD reorder rows).
 * Defined after buildPhaseTridiagMatrix, which it delegates to. */
static inline void fillPhaseTridiagGMatrix(GMatrix *gm, int n, double phi)
{
  V_ELE super = VCONST(-cos(phi), sin(phi));  /* -e^{-i phi}, col i+1 */
  V_ELE sub   = VCONST(-cos(phi), -sin(phi)); /* -e^{+i phi}, col i-1 */

  memset(gm, 0, sizeof(*gm));
  gm->nr       = (CG_UINT)n;
  gm->nc       = (CG_UINT)n;
  gm->nnz      = (CG_UINT)(3 * n - 2);
  gm->totalNr  = (CG_UINT)n;
  gm->totalNnz = gm->nnz;
  gm->startRow = 0;
  gm->stopRow  = (CG_UINT)n;
  gm->rowPtr   = (CG_UINT *)allocate(ARRAY_ALIGNMENT, (size_t)(n + 1) * sizeof(CG_UINT));
  gm->entries  = (Entry *)allocate(ARRAY_ALIGNMENT, (size_t)gm->nnz * sizeof(Entry));

  CG_UINT idx   = 0;
  for (int i = 0; i < n; i++) {
    gm->rowPtr[i] = idx;
    if (i > 0) {
      gm->entries[idx].col = (CG_UINT)(i - 1);
      gm->entries[idx].val = sub;
      idx++;
    }
    gm->entries[idx].col = (CG_UINT)i;
    gm->entries[idx].val = VCONST(2.0, 0.0);
    idx++;
    if (i < n - 1) {
      gm->entries[idx].col = (CG_UINT)(i + 1);
      gm->entries[idx].val = super;
      idx++;
    }
  }
  gm->rowPtr[n] = idx;
}

static inline void buildPhaseTridiagMatrix(
    Matrix *A, GMatrix *gm, int n, int C, int sigma, double phi)
{
  fillPhaseTridiagGMatrix(gm, n, phi);
  memset(A, 0, sizeof(*A));
#ifdef SCS
  A->C     = (CG_UINT)C;
  A->sigma = (CG_UINT)sigma;
#else
  (void)C;
  (void)sigma;
#endif
  convertMatrix(A, gm);
}

/* The plain real Laplacian: the phase-gauged builder at phi = 0, where the
 * gauge factors are exactly +-1. */
static inline void buildTridiagMatrix(Matrix *A, GMatrix *gm, int n, int C, int sigma)
{
  buildPhaseTridiagMatrix(A, gm, n, C, sigma, 0.0);
}

/* Normalized u_k[j] = e^{i j phi} * sin((j+1) k pi/(n+1)) * sqrt(2/(n+1));
 * the normalization uses sum_j sin^2(j k pi/(n+1)) = (n+1)/2 exactly. */
static inline void phaseTridiagEigenvector(int n, int k, double phi, V_ELE *u)
{
  double inv = sqrt(2.0 / (double)(n + 1));
  for (int j = 0; j < n; j++) {
    double s    = sin((double)(j + 1) * (double)k * M_PI / (double)(n + 1)) * inv;
    double arg  = (double)j * phi;
    u[(CG_UINT)j] = VCONST(s * cos(arg), s * sin(arg));
  }
}

/* Dense row-major n x n phase-gauged Laplacian: the same matrix the sparse
 * builders above construct, in dense storage for the direct eigensolver.
 * phi = 0 gives the plain real Laplacian. */
static inline void phaseTridiagDense(V_ELE *a, int n, double phi)
{
  V_ELE super = VCONST(-cos(phi), sin(phi));  /* -e^{-i phi}, col i+1 */
  V_ELE sub   = VCONST(-cos(phi), -sin(phi)); /* -e^{+i phi}, col i-1 */
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      a[i * n + j] = VCONST(0.0, 0.0);
    }
    a[i * n + i] = VCONST(2.0, 0.0);
    if (i > 0) {
      a[i * n + (i - 1)] = sub;
    }
    if (i < n - 1) {
      a[i * n + (i + 1)] = super;
    }
  }
}

/* Deterministic splitmix64-based fill, independent of randomInitBlock in
 * chebFDSolver.c (that one is not exposed, and the tests want their own
 * source of pseudo-random data anyway). */
static inline unsigned long long splitmix64Local(unsigned long long z)
{
  z += 0x9E3779B97F4A7C15ull;
  z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
  z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
  return z ^ (z >> 31);
}

/* Random fill of rows [0, nr), zero padding rows [nr, vecRows), of a
 * row-major vecRows x nc block. Complex builds fill the imaginary part
 * from an independent second hash stream (the real part keeps its values). */
static inline void fillRandomBlock(
    V_ELE *e, CG_UINT vecRows, CG_UINT nr, int nc, unsigned long long seed)
{
  for (CG_UINT r = 0; r < vecRows; r++) {
    for (int c = 0; c < nc; c++) {
      double rv = 0.0;
      double iv = 0.0;
      if (r < nr) {
        unsigned long long h =
            splitmix64Local(seed + r * 0x9E3779B97F4A7C15ull +
                            (unsigned long long)c * 0xff51afd7ed558ccdull);
        rv = (double)(h >> 11) / (double)(1ull << 53) * 2.0 - 1.0;
#ifdef USE_COMPLEX
        unsigned long long h2 =
            splitmix64Local(seed ^ (r * 0x2545F4914F6CDD1Dull +
                                 (unsigned long long)c * 0x9E3779B97F4A7C15ull));
        iv = (double)(h2 >> 11) / (double)(1ull << 53) * 2.0 - 1.0;
#endif
      }
      e[r * (CG_UINT)nc + (CG_UINT)c] = VCONST(rv, iv);
    }
  }
}

static inline double maxAbsDiff(const V_ELE *a, const V_ELE *b, size_t sz)
{
  double maxd = 0.0;
  for (size_t i = 0; i < sz; i++) {
    double d = VABS(a[i] - b[i]);
    /* fmax would silently swallow a NaN difference; surface it instead. */
    maxd = isnan(d) ? INFINITY : fmax(maxd, d);
  }
  return maxd;
}

/* Max deviation from orthogonality of a raw nr x m block under the
 * Hermitian dot: max over |E^H E - I|. */
static inline double gramOffDiagMax(const V_ELE *e, CG_UINT nr, int m)
{
  double maxOff = 0.0;
  for (int i = 0; i < m; i++) {
    for (int j = i; j < m; j++) {
      V_ELE dot = VCONST(0.0, 0.0);
      for (CG_UINT r = 0; r < nr; r++) {
        dot += VCONJ(e[r * (CG_UINT)m + (CG_UINT)i]) *
               e[r * (CG_UINT)m + (CG_UINT)j];
      }
      double target = (i == j) ? 1.0 : 0.0;
      double d      = VABS(dot - VCONST(target, 0.0));
      if (d > maxOff)
        maxOff = d;
    }
  }
  return maxOff;
}

/* Max |G - I| over an already-computed m x m Gram result: 0 iff the block
 * the Gram was formed from is orthonormal. */
static inline double maxDevFromIdentity(const V_ELE *G, int m)
{
  double offd = 0.0;
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < m; j++) {
      offd = fmax(offd, VABS(G[i * m + j] - VCONST(i == j ? 1.0 : 0.0, 0.0)));
    }
  }
  return offd;
}

#endif /* __CHEBFD_TEST_UTIL_H_ */
