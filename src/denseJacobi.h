/* Copyright (C) NHR@FAU, University Erlangen-Nuremberg.
 * All rights reserved. This file is part of SparseBench.
 * Use of this source code is governed by a MIT style
 * license that can be found in the LICENSE file. */
#ifndef __DENSEJACOBI_H_
#define __DENSEJACOBI_H_

#include "vtype.h"

// Self-contained cyclic Jacobi eigensolver for a real-symmetric or
// complex-Hermitian matrix
void jacobiEigen(V_ELE *a, int n, double *eval, V_ELE *evec);

#endif // __DENSEJACOBI_H_
