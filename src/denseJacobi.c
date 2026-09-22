/* Copyright (C) NHR@FAU, University Erlangen-Nuremberg.
 * All rights reserved. This file is part of SparseBench.
 * Use of this source code is governed by a MIT style
 * license that can be found in the LICENSE file. */
#include "denseJacobi.h"

#include <math.h>
#include <string.h>

// Classic cyclic Jacobi rotations until the off-diagonal norm is negligible.
// Threshold strategy (Schur): skip tiny rotations in the first sweeps.
void jacobiEigen(V_ELE *a, int n, double *eval, V_ELE *evec)
{
  if (n <= 0) {
    return;
  }
  if (n == 1) {
    eval[0] = VREAL(a[0]);
    evec[0] = VCONST(1.0, 0.0);
    return;
  }

  /* eigenvectors start as identity */
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      evec[i * n + j] = (i == j) ? VCONST(1.0, 0.0) : VCONST(0.0, 0.0);
    }
  }

  const int maxsweeps = 100;
  for (int sweep = 0; sweep < maxsweeps; sweep++) {
    /* Sum of |a_pq| over the strict upper triangle. It must be the same measure
     * `thresh` below is compared against (|a_pq|), and it reaches exactly 0.0
     * once every off-diagonal has been rotated/flushed to zero -- an absolute
     * epsilon here never trips for a matrix of O(1..100) scale (round-off keeps
     * the sum near eps*||A||), which would burn all `maxsweeps` sweeps. */
    double off = 0.0;
    for (int p = 0; p < n - 1; p++) {
      for (int q = p + 1; q < n; q++) {
        off += VABS(a[p * n + q]);
      }
    }
    if (off == 0.0) {
      break;
    }
    /* In the first three sweeps rotate only on entries above a threshold. */
    double thresh = (sweep < 3) ? 0.2 * off / ((double)n * n) : 0.0;

    for (int p = 0; p < n - 1; p++) {
      for (int q = p + 1; q < n; q++) {
        V_ELE w     = a[p * n + q]; /* w = a_pq = |w| e^{i gamma} */
        double wabs = VABS(w);
        double g    = 100.0 * wabs;
        double app  = VREAL(a[p * n + p]);
        double aqq  = VREAL(a[q * n + q]);

        if (sweep > 3 && fabs(app) + g == fabs(app) && fabs(aqq) + g == fabs(aqq)) {
          a[p * n + q] = VCONST(0.0, 0.0); /* element negligible */
          a[q * n + p] = VCONST(0.0, 0.0);
        } else if (wabs > thresh) {
          double h = aqq - app;
          double t;
          if (fabs(h) + g == fabs(h)) {
            t = wabs / h;
          } else {
            double theta = 0.5 * h / wabs;
            t            = 1.0 / (fabs(theta) + sqrt(1.0 + theta * theta));
            if (theta < 0.0)
              t = -t;
          }
          double c   = 1.0 / sqrt(1.0 + t * t);
          double s   = t * c;
          double tau = s / (1.0 + c);

          /* unit phase of a_pq; exactly +-1 for real input */
          V_ELE e      = w / (V_ELE)wabs;

          double h_rot = t * wabs;
          a[p * n + p] = VCONST(app - h_rot, 0.0);
          a[q * n + q] = VCONST(aqq + h_rot, 0.0);
          a[p * n + q] = VCONST(0.0, 0.0);
          a[q * n + p] = VCONST(0.0, 0.0);

          for (int k = 0; k < n; k++) {
            if (k != p && k != q) {
              V_ELE akp    = a[k * n + p];
              V_ELE akq    = a[k * n + q];
              a[k * n + p] = akp - s * (VCONJ(e) * akq + tau * akp);
              a[p * n + k] = VCONJ(a[k * n + p]);
              a[k * n + q] = akq + s * (e * akp - tau * akq);
              a[q * n + k] = VCONJ(a[k * n + q]);
            }
            V_ELE vkp       = evec[k * n + p];
            V_ELE vkq       = evec[k * n + q];
            evec[k * n + p] = vkp - s * (VCONJ(e) * vkq + tau * vkp);
            evec[k * n + q] = vkq + s * (e * vkp - tau * vkq);
          }
        }
      }
    }
  }

  for (int i = 0; i < n; i++) {
    eval[i] = VREAL(a[i * n + i]);
  }

  /* selection sort of eigenpairs by ascending eigenvalue (swaps full columns);
   * the sort key is the real eval[] only, never the matrix entries */
  for (int i = 0; i < n - 1; i++) {
    int best = i;
    for (int j = i + 1; j < n; j++) {
      if (eval[j] < eval[best]) {
        best = j;
      }
    }
    if (best != i) {
      double tmp = eval[i];
      eval[i]    = eval[best];
      eval[best] = tmp;
      for (int k = 0; k < n; k++) {
        V_ELE vk           = evec[k * n + i];
        evec[k * n + i]    = evec[k * n + best];
        evec[k * n + best] = vk;
      }
    }
  }
}
