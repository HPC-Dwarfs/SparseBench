/* Copyright (C) NHR@FAU, University Erlangen-Nuremberg.
 * All rights reserved. This file is part of SparseBench.
 * Use of this source code is governed by a MIT style
 * license that can be found in the LICENSE file. */
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "allocate.h"
#include "chebFDSolver.h"
#include "cli.h"
#include "comm.h"
#include "kernel_dispatch.h"
#include "matrix.h"
#include "matrixBinfile.h"
#include "matrixScamac.h"
#include "nvtx_marker.h"
#include "parameter.h"
#include "profiler.h"
#include "solver.h"
#include "timing.h"
#include "util.h"
#include "vtype.h"

#if defined(RUNTIME_BACKEND_IS_CUDA) || defined(RUNTIME_BACKEND_IS_HIP)
#include "cuda_kernels.h"
#endif

// NUMA first-touch fill
static void firstTouchFill(V_ELE *data_ptr, size_t elem_count, V_ELE value)
{
#pragma omp parallel for schedule(OMP_SCHEDULE)
  for (size_t i = 0; i < elem_count; i++) {
    data_ptr[i] = value;
  }
}

static void initMatrix(CommType *c, Parameter *p, GMatrix *m)
{
  if (matrixIsScamac(p->filename)) {
    matrixGenerateScamac(m, p->filename, c->rank, c->size);
  } else if (strcmp(p->filename, "generate") == 0) {
    matrixGenerate(m, p, c->rank, c->size, false);
  } else if (strcmp(p->filename, "generate7P") == 0) {
    matrixGenerate(m, p, c->rank, c->size, true);
  } else {
    char *dot = strrchr(p->filename, '.');
    if (dot == NULL) {
      commAbort(c, "Unknown matrix file format (filename has no extension)!\n");
    } else if (strcmp(dot, ".mtx") == 0) {
      MMMatrix mm;
      MMMatrix mmLocal;

      if (commIsMaster(c)) {
        printf("Read MTX matrix\n");
        MMMatrixRead(&mm, p->filename);
      }

      commDistributeMatrix(c, &mm, &mmLocal);
      matrixConvertfromMM(&mmLocal, m);
      // In the 1-rank build mmLocal.entries aliases mm.entries freeing local is enough
      freeMMMatrix(&mmLocal);
#ifdef _MPI
      if (commIsMaster(c)) {
        freeMMMatrix(&mm);
      }
#endif
    } else if (strcmp(dot, ".bmx") == 0) {
#ifdef _MPI
      if (commIsMaster(c)) {
        printf("Read BMX matrix\n");
      }
      matrixBinRead(m, c, p->filename);
#else
      // Like the sibling arms: an input this build cannot read is a failure, so
      // it must not exit 0 and let a driver record the run as successful.
      commAbort(c, "Binary matrix files are only supported with MPI!\n");
#endif
    } else {
      /* Running on an uninitialized matrix is useless; stop all ranks. */
      commAbort(c, "Unknown matrix file format (expected .mtx or .bmx)");
    }
  }
}

