/* Copyright (C) NHR@FAU, University Erlangen-Nuremberg.
 * All rights reserved. This file is part of SparseBench.
 * Use of this source code is governed by a MIT style
 * license that can be found in the LICENSE file. */

/* MPI halo exchange regression test.
 *
 * Why this test exists: the CG solver validates itself against an exact
 * solution of ones (x = 1). A halo exchange that permutes or swaps rows still
 * converges to x = 1 in that setup, so the CG check can NOT catch a broken
 * exchange. Here a non-constant field v[i] = (global row id of i) is exchanged
 * and A*v is compared against the analytic stencil result, which makes a
 * misrouted halo contribution visible immediately.
 *
 * Run with:  mpirun -np <P> ./haloCheck [nx ny nz]
 * Exit code 0: all rows on all ranks match the analytic reference. */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../src/comm.h"
#include "../../src/matrix.h"
#include "../../src/parameter.h"
#include "../../src/solver.h"
#include "../../src/util.h"
#include "../../src/vtype.h"

static double analyticAv(const Parameter *p, CG_UINT R, CG_UINT totalRows)
{
  int nx = p->nx, ny = p->ny;
  int iz = (int)(R / ((CG_UINT)nx * ny));
  int iy = (int)((R / (CG_UINT)nx) % (CG_UINT)ny);
  int ix = (int)(R % (CG_UINT)nx);

  double ref = 27.0 * (double)R; // diagonal
  for (int sz = -1; sz <= 1; sz++) {
    for (int sy = -1; sy <= 1; sy++) {
      for (int sx = -1; sx <= 1; sx++) {
        if (sz == 0 && sy == 0 && sx == 0) {
          continue;
        }
        if (ix + sx < 0 || ix + sx >= nx || iy + sy < 0 || iy + sy >= ny) {
          continue;
        }
        long long gc = (long long)R + sz * nx * ny + sy * nx + sx;
        if (gc < 0 || gc >= (long long)totalRows) {
          continue;
        }
        ref -= (double)gc;
      }
    }
  }
  return ref;
}

int main(int argc, char **argv)
{
  Parameter p;
  CommType comm;
  commInit(&comm, argc, argv);
  initParameter(&p);

  /* Small default mesh; override with ./haloCheck nx ny nz */
  p.nx = 8;
  p.ny = 8;
  p.nz = 8;
  if (argc == 4) {
    p.nx = atoi(argv[1]);
    p.ny = atoi(argv[2]);
    p.nz = atoi(argv[3]);
  }

  GMatrix m;
  matrixGenerate(&m, &p, comm.rank, comm.size, false);
  commLocalization(&comm, &m);

  Matrix sm;
#if SCS
  /* Ask for an aggressive permutation: tiny chunks and sorting scope */
  sm.C     = 2;
  sm.sigma = 2;
#endif
  convertMatrix(&sm, &m);

  /* Mirrors what main.c does before handing vectors to the solver */
  commRemapSendIndices(&comm,
#ifdef SCS
      sm.oldToNewPerm
#else
      NULL
#endif
  );

  CG_UINT nr   = sm.nr;
  CG_UINT ncol = sm.nc;
  CG_UINT nrOut = nr;
#ifdef SCS
  /* SCS padded row slots are written by spMVM as well */
  nrOut = sm.nrPadded;
#endif
  V_ELE *v = (V_ELE *)malloc(ncol * sizeof(V_ELE));
  V_ELE *y = (V_ELE *)malloc(nrOut * sizeof(V_ELE));

  /* Non-constant field: value == global row index */
  for (CG_UINT i = 0; i < ncol; i++) {
    v[i] = (V_ELE)(m.startRow + i);
  }
#ifdef SCS
  /* The solver holds its vectors in SCS order, replicate that here */
  V_ELE *tmp = (V_ELE *)malloc(nr * sizeof(V_ELE));
  permute_vector(sm.oldToNewPerm, v, tmp, nr);
  for (CG_UINT i = 0; i < nr; i++) {
    v[i] = tmp[i];
  }
  free(tmp);
#endif

  commExchange(&comm, nr, v);
  spMVM(&sm, v, y);

  int fails = 0;
  for (CG_UINT lr = 0; lr < nr; lr++) {
    CG_UINT R  = m.startRow + lr;
    double ref = analyticAv(&p, R, m.totalNr);
#ifdef SCS
    double got = (double)y[sm.oldToNewPerm[lr]];
#else
    double got = (double)y[lr];
#endif
    if (fabs(got - ref) > 1.0e-9) {
      printf("rank %d row %u: reference %f, computed %f\n",
          comm.rank,
          (unsigned)R,
          ref,
          got);
      fails = 1;
    }
  }

#ifdef _MPI
  int anyFail = 0;
  MPI_Allreduce(&fails, &anyFail, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
  fails = anyFail;
#endif

  if (commIsMaster(&comm)) {
    if (fails) {
      printf("haloCheck: FAIL (halo exchange delivered wrong values)\n");
    } else {
      printf("haloCheck: PASS (%d rank%s, %u local rows on rank 0)\n",
          comm.size,
          comm.size > 1 ? "s" : "",
          (unsigned)nr);
    }
  }

  free(v);
  free(y);
  commFinalize(&comm);
  return fails ? EXIT_FAILURE : EXIT_SUCCESS;
}
