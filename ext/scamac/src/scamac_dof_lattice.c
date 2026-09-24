#include "scamac_dof_lattice.h"
#include "scamac_safeint.h"

ScamacIdx scamac_lattice_hexrect_ns(ScamacIdx W, ScamacIdx H)
{
  ScamacIdx N;
  N = scamac_safeidx_mult(W, H);
  if (N < 0) {
    return -1;
  }
  if ((W % 2 == 0) && (H % 2 == 1)) {
    return N;
  } else {
    return N - 2;
  }
}

void scamac_lattice_hexrect_decode(
    ScamacIdx W, ScamacIdx H, ScamacIdx idx, ScamacIdx *i, ScamacIdx *j)
{
  SCAMAC_ASSERT(i != NULL);
  SCAMAC_ASSERT(j != NULL);
  ScamacIdx ii, jj;
  if ((W % 2 == 0) && (H % 2 == 1)) {
    jj = idx / W;
    ii = idx - W * jj;
  } else if ((W % 2 == 0) && (H % 2 == 0)) {
    if (idx > W * (H - 1) - 1) {
      jj = (idx + 1) / W;
      ii = (idx + 1) - W * jj;
    } else {
      jj = idx / W;
      ii = idx - W * jj;
    }
  } else if ((W % 2 == 1) && (H % 2 == 0)) {
    if (idx < W - 1) {
      jj = 0;
      ii = idx;
    } else if (idx > W * (H - 1) - 2) {
      jj = (idx + 2) / W;
      ii = (idx + 2) - W * jj;
    } else {
      jj = (idx + 1) / W;
      ii = (idx + 1) - W * jj;
    }
  } else {
    if (idx < W - 1) {
      jj = 0;
      ii = idx;
    } else {
      jj = (idx + 1) / W;
      ii = (idx + 1) - W * jj;
    }
  }
  *i = ii;
  *j = jj;
}

ScamacIdx scamac_lattice_hexrect_encode(
    ScamacIdx W, ScamacIdx H, ScamacIdx i, ScamacIdx j)
{
  if ((i < 0) || (j < 0) || (i >= W) || (j >= H)) {
    return -1;
  }
  if ((W % 2 == 0) && (H % 2 == 1)) {
    return i + W * j;
  } else if ((W % 2 == 0) && (H % 2 == 0)) {
    if (j == H - 1) {
      if ((i == 0) || (i == W - 1)) {
        return -1;
      }
      return (i - 1) + W * j;
    } else {
      return i + W * j;
    }
  } else if ((W % 2 == 1) && (H % 2 == 0)) {
    if (j == 0) {
      if (i == W - 1) {
        return -1;
      }
      return i;
    } else if (j == H - 1) {
      if (i == 0) {
        return -1;
      }
      return i + W * j - 2;
    } else {
      return i + W * j - 1;
    }
  } else {
    if (j == 0) {
      if (i == W - 1) {
        return -1;
      }
      return i;
    } else if (j == H - 1) {
      if (i == W - 1) {
        return -1;
      }
      return i + W * j - 1;
    } else {
      return i + W * j - 1;
    }
  }
}

void scamac_lattice_hexcoord(ScamacIdx i, ScamacIdx j, double *x, double *y)
{
  SCAMAC_ASSERT(x != NULL);
  SCAMAC_ASSERT(y != NULL);
  static const double SQRT3 = 1.732050807568877293527;
  *y                        = 0.5 * SQRT3 * j;
  if ((i % 2) == (j % 2)) {
    *x = 1.5 * i + 0.25;
  } else {
    *x = 1.5 * i - 0.25;
  }
}

ScamacIdx scamac_dof_lattice_square_ns(const scamac_dof_lattice_square_st *dof)
{
  SCAMAC_ASSERT(dof != NULL);
  return scamac_safeidx_mult(dof->Lx, dof->Ly);
}

