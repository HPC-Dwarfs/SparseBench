#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "scamac_aux.h"
#include "scamac_sparserow.h"

ScamacErrorCode scamac_sparserow_real_alloc(ScamacIdx n, scamac_sparserow_real_st **srow)
{
  if (n < 1) {
    return scamac_error_set_par(SCAMAC_EINPUT, 1);
  }
  if (!srow) {
    return SCAMAC_ENULL;
  }
  scamac_sparserow_real_st *row = malloc(sizeof *row);
  if (!row) {
    return SCAMAC_EMALLOCFAIL;
  }
  row->nalloc = n;
  row->row    = malloc(row->nalloc * sizeof *(row->row));
  if (!row->row) {
    return SCAMAC_EMALLOCFAIL;
  }
  row->nrow = 0;
  *srow     = row;
  return SCAMAC_EOK;
}

ScamacErrorCode scamac_sparserow_cplx_alloc(ScamacIdx n, scamac_sparserow_cplx_st **srow)
{
  if (n < 1) {
    return scamac_error_set_par(SCAMAC_EINPUT, 1);
  }
  if (!srow) {
    return SCAMAC_ENULL;
  }
  scamac_sparserow_cplx_st *row = malloc(sizeof *row);
  if (!row) {
    return SCAMAC_EMALLOCFAIL;
  }
  row->nalloc = n;
  row->row    = malloc(row->nalloc * sizeof *(row->row));
  if (!row->row) {
    return SCAMAC_EMALLOCFAIL;
  }
  row->nrow = 0;
  *srow     = row;
  return SCAMAC_EOK;
}

ScamacErrorCode scamac_sparserow_real_free(scamac_sparserow_real_st *row)
{
  if (row) {
    free(row->row);
    free(row);
  }
  return SCAMAC_EOK;
}

ScamacErrorCode scamac_sparserow_cplx_free(scamac_sparserow_cplx_st *row)
{
  if (row) {
    free(row->row);
    free(row);
  }
  return SCAMAC_EOK;
}

ScamacErrorCode scamac_sparserow_real_zero(scamac_sparserow_real_st *row)
{
  if (!row) {
    return SCAMAC_ENULL;
  }
  row->nrow = 0;
  return SCAMAC_EOK;
}

ScamacErrorCode scamac_sparserow_cplx_zero(scamac_sparserow_cplx_st *row)
{
  if (!row) {
    return SCAMAC_ENULL;
  }
  row->nrow = 0;
  return SCAMAC_EOK;
}

ScamacErrorCode scamac_sparserow_real_add(
    scamac_sparserow_real_st *row, double alpha, ScamacIdx idx)
{
  if (!row) {
    return SCAMAC_ENULL;
  }

  if (idx >= 0) {
    (row->nrow)++;
    if (row->nrow > row->nalloc) {
      return SCAMAC_ESHORTROW;
    }
    row->row[row->nrow - 1].val = alpha;
    row->row[row->nrow - 1].idx = idx;
  }
  return SCAMAC_EOK;
}

ScamacErrorCode scamac_sparserow_cplx_add(
    scamac_sparserow_cplx_st *row, double complex alpha, ScamacIdx idx)
{
  if (!row) {
    return SCAMAC_ENULL;
  }

  if (idx >= 0) {
    (row->nrow)++;
    if (row->nrow > row->nalloc) {
      return SCAMAC_ESHORTROW;
    }
    row->row[row->nrow - 1].val = alpha;
    row->row[row->nrow - 1].idx = idx;
  }
  return SCAMAC_EOK;
}

static int compare_idxreal(const void *a, const void *b)
{
  const scamac_idxreal_st *mya = (const scamac_idxreal_st *)a;
  const scamac_idxreal_st *myb = (const scamac_idxreal_st *)b;
  if (mya->idx < myb->idx) {
    return -1;
  } else if (mya->idx > myb->idx) {
    return 1;
  } else {
    return 0;
  }
}

ScamacErrorCode scamac_sparserow_real_normalize(
    scamac_sparserow_real_st *row, bool keep_zeros)
{
  if (!row) {
    return SCAMAC_ENULL;
  }

  if (row->nrow > 0) {
    qsort(row->row, row->nrow, sizeof *(row->row), compare_idxreal);

    ScamacIdx i, j;
    i = 0;
    for (j = 1; j < row->nrow; j++) {
      if (row->row[i].idx != row->row[j].idx) {
        if (keep_zeros || row->row[i].val != 0.0) {
          i++;
        }
        if (i < j) {
          row->row[i] = row->row[j];
        }
      } else {
        row->row[i].val += row->row[j].val;
      }
    }
    if (keep_zeros || row->row[i].val != 0.0) {
      i++;
    }
    row->nrow = i;
  }

  return SCAMAC_EOK;
}

ScamacErrorCode scamac_sparserow_real_to_idxval(scamac_sparserow_real_st *row,
    ScamacIdx maxrowlength,
    ScamacIdx *nz,
    ScamacIdx *idx,
    double *val)
{
  if ((!row) || (!nz)) {
    return SCAMAC_ENULL;
  }

  if (row->nrow > 0) {
    if ((idx || val) && (row->nrow > maxrowlength)) {
      return SCAMAC_ESHORTROW;
    }
    *nz = row->nrow;

    if (idx && val) {
      ScamacIdx i;
      for (i = 0; i < row->nrow; i++) {
        idx[i] = row->row[i].idx;
        val[i] = row->row[i].val;
      }
    } else if (idx && !val) {
      ScamacIdx i;
      for (i = 0; i < row->nrow; i++) {
        idx[i] = row->row[i].idx;
      }
    } else if (!idx && val) {
      ScamacIdx i;
      for (i = 0; i < row->nrow; i++) {
        val[i] = row->row[i].val;
      }
    }
    return SCAMAC_EOK;
  } else {
    *nz = 0;
    return SCAMAC_EOK;
  }
}

