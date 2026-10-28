/* Copyright (C) NHR@FAU, University Erlangen-Nuremberg.
 * All rights reserved. This file is part of SparseBench.
 * Use of this source code is governed by a MIT style
 * license that can be found in the LICENSE file. */

/* Randomized cross-check of the SELL-C-sigma conversion and kernels against a
 * plain CRS product computed straight from the input GMatrix. Unlike the
 * convertSCS tests this does not depend on the Python-generated expected
 * files, so a misconception shared by generator and C code still shows up.
 *
 * Per random matrix and (C, sigma) it checks:
 *   - oldToNewPerm is a bijection on [0, nr) and newToOldPerm its inverse
 *   - rows only move inside their sigma window and are sorted there by
 *     descending length
 *   - chunkLens / chunkPtr / nElems describe the padded chunk layout
 *   - spMVM and spMMVM in permuted space equal the reference product, and the
 *     C-padding rows of the result are zero
 *
 * The matrices cover empty rows, nr not a multiple of C, sigma not a multiple
 * of C, sigma larger than nr, and external columns >= nr (as left behind by
 * MPI localization), which must not be permuted.
 */

#include "scsReferenceTests.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/matrix.h"
#include "../../src/solver.h"
#include "../../src/vtype.h"

#ifdef USE_COMPLEX
#define VDIFF(a) VABS(a)
#else
#define VDIFF(a) fabs((double)(a))
#endif

#if PRECISION == 1
#define TOL 1.0e-4
#else
#define TOL 1.0e-10
#endif

#define NUM_VECS 3
#define ROUNDS 20

static unsigned long long rngState = 88172645463325252ull;

static CG_UINT rnd(CG_UINT n)
{
  rngState ^= rngState << 13;
  rngState ^= rngState >> 7;
  rngState ^= rngState << 17;
  return (CG_UINT)(rngState % n);
}

static CG_UINT rowLen(const GMatrix *g, CG_UINT row)
{
  return g->rowPtr[row + 1] - g->rowPtr[row];
}

/* Random nr x (nr + nExt) matrix; about every fifth row is empty. */
static void buildRandomMatrix(GMatrix *g, CG_UINT nr, CG_UINT nExt, CG_UINT maxRow)
{
  memset(g, 0, sizeof(*g));
  g->nr        = nr;
  g->nc        = nr + nExt;
  g->totalNr   = nr;
  g->startRow  = 0;
  g->stopRow   = nr;
  g->rowPtr    = (CG_UINT *)malloc((nr + 1) * sizeof(CG_UINT));
  g->entries   = (Entry *)malloc(((size_t)nr * maxRow + 1) * sizeof(Entry));

  CG_UINT nnz  = 0;
  g->rowPtr[0] = 0;
  for (CG_UINT i = 0; i < nr; i++) {
    CG_UINT len = (rnd(5) == 0) ? 0 : rnd(maxRow + 1);
    for (CG_UINT k = 0; k < len; k++) {
      g->entries[nnz].col = rnd(g->nc);
      g->entries[nnz].val = (V_ELE)((double)(rnd(1000) + 1) / 7.0);
      nnz++;
    }
    g->rowPtr[i + 1] = nnz;
  }
  g->nnz      = nnz;
  g->totalNnz = nnz;
}

/* Returns NULL on success, otherwise a description of the first violation. */
static const char *checkLayout(const GMatrix *g, const Matrix *m)
{
  CG_UINT nr = g->nr;
  char *seen = (char *)calloc(nr ? nr : 1, 1);
  for (CG_UINT i = 0; i < nr; i++) {
    CG_UINT n = m->oldToNewPerm[i];
    if (n >= nr || seen[n] || m->newToOldPerm[n] != i) {
      free(seen);
      return "row permutation is not a bijection with matching inverse";
    }
    seen[n] = 1;
  }
  free(seen);

  for (CG_UINT r = 0; r < nr; r++) {
    if (m->newToOldPerm[r] / m->sigma != r / m->sigma) {
      return "row moved out of its sigma window";
    }
    if (r % m->sigma != 0 &&
        rowLen(g, m->newToOldPerm[r - 1]) < rowLen(g, m->newToOldPerm[r])) {
      return "sigma window not sorted by descending row length";
    }
  }

  CG_UINT offset = 0;
  for (CG_UINT c = 0; c < m->nChunks; c++) {
    CG_UINT maxLen = 0;
    for (CG_UINT k = 0; k < m->C && c * m->C + k < nr; k++) {
      CG_UINT len = rowLen(g, m->newToOldPerm[c * m->C + k]);
      maxLen      = len > maxLen ? len : maxLen;
    }
    if (m->chunkLens[c] != maxLen || m->chunkPtr[c] != offset) {
      return "chunkLens/chunkPtr do not match the longest row per chunk";
    }
    offset += maxLen * m->C;
  }
  if (m->nElems != offset || m->chunkPtr[m->nChunks] != offset) {
    return "nElems does not match the chunk layout";
  }
  return NULL;
}