void scamac_dof_lattice_square_decode(
    const scamac_dof_lattice_square_st *dof, ScamacIdx idx, ScamacIdx *x, ScamacIdx *y)
{
  SCAMAC_ASSERT(dof != NULL);
  SCAMAC_ASSERT(x != NULL);
  SCAMAC_ASSERT(y != NULL);

  ScamacIdx Lx = dof->Lx;
  ScamacIdx Ly = dof->Ly;
  ScamacIdx mx = dof->mx;
  ScamacIdx my = dof->my;

  ScamacIdx Ix, rx, Iy, ry;
  Ix = Lx / mx;
  rx = Lx - mx * Ix;
  Iy = Ly / my;
  ry = Ly - my * Iy;

  ScamacIdx ix, jx, iy, jy;

  iy  = idx / (my * Lx);
  idx = idx - iy * my * Lx;

  if (iy < Iy) {
    ix  = idx / (mx * my);
    idx = idx - ix * mx * my;
  } else {
    ix  = idx / (mx * ry);
    idx = idx - ix * mx * ry;
  }

  if (ix < Ix) {
    jy  = idx / mx;
    idx = idx - jy * mx;
  } else {
    jy  = idx / rx;
    idx = idx - jy * rx;
  }

  jx = idx;

  if (dof->flip_innerrow) {
    if (jy % 2 != 0) {
      if (ix < Ix) {
        jx = mx - 1 - jx;
      } else {
        jx = rx - 1 - jx;
      }
    }
  }

  *x = ix * mx + jx;
  *y = iy * my + jy;

  if (dof->flip_outerrow) {
    if (iy % 2 != 0) {
      *x = Lx - 1 - *x;
    }
  }
}

ScamacIdx scamac_dof_lattice_square_encode(
    const scamac_dof_lattice_square_st *dof, ScamacIdx x, ScamacIdx y)
{
  SCAMAC_ASSERT(dof != NULL);

  ScamacIdx Lx = dof->Lx;
  ScamacIdx Ly = dof->Ly;
  ScamacIdx mx = dof->mx;
  ScamacIdx my = dof->my;

  if ((x < 0) || (y < 0) || (x >= Lx) || (y >= Ly)) {
    return -1;
  }

  ScamacIdx Ix, rx, Iy, ry;
  Ix = Lx / mx;
  rx = Lx - mx * Ix;
  Iy = Ly / my;
  ry = Ly - my * Iy;

  ScamacIdx ix, jx, iy, jy;
  iy = y / my;
  jy = y - iy * my;
  if (dof->flip_outerrow) {
    if (iy % 2 != 0) {
      x = Lx - 1 - x;
    }
  }
  ix = x / mx;
  jx = x - ix * mx;
  if (dof->flip_innerrow) {
    if (jy % 2 != 0) {
      if (ix < Ix) {
        jx = mx - 1 - jx;
      } else {
        jx = rx - 1 - jx;
      }
    }
  }

  ScamacIdx idx;

  idx = iy * my * Lx;
  if (iy < Iy) {
    idx += ix * mx * my;
  } else {
    idx += ix * mx * ry;
  }
  if (ix < Ix) {
    idx += jy * mx;
  } else {
    idx += jy * rx;
  }
  idx += jx;

  return idx;
}

ScamacIdx scamac_dof_lattice_cubic_ns(const scamac_dof_lattice_cubic_st *dof)
{
  SCAMAC_ASSERT(dof != NULL);
  ScamacIdx N;
  N = scamac_safeidx_mult(dof->Lx, dof->Ly);
  if (N > 0) {
    N = scamac_safeidx_mult(N, dof->Lz);
  }
  return N;
}