static int compare_idxcplx(const void *a, const void *b)
{
  const scamac_idxcplx_st *mya = (const scamac_idxcplx_st *)a;
  const scamac_idxcplx_st *myb = (const scamac_idxcplx_st *)b;
  if (mya->idx < myb->idx) {
    return -1;
  } else if (mya->idx > myb->idx) {
    return 1;
  } else {
    return 0;
  }
}

ScamacErrorCode scamac_sparserow_cplx_normalize(
    scamac_sparserow_cplx_st *row, bool keep_zeros)
{
  if (!row) {
    return SCAMAC_ENULL;
  }

  if (row->nrow > 0) {
    qsort(row->row, row->nrow, sizeof *(row->row), compare_idxcplx);

    ScamacIdx i, j;
    i = 0;
    for (j = 1; j < row->nrow; j++) {
      if (row->row[i].idx != row->row[j].idx) {
        if (keep_zeros || row->row[i].val != 0.0) {
          i++;
        }
        if (i < j) {
          row->row[i] = row->row[j];
        }
      } else {
        row->row[i].val += row->row[j].val;
      }
    }
    if (keep_zeros || row->row[i].val != 0.0) {
      i++;
    }
    row->nrow = i;
  }

  return SCAMAC_EOK;
}

ScamacErrorCode scamac_sparserow_cplx_to_idxval(scamac_sparserow_cplx_st *row,
    ScamacIdx maxrowlength,
    ScamacIdx *nz,
    ScamacIdx *idx,
    double complex *val)
{
  if ((!row) || (!nz)) {
    return SCAMAC_ENULL;
  }

  if (row->nrow > 0) {
    if ((idx || val) && (row->nrow > maxrowlength)) {
      return SCAMAC_ESHORTROW;
    }
    *nz = row->nrow;

    if (idx && val) {
      ScamacIdx i;
      for (i = 0; i < row->nrow; i++) {
        idx[i] = row->row[i].idx;
        val[i] = row->row[i].val;
      }
    } else if (idx && !val) {
      ScamacIdx i;
      for (i = 0; i < row->nrow; i++) {
        idx[i] = row->row[i].idx;
      }
    } else if (!idx && val) {
      ScamacIdx i;
      for (i = 0; i < row->nrow; i++) {
        val[i] = row->row[i].val;
      }
    }
    return SCAMAC_EOK;
  } else {
    *nz = 0;
    return SCAMAC_EOK;
  }
}

ScamacErrorCode scamac_sparserow_real_to_idxval_int(
    scamac_sparserow_real_st *row, ScamacIdx maxrowlength, int *nz, int *idx, double *val)
{
  if ((!row) || (!nz)) {
    return SCAMAC_ENULL;
  }

  if (row->nrow > 0) {
    if (row->nrow > INT_MAX) {
      return SCAMAC_EOVERFLOW;
    }
    if ((idx || val) && (row->nrow > maxrowlength)) {
      return SCAMAC_ESHORTROW;
    }

    *nz = (int)row->nrow;

    if (idx && val) {
      int i;
      for (i = 0; i < row->nrow; i++) {
        if (idx[i] > INT_MAX) {
          return SCAMAC_EOVERFLOW;
        }
        idx[i] = (int)row->row[i].idx;
        val[i] = row->row[i].val;
      }
    } else if (idx && !val) {
      int i;
      for (i = 0; i < row->nrow; i++) {
        if (idx[i] > INT_MAX) {
          return SCAMAC_EOVERFLOW;
        }
        idx[i] = (int)row->row[i].idx;
      }
    } else if (!idx && val) {
      int i;
      for (i = 0; i < row->nrow; i++) {
        val[i] = row->row[i].val;
      }
    }
    return SCAMAC_EOK;
  } else {
    *nz = 0;
    return SCAMAC_EOK;
  }
}

ScamacErrorCode scamac_sparserow_cplx_to_idxval_int(scamac_sparserow_cplx_st *row,
    ScamacIdx maxrowlength,
    int *nz,
    int *idx,
    double complex *val)
{
  if ((!row) || (!nz)) {
    return SCAMAC_ENULL;
  }

  if (row->nrow > 0) {
    if (row->nrow > INT_MAX) {
      return SCAMAC_EOVERFLOW;
    }
    if ((idx || val) && (row->nrow > maxrowlength)) {
      return SCAMAC_ESHORTROW;
    }
    *nz = (int)row->nrow;

    if (idx && val) {
      ScamacIdx i;
      for (i = 0; i < row->nrow; i++) {
        if (idx[i] > INT_MAX) {
          return SCAMAC_EOVERFLOW;
        }
        idx[i] = (int)row->row[i].idx;
        val[i] = row->row[i].val;
      }
    } else if (idx && !val) {
      ScamacIdx i;
      for (i = 0; i < row->nrow; i++) {
        if (idx[i] > INT_MAX) {
          return SCAMAC_EOVERFLOW;
        }
        idx[i] = (int)row->row[i].idx;
      }
    } else if (!idx && val) {
      ScamacIdx i;
      for (i = 0; i < row->nrow; i++) {
        val[i] = row->row[i].val;
      }
    }
    return SCAMAC_EOK;
  } else {
    *nz = 0;
    return SCAMAC_EOK;
  }
}
