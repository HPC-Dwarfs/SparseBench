/* Copyright (C) NHR@FAU, University Erlangen-Nuremberg.
 * All rights reserved. This file is part of SparseBench.
 * Use of this source code is governed by a MIT style
 * license that can be found in the LICENSE file. */
#ifndef __PROFILER_H_
#define __PROFILER_H_
#include "comm.h"
#include "likwid-marker.h"
#include <stddef.h>

#ifdef LIKWID_PERFMON
#ifdef _OPENMP
#define PROFILE_TIMED(tag, call)                                                         \
  _Pragma("omp parallel")                                                                \
  {                                                                                      \
    LIKWID_MARKER_START(#tag);                                                           \
  }                                                                                      \
  ts = getTimeStamp();                                                                   \
  call;                                                                                  \
  T[tag] += (getTimeStamp() - ts);                                                       \
  _Pragma("omp parallel")                                                                \
  {                                                                                      \
    LIKWID_MARKER_STOP(#tag);                                                            \
  }
#else
#define PROFILE_TIMED(tag, call)                                                         \
  LIKWID_MARKER_START(#tag);                                                             \
  ts = getTimeStamp();                                                                   \
  call;                                                                                  \
  T[tag] += (getTimeStamp() - ts);                                                       \
  LIKWID_MARKER_STOP(#tag);
#endif
#else /* LIKWID_PERFMON */
#define PROFILE_TIMED(tag, call)                                                         \
  ts = getTimeStamp();                                                                   \
  call;                                                                                  \
  T[tag] += (getTimeStamp() - ts);
#endif /* LIKWID_PERFMON */

/* PROFILE_TIMED only accumulates time; use it for partial work (e.g. one row
 * chunk of an SpMV) and bump NCalls[tag] once per complete call yourself.
 * profilerPrint charges the per-call work NCalls[tag] times. */
#define PROFILE(tag, call)                                                               \
  PROFILE_TIMED(tag, call)                                                               \
  NCalls[tag]++

typedef enum {
  WAXPBY = 0,
  SPMVM,
  SPMMVM,
  SPMVM_LOCAL,
  SPMVM_EXT,
  DDOT,
  COMM, // packing and posting the halo exchange
  COMM_WAIT, // exposed (non-overlapped) halo exchange time
  NUMREGIONS
} RegionsType;

extern double T[NUMREGIONS];
extern size_t NCalls[NUMREGIONS];
extern void profilerInit(size_t *facFlops, size_t *facWords);
extern void profilerPrint(CommType *c, int *seq, int numSeq);
extern void profilerFinalize(void);
#endif // __PROFILER_H