int main(int argc, char **argv)
{
  Parameter param;
  CommType comm;

  commInit(&comm, argc, argv);
  initParameter(&param);
  parseArguments(&comm, &param, argc, argv);
  NVTX_INIT();
#if defined(RUNTIME_BACKEND_IS_CUDA) || defined(RUNTIME_BACKEND_IS_HIP)
  /* Multi-rank GPU runs are not supported yet and must not be silently wrong:
   * gpu_ddot has no counterpart to the commReductionV that solver.c's ddot
   * does, so every rank would converge on its own rank-local dot products.
   * Lifting this needs a device-pointer halo exchange in comm.c plus the
   * allreduce in gpu_ddot — see GPU-Port-Plan.md. */
  if (comm.size > 1) {
    commAbort(&comm,
        "GPU builds are single-rank only (gpu_ddot performs no MPI reduction); "
        "run with one rank.\n");
  }
  gpu_init(param.device);
  /* ChebFD keeps the matrix device-resident as managed memory (prefetched
   * once in solveChebFD) and streams the pinned search-space blocks; the
   * gpu_alloc knob only governs the other benchmarks. */
  if (BenchType == CHEBFD && param.allocType != ALLOC_MANAGED) {
    if (commIsMaster(&comm)) {
      printf("ChebFD: gpu_alloc %s overridden to managed (matrix is prefetched to the "
             "device, search space is streamed from pinned host memory).\n",
          allocTypeName(param.allocType));
    }
    param.allocType = ALLOC_MANAGED;
  }
  gpu_set_alloc_type(param.allocType);
#endif
  commPrintBanner(&comm);
  if (param.verbose > 0 && commIsMaster(&comm)) {
    printParameter(&param);
  }

  double ts;
  GMatrix m;
  NVTX_RANGE_PUSH_C("Main.initMatrix", NVTX_C_CONVERT);
  double timeStart = getTimeStamp();
  printf("Probe Matrix %s\n", param.filename);
  initMatrix(&comm, &param, &m);
  commBarrier();
  double timeStop = getTimeStamp();
  if (commIsMaster(&comm)) {
    printf("Init matrix took %.2fs\n", timeStop - timeStart);
  }
  NVTX_RANGE_POP();
  NVTX_RANGE_PUSH_C("Main.localize+convert", NVTX_C_CONVERT);
  timeStart = getTimeStamp();
  commLocalization(&comm, &m);

  Matrix sm;
#if SCS
  sm.C     = param.C;
  sm.sigma = param.Sigma;
#endif
  convertMatrix(&sm, &m);

#ifdef SCS
  /* The solver holds its vectors in SCS (permuted) order, but the halo pack
   * indices built by commLocalization refer to the original row numbering.
   * Remap them once so commExchange sends the values of the correct rows. */
  commRemapSendIndices(&comm, sm.oldToNewPerm);
#endif

  commBarrier();
  timeStop = getTimeStamp();
  if (commIsMaster(&comm)) {
    printf(
        "Parallel localization and matrix conversion took %.2fs\n", timeStop - timeStart);
  }
  NVTX_RANGE_POP();

  size_t factorFlops[NUMREGIONS] = { 0 };
  size_t factorWords[NUMREGIONS] = { 0 };

  // TODO : update the flops based on V_ELE type
  factorFlops[DDOT]   = m.totalNr;
  factorWords[DDOT]   = 3 * sizeof(CG_FLOAT) * m.totalNr / 2;
  factorFlops[WAXPBY] = m.totalNr;
  factorWords[WAXPBY] = 3 * sizeof(CG_FLOAT) * m.totalNr;
  /* m.nnz / m.totalNnz are allocation upper bounds for generated matrices
   * (27 entries per row regardless of stencil and of boundary truncation), so
   * they would over-report every SpMV rate. The real local count is rowPtr[nr];
   * sum it up to get the global one. */
  CG_FLOAT nnzSum = (CG_FLOAT)m.rowPtr[m.nr];
  commReduction(&nnzSum, SUM);
  size_t globalNnz    = (size_t)nnzSum;

  factorFlops[SPMVM]  = globalNnz;
  factorWords[SPMVM]  = (sizeof(CG_FLOAT) * globalNnz) + (sizeof(CG_UINT) * globalNnz);
  factorFlops[SPMMVM] = factorFlops[SPMVM] * param.blockwidth;
  factorWords[SPMMVM] = factorWords[SPMVM] * param.blockwidth;

#ifdef CRS
  /* The split kernels each only touch part of the matrix, so they need their
   * own nnz counts - charging both the full nnz would report twice the work
   * that is actually done. By construction local + external == globalNnz. */
  CG_UINT localNnzLocal = 0;
  for (CG_UINT i = 0; i < sm.nr; i++) {
    localNnzLocal += sm.rowLocalEnd[i] - sm.rowPtr[i];
  }

  CG_FLOAT localNnzSum = (CG_FLOAT)localNnzLocal;
  CG_FLOAT extNnzSum   = (CG_FLOAT)(sm.rowPtr[sm.nr] - localNnzLocal);
  commReduction(&localNnzSum, SUM);
  commReduction(&extNnzSum, SUM);

  size_t localNnz          = (size_t)localNnzSum;
  size_t extNnz            = (size_t)extNnzSum;

  factorFlops[SPMVM_LOCAL] = localNnz;
  factorWords[SPMVM_LOCAL] = (sizeof(CG_FLOAT) * localNnz) + (sizeof(CG_UINT) * localNnz);
  factorFlops[SPMVM_EXT]   = extNnz;
  factorWords[SPMVM_EXT]   = (sizeof(CG_FLOAT) * extNnz) + (sizeof(CG_UINT) * extNnz);
#endif

  profilerInit(factorFlops, factorWords);

  int k = 0;
  /* Storage for the profiler region sequences at function scope: seq is
   * handed to profilerPrint() after the switch, so pointing it at arrays
   * declared inside the switch would dangle (stack-use-after-scope). */
  int seqCgPlain[3]   = { DDOT, WAXPBY, SPMVM };
  int seqCgOverlap[5] = { DDOT, WAXPBY, SPMVM_LOCAL, SPMVM_EXT, COMM_WAIT };
  int seqSpmvMpi[2]   = { SPMVM, COMM };
  int seqSpmv[1]      = { SPMVM };
  int seqSpmmv[1]     = { SPMMVM };

  int numSeq          = 0;
  int *seq            = NULL;
  int rc              = EXIT_SUCCESS;

  /* Input vectors must span nc (locals + externals after localization); output
   * vectors must span nrPadded because the SCS kernels also write the padded
   * row slots. */
#ifdef SCS
  CG_UINT inSize  = MAX(sm.nc, sm.nrPadded);
  CG_UINT outSize = sm.nrPadded;
#else
  CG_UINT inSize  = sm.nc;
  CG_UINT outSize = sm.nr;
#endif

  switch (BenchType) {
  case CG:
#ifdef USE_OVERLAP_SPMVM
    numSeq = 5;
    seq    = seqCgOverlap;
#else
    numSeq = 3;
    seq    = seqCgPlain;
#endif
    if (commIsMaster(&comm)) {
      printf("Test type: CG\n");
    }
    NVTX_RANGE_PUSH_C("Bench.CG", NVTX_C_CG);
    k = solveCG(&comm, &param, &sm);
    NVTX_RANGE_POP();
    break;

  case SPMV: {
#ifdef _MPI
    /* The exchange is part of every well-formed distributed SpMV benchmark */
    numSeq = 2;
    seq    = seqSpmvMpi;
#else
    numSeq = 1;
    seq    = seqSpmv;
#endif
    if (commIsMaster(&comm)) {
      printf("Test type: SPMVM\n");
    }
    const int itermax = param.itermax;
    V_ELE *x = (V_ELE *)allocate(ARRAY_ALIGNMENT, (size_t)inSize * sizeof(V_ELE));
    V_ELE *y = (V_ELE *)allocate(ARRAY_ALIGNMENT, (size_t)outSize * sizeof(V_ELE));

    // Parallel init for NUMA first-touch — must match spMVM's schedule.
    firstTouchFill(x, inSize, 1.0);
    firstTouchFill(y, outSize, 0.0);

    NVTX_RANGE_PUSH_C("Bench.SPMV", NVTX_C_MATVEC);
    for (k = 1; k < itermax; k++) {
      /* Refresh the halo like a real application would; no-op without MPI. */
      PROFILE(COMM, commExchange(&comm, sm.nr, x));
      PROFILE(SPMVM, SPMVMFUNC(&sm, x, y));
    }
    NVTX_RANGE_POP();
    deallocate(x);
    deallocate(y);
  } break;

  case SPMMV: {
    numSeq = 1;
    seq    = seqSpmmv;
    if (commIsMaster(&comm)) {
      printf("Test type: SPMMVM\n");
    }
    int itermax = param.itermax;
#ifdef SCS
    /* spMMVM stacks a per-thread V_ELE tmp[C * blockwidth] VLA; reject a width
     * that would overflow the worker stack (or a non-positive one, which is a
     * zero-length VLA / a huge unsigned nc) instead of crashing in the kernel.
     * Same limit ChebFD applies to cheb_NS. */
    if (!spMMVMBlockWidthOk(sm.C, param.blockwidth)) {
      if (commIsMaster(&comm)) {
        printf("SPMMV: block width %d is invalid for the SCS spMMVM stack "
               "scratch (C=%llu, limit ~%u bytes/thread); reduce -w or raise "
               "OMP_STACKSIZE.\n",
            param.blockwidth,
            (unsigned long long)sm.C,
            (unsigned)SCS_MAX_SPMMVM_VLA_BYTES);
      }
      rc     = EXIT_FAILURE;
      numSeq = 0;
      break;
    }
#endif
    DMatrix x = { .nr = inSize, .nc = param.blockwidth, .entries = NULL };
    DMatrix y = { .nr = outSize, .nc = param.blockwidth, .entries = NULL };
    x.entries = (V_ELE *)allocate(ARRAY_ALIGNMENT, (size_t)x.nr * x.nc * sizeof(V_ELE));
    y.entries = (V_ELE *)allocate(ARRAY_ALIGNMENT, (size_t)y.nr * y.nc * sizeof(V_ELE));

    // Parallel init for NUMA first-touch — must match spMMVM's schedule.
    firstTouchFill(x.entries, (size_t)x.nr * x.nc, 1.0);
    firstTouchFill(y.entries, (size_t)y.nr * y.nc, 0.0);

    NVTX_RANGE_PUSH_C("Bench.SPMMV", NVTX_C_MATVEC);
    for (k = 1; k < itermax; k++) {
      PROFILE(SPMMVM, SPMMVMFUNC(&sm, &x, &y));
    }
    NVTX_RANGE_POP();
    deallocate(x.entries);
    deallocate(y.entries);
  } break;

  case GMRES:
    /* Same kernels and SpMV path (solverApplyA) as CG */
#ifdef USE_OVERLAP_SPMVM
    numSeq = 5;
    seq    = seqCgOverlap;
#else
    numSeq = 3;
    seq    = seqCgPlain;
#endif
    if (commIsMaster(&comm)) {
      printf("Test type: GMRES\n");
    }
    NVTX_RANGE_PUSH_C("Bench.GMRES", NVTX_C_CG);
    k = solveGMRES(&comm, &param, &sm);
    NVTX_RANGE_POP();
    break;

  case CHEBFD: {
    if (commIsMaster(&comm)) {
      printf("Test type: CHEBFD\n");
    }
#if defined(_MPI)
    if (commIsMaster(&comm)) {
      printf("CURRENTLY CHEB FD doesn't support MPI\n");
    }
    // Fall through to the shared cleanup tail (free sm/m, finalize
    // GPU/LIKWID/comm); skip solveChebFD and the profiler report.
    rc = EXIT_FAILURE;
    break;
#endif
    // ChebFD does its own timing/reporting, so it is left out of the profiler sequence.
    // A negative return means a configuration/validation failure -> propagate a non-zero exit.
    NVTX_RANGE_PUSH_C("Bench.CHEBFD", NVTX_C_FILTER);
    int found = solveChebFD(&comm, &param, &sm);
    NVTX_RANGE_POP();
    if (found < 0) {
      rc = EXIT_FAILURE;
    } else {
      k = found;
    }
    break;
  }

  default:;
  }

  if (rc == EXIT_SUCCESS && numSeq > 0) {
    profilerPrint(&comm, seq, numSeq);
  }
  profilerFinalize();
  NVTX_RANGE_PUSH_C("Main.cleanup", NVTX_C_SETUP);
  freeMatrix(&sm);
  freeGMatrix(&m);
  NVTX_RANGE_POP();

#if defined(RUNTIME_BACKEND_IS_CUDA) || defined(RUNTIME_BACKEND_IS_HIP)
  gpu_finalize();
#endif
  commFinalize(&comm);
  freeParameter(&param);

  return rc;
}