static const char *checkKernels(const GMatrix *g, Matrix *m)
{
  const char *err = NULL;
  CG_UINT nr      = g->nr;
  CG_UINT nc      = g->nc;
  CG_UINT inSize  = nc > m->nrPadded ? nc : m->nrPadded;

  V_ELE *x        = (V_ELE *)malloc(nc * sizeof(V_ELE));
  V_ELE *xPerm    = (V_ELE *)calloc(inSize, sizeof(V_ELE));
  V_ELE *y        = (V_ELE *)calloc(m->nrPadded, sizeof(V_ELE));
  DMatrix X       = {
    inSize, NUM_VECS, (V_ELE *)calloc((size_t)inSize * NUM_VECS, sizeof(V_ELE))
  };
  DMatrix Y = {
    m->nrPadded, NUM_VECS, (V_ELE *)calloc((size_t)m->nrPadded * NUM_VECS, sizeof(V_ELE))
  };

  /* Local entries go to their permuted slot, externals stay where they are. */
  for (CG_UINT i = 0; i < nc; i++) {
    x[i] = (V_ELE)((double)(rnd(100) + 1) / 3.0);
  }
  for (CG_UINT i = 0; i < nc; i++) {
    xPerm[i < nr ? m->oldToNewPerm[i] : i] = x[i];
  }
  for (CG_UINT i = 0; i < inSize; i++) {
    for (int v = 0; v < NUM_VECS; v++) {
      X.entries[i * NUM_VECS + v] = xPerm[i] * (V_ELE)(v + 1);
    }
  }

  spMVM(m, xPerm, y);
  spMMVM(m, &X, &Y);

  for (CG_UINT i = 0; i < nr && !err; i++) {
    V_ELE ref = 0.0;
    for (CG_UINT j = g->rowPtr[i]; j < g->rowPtr[i + 1]; j++) {
      ref += g->entries[j].val * x[g->entries[j].col];
    }
    CG_UINT row = m->oldToNewPerm[i];
    double tol  = TOL * (1.0 + VDIFF(ref));
    if (VDIFF(y[row] - ref) > tol) {
      err = "spMVM differs from the CRS reference";
    }
    for (int v = 0; v < NUM_VECS && !err; v++) {
      if (VDIFF(Y.entries[row * NUM_VECS + v] - ref * (V_ELE)(v + 1)) > tol * (v + 1)) {
        err = "spMMVM differs from the CRS reference";
      }
    }
  }
  for (CG_UINT i = nr; i < m->nrPadded && !err; i++) {
    if (VDIFF(y[i]) != 0.0) {
      err = "C-padding row of the spMVM result is not zero";
    }
  }

  free(x);
  free(xPerm);
  free(y);
  free(X.entries);
  free(Y.entries);
  return err;
}

int scsReferenceTests(int argc, char **argv)
{
  (void)argc;
  (void)argv;

  const CG_UINT Cs[]     = { 1, 2, 3, 4, 8, 32, 64 };
  const CG_UINT sigmas[] = { 1, 2, 3, 5, 8, 16, 64, 128, 1000 };
  const int numC         = (int)(sizeof(Cs) / sizeof(Cs[0]));
  const int numSigma     = (int)(sizeof(sigmas) / sizeof(sigmas[0]));

  int runs               = 0;
  int failed             = 0;

  for (int round = 0; round < ROUNDS; round++) {
    for (int a = 0; a < numC; a++) {
      for (int b = 0; b < numSigma; b++) {
        CG_UINT nr     = 1 + rnd(300);
        CG_UINT nExt   = rnd(3) ? 0 : rnd(20);
        CG_UINT maxRow = 1 + rnd(12);

        GMatrix g;
        buildRandomMatrix(&g, nr, nExt, maxRow);

        Matrix m;
        memset(&m, 0, sizeof(m));
        m.C     = Cs[a];
        m.sigma = sigmas[b];
        convertMatrix(&m, &g);

        const char *err = checkLayout(&g, &m);
        if (err == NULL) {
          err = checkKernels(&g, &m);
        }
        runs++;
        if (err != NULL) {
          if (failed < 10) {
            printf("    FAIL nr=%llu nc=%llu C=%llu sigma=%llu: %s\n",
                (unsigned long long)nr,
                (unsigned long long)g.nc,
                (unsigned long long)m.C,
                (unsigned long long)m.sigma,
                err);
          }
          failed++;
        }

        freeMatrix(&m);
        free(g.rowPtr);
        free(g.entries);
      }
    }
  }

  printf("Summary: %d/%d SELL-C-sigma reference checks passed.\n", runs - failed, runs);
  return failed != 0;
}
