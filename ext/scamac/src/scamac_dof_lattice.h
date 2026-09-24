/** \file
 *  \author Andreas Alvermann (University of Greifswald)
 *  \date   June 2018 --- today
 *  \brief  ...
 *  \ingroup internal
 */

#ifndef SCAMAC_DOF_LATTICE_H
#define SCAMAC_DOF_LATTICE_H

#include "scamac_inttypes.h"
#include "scamac_error.h"
#include <stdbool.h>

typedef struct {
  ScamacIdx Lx;
  ScamacIdx Ly;
  ScamacIdx mx;
  ScamacIdx my;
  bool flip_innerrow;
  bool flip_outerrow;
} scamac_dof_lattice_square_st;

typedef struct {
  ScamacIdx Lx;
  ScamacIdx Ly;
  ScamacIdx Lz;
  ScamacIdx mx;
  ScamacIdx my;
  ScamacIdx mz;
} scamac_dof_lattice_cubic_st;

/* particle on a lattice: hexagonal lattice*/

// rectangular piece: Width x Height. Zigzag edges along the "j", armchair along the "i" direction.

ScamacIdx scamac_dof_lattice_hexrect_ns(ScamacIdx W, ScamacIdx H);
void scamac_dof_lattice_hexrect_decode(
    ScamacIdx W, ScamacIdx H, ScamacIdx idx, ScamacIdx *i, ScamacIdx *j);
ScamacIdx scamac_dof_lattice_hexrect_encode(
    ScamacIdx W, ScamacIdx H, ScamacIdx i, ScamacIdx j);

// real-world position (x,y) of site (i,j) of hexagonal lattice. Edge length = 1
void scamac_dof_lattice_hexcoord(ScamacIdx i, ScamacIdx j, double *x, double *y);

/* particle on a lattice: 2D square lattice */
ScamacIdx scamac_dof_lattice_square_ns(const scamac_dof_lattice_square_st *dof);
void scamac_dof_lattice_square_decode(
    const scamac_dof_lattice_square_st *dof, ScamacIdx idx, ScamacIdx *x, ScamacIdx *y);
ScamacIdx scamac_dof_lattice_square_encode(
    const scamac_dof_lattice_square_st *dof, ScamacIdx x, ScamacIdx y);

/* particle on a lattice: 3D cubic lattice */
ScamacIdx scamac_dof_lattice_cubic_ns(const scamac_dof_lattice_cubic_st *dof);
void scamac_dof_lattice_cubic_decode(const scamac_dof_lattice_cubic_st *dof,
    ScamacIdx idx,
    ScamacIdx *x,
    ScamacIdx *y,
    ScamacIdx *z);
ScamacIdx scamac_dof_lattice_cubic_encode(
    const scamac_dof_lattice_cubic_st *dof, ScamacIdx x, ScamacIdx y, ScamacIdx z);

#endif /* SCAMAC_DOF_LATTICE_H */