void scamac_dof_lattice_cubic_decode(const scamac_dof_lattice_cubic_st *dof,
    ScamacIdx idx,
    ScamacIdx *x,
    ScamacIdx *y,
    ScamacIdx *z)
{
  SCAMAC_ASSERT(dof != NULL);
  SCAMAC_ASSERT(x != NULL);
  SCAMAC_ASSERT(y != NULL);
  SCAMAC_ASSERT(z != NULL);

  ScamacIdx Lx = dof->Lx;
  ScamacIdx Ly = dof->Ly;
  ScamacIdx Lz = dof->Lz;
  ScamacIdx mx = dof->mx;
  ScamacIdx my = dof->my;
  ScamacIdx mz = dof->mz;

  ScamacIdx Ix, rx, Iy, ry, Iz, rz;
  Ix = Lx / mx;
  rx = Lx - mx * Ix;
  Iy = Ly / my;
  ry = Ly - my * Iy;
  Iz = Lz / mz;
  rz = Lz - mz * Iz;

  ScamacIdx ix, jx, iy, jy, iz, jz;

  iz  = idx / (Lx * Ly * mz);
  idx = idx - iz * Lx * Ly * mz;

  if (iz < Iz) {
    iy  = idx / (Lx * my * mz);
    idx = idx - iy * Lx * my * mz;
    if (iy < Iy) {
      ix  = idx / (mx * my * mz);
      idx = idx - ix * mx * my * mz;
    } else {
      ix  = idx / (mx * ry * mz);
      idx = idx - ix * mx * ry * mz;
    }
  } else {
    iy  = idx / (Lx * my * rz);
    idx = idx - iy * Lx * my * rz;
    if (iy < Iy) {
      ix  = idx / (mx * my * rz);
      idx = idx - ix * mx * my * rz;
    } else {
      ix  = idx / (mx * ry * rz);
      idx = idx - ix * mx * ry * rz;
    }
  }

  if (ix < Ix) {
    if (iy < Iy) {
      jz  = idx / (mx * my);
      idx = idx - jz * mx * my;
    } else {
      jz  = idx / (mx * ry);
      idx = idx - jz * mx * ry;
    }
    jy  = idx / mx;
    idx = idx - jy * mx;
  } else {
    if (iy < Iy) {
      jz  = idx / (rx * my);
      idx = idx - jz * rx * my;
    } else {
      jz  = idx / (rx * ry);
      idx = idx - jz * rx * ry;
    }
    jy  = idx / rx;
    idx = idx - jy * rx;
  }

  jx = idx;

  *x = ix * mx + jx;
  *y = iy * my + jy;
  *z = iz * mz + jz;
}

ScamacIdx scamac_dof_lattice_cubic_encode(
    const scamac_dof_lattice_cubic_st *dof, ScamacIdx x, ScamacIdx y, ScamacIdx z)
{
  SCAMAC_ASSERT(dof != NULL);

  ScamacIdx Lx = dof->Lx;
  ScamacIdx Ly = dof->Ly;
  ScamacIdx Lz = dof->Lz;
  ScamacIdx mx = dof->mx;
  ScamacIdx my = dof->my;
  ScamacIdx mz = dof->mz;

  if ((x < 0) || (y < 0) || (z < 0) || (x >= Lx) || (y >= Ly) || (z >= Lz)) {
    return -1;
  }

  ScamacIdx Ix, rx, Iy, ry, Iz, rz;
  Ix = Lx / mx;
  rx = Lx - mx * Ix;
  Iy = Ly / my;
  ry = Ly - my * Iy;
  Iz = Lz / mz;
  rz = Lz - mz * Iz;

  ScamacIdx ix, jx, iy, jy, iz, jz;
  iz = z / mz;
  jz = z - iz * mz;
  iy = y / my;
  jy = y - iy * my;
  ix = x / mx;
  jx = x - ix * mx;

  ScamacIdx idx;

  idx = iz * Lx * Ly * mz;
  if (iz < Iz) {
    idx += iy * Lx * my * mz;
    if (iy < Iy) {
      idx += ix * mx * my * mz;
    } else {
      idx += ix * mx * ry * mz;
    }
  } else {
    idx += iy * Lx * my * rz;
    if (iy < Iy) {
      idx += ix * mx * my * rz;
    } else {
      idx += ix * mx * ry * rz;
    }
  }

  if (ix < Ix) {
    idx += jy * mx;
    if (iy < Iy) {
      idx += jz * mx * my;
    } else {
      idx += jz * mx * ry;
    }
  } else {
    idx += jy * rx;
    if (iy < Iy) {
      idx += jz * rx * my;
    } else {
      idx += jz * rx * ry;
    }
  }
  idx += jx;

  return idx;
}
