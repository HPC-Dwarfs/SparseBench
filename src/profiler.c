/* Copyright (C) NHR@FAU, University Erlangen-Nuremberg.
 * All rights reserved. This file is part of SparseBench.
 * Use of this source code is governed by a MIT style
 * license that can be found in the LICENSE file. */
#include "profiler.h"
#include "comm.h"
#include "likwid-marker.h"
#include "util.h"
#include "vtype.h"
#include <stddef.h>

typedef struct {
  char *label;
  size_t words;
  size_t flops;
} WorkType;

double T[NUMREGIONS];
size_t NCalls[NUMREGIONS];

/* Regions that are compiled out of the current build have a walltime of zero;
 * report a rate of zero for them instead of dividing by it. */
static double rate(double amount, double time)
{
  return (time > 0.0) ? (1.0E-06 * amount / time) : 0.0;
}

/* Work per single call of each region (scaled by facWords/facFlops in
 * profilerInit); the reported totals are this times NCalls, so solvers with
 * different call patterns (CG, GMRES) are accounted correctly. */
static WorkType Regions[NUMREGIONS] = {
  { "waxpby:  ", 1, 2 },
  { "spMVM:   ", 0, 2 },
  { "spMMVM:  ", 5, 2 },
  { "spMVM_l: ", 0, 2 },
  { "spMVM_e: ", 0, 2 },
  { "ddot:    ", 1, 2 },
  { "comm:    ", 0, 0 },
  { "commwait:", 0, 0 }
};

void profilerInit(size_t *facFlops, size_t *facWords)
{
  LIKWID_MARKER_INIT;
#ifdef _OPENMP
  _Pragma("omp parallel")
  {
    LIKWID_MARKER_REGISTER("WAXPBY");
    LIKWID_MARKER_REGISTER("SPMVM");
    LIKWID_MARKER_REGISTER("SPMMVM");
    LIKWID_MARKER_REGISTER("SPMVM_LOCAL");
    LIKWID_MARKER_REGISTER("SPMVM_EXT");
    LIKWID_MARKER_REGISTER("DDOT");
    LIKWID_MARKER_REGISTER("COMM");
    LIKWID_MARKER_REGISTER("COMM_WAIT");
  }
#else
  LIKWID_MARKER_REGISTER("WAXPBY");
  LIKWID_MARKER_REGISTER("SPMVM");
  LIKWID_MARKER_REGISTER("SPMMVM");
  LIKWID_MARKER_REGISTER("SPMVM_LOCAL");
  LIKWID_MARKER_REGISTER("SPMVM_EXT");
  LIKWID_MARKER_REGISTER("DDOT");
  LIKWID_MARKER_REGISTER("COMM");
  LIKWID_MARKER_REGISTER("COMM_WAIT");
#endif

  for (int i = 0; i < NUMREGIONS; i++) {
    T[i]      = 0.0;
    NCalls[i] = 0;
    Regions[i].flops *= facFlops[i];
    Regions[i].words *= facWords[i];
  }

  Regions[SPMVM].words       = facWords[SPMVM];
  Regions[SPMVM_LOCAL].words = facWords[SPMVM_LOCAL];
  Regions[SPMVM_EXT].words   = facWords[SPMVM_EXT];
}

void profilerPrint(CommType *c, int *seq, int numSeq)
{

  if (c->size > 1) {
#ifdef _MPI
    double tmin[NUMREGIONS];
    double tmax[NUMREGIONS];
    double tavg[NUMREGIONS];

    MPI_Reduce(T, tmin, NUMREGIONS, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(T, tmax, NUMREGIONS, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(T, tavg, NUMREGIONS, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    for (int i = 0; i < NUMREGIONS; i++) {
      tavg[i] /= c->size;
    }

    int commWords = 0;
    for (int i = 0; i < c->outdegree; i++) {
      commWords += c->sendCounts[i];
    }
    for (int i = 0; i < c->indegree; i++) {
      commWords += c->recvCounts[i];
    }

    // The same volume moves either way; COMM measures pack+post, COMM_WAIT the
    // part of the transfer that could not be hidden behind the local SpMV.
    Regions[COMM].words      = sizeof(CG_FLOAT) * commWords;
    Regions[COMM_WAIT].words = sizeof(CG_FLOAT) * commWords;
    int commVolume[c->size];
    MPI_Gather(&commWords, 1, MPI_INT, commVolume, 1, MPI_INT, 0, MPI_COMM_WORLD);
    double commTime[c->size];
    MPI_Gather(&T[COMM], 1, MPI_DOUBLE, commTime, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (commIsMaster(c)) {
      printf(HLINE);
      printf("Function   avg MB/s  avg MFlop/s  Walltime(s) min, max, avg\n");
      for (int s = 0; s < numSeq; s++) {
        int j        = seq[s];
        double bytes = (double)Regions[j].words * NCalls[j];
        double flops = (double)Regions[j].flops * NCalls[j];

        /* Zero accumulated time is legitimate (e.g. `-i 1` never enters the
         * iteration loop); a rate would divide by zero and print inf/nan. */
        if (tavg[j] <= 0.0) {
          printf("%s%11s %11s %11.2f %11.2f %11.2f\n",
              Regions[j].label,
              "n/a",
              "n/a",
              tmin[j],
              tmax[j],
              tavg[j]);
          continue;
        }
        printf("%s%11.2f %11.2f %11.2f %11.2f %11.2f\n",
            Regions[j].label,
            rate(bytes, tavg[j]),
            rate(flops, tavg[j]),
            tmin[j],
            tmax[j],
            tavg[j]);
      }
      printf(HLINE);
      double totalVolume = 0.0;
      printf("Communication\n");
      printf("rank\tkB\tkB/s\tWalltime(s)\n");
      for (int i = 0; i < c->size; i++) {
        double dataVolume = 1.0E-03 * commVolume[i];
        /* T[COMM] stays 0.0 for benchmarks with no COMM region (SPMV/SPMMV). */
        if (commTime[i] <= 0.0) {
          printf("%d %11.2f %11s %11.2e\n", i, dataVolume, "n/a", commTime[i]);
        } else {
          printf("%d %11.2f %11.2f %11.2e\n",
              i,
              dataVolume,
              dataVolume / commTime[i],
              commTime[i]);
        }
        totalVolume += commVolume[i];
      }

      printf("Total data volume %.2f kB\n", 1.0E-03 * totalVolume);
      printf("Walltime(s): min %.2e s, max %.2e s, avg %.2e s\n",
          tmin[COMM],
          tmax[COMM],
          tavg[COMM]);
      printf(HLINE);
    }
#endif
  } else {
    printf(HLINE);
    printf("Function   Rate(MB/s)  Rate(MFlop/s)  Walltime(s)\n");
    for (int j = 0; j < numSeq; j++) {
      int id       = seq[j];
      double bytes = (double)Regions[id].words * NCalls[id];
      double flops = (double)Regions[id].flops * NCalls[id];

      /* See above: no recorded time means no rate, not inf/nan. */
      if (T[id] <= 0.0) {
        printf("%s%11s %11s %11.2f\n", Regions[id].label, "n/a", "n/a", T[id]);
        continue;
      }
      printf("%s%11.2f %11.2f %11.2f\n",
          Regions[id].label,
          rate(bytes, T[id]),
          rate(flops, T[id]),
          T[id]);
    }
    printf(HLINE);
  }
}

void profilerFinalize(void)
{
  LIKWID_MARKER_CLOSE;
}
