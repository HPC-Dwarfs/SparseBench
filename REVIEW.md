# SparseBench Code Review

This document summarizes the issues found in the SparseBench application and the changes applied to fix them.

## Build & Environment

### Issue: Default toolchain (`CLANG`) is not installed in this environment
The checked-in `config.mk` selected `TOOLCHAIN = CLANG`, but only `gcc` is available here. The build failed immediately with `clang: No such file or directory`.

**Fix:**
- Changed `config.mk` to use `TOOLCHAIN = GCC`.
- Added an early compiler-availability check in `Makefile` that prints a helpful error when the selected compiler is missing:
  ```
  Compiler "clang" not found. Please install it or set TOOLCHAIN to a supported toolchain in config.mk
  ```

### Verification
```bash
make clean && make
./sparseBench-CRS-GCC -x 10 -y 10 -z 10 -i 20 -t cg
```

---

## Critical Correctness Issues

### 1. Stack-use-after-scope in `main.c`
The profiler sequence arrays (`seqCg`, `secSpmvm`, `secSpmmv`) were declared inside `switch` `case` blocks. Their lifetime ended at the closing brace of each `case`, but `seq` still pointed to them when `profilerPrint` was called later.

**Impact:** Undefined behavior; reliably reproduced as `stack-use-after-scope` with AddressSanitizer in CG, SPMV, and SPMMV modes.

**Fix:** Moved the sequence arrays to function scope so they remain alive through `profilerPrint`.

**Verification:**
```bash
make CFLAGS='-O0 -g -fsanitize=address -fno-omit-frame-pointer' LFLAGS='-g -fsanitize=address -fno-omit-frame-pointer'
ASAN_OPTIONS=detect_leaks=1 ./sparseBench-CRS-GCC -x 10 -y 10 -z 10 -i 20 -t cg
# No ASan errors
```

### 2. `restrict` contract violations in `waxpby` / `ddot`
`solver.h` declared `waxpby` and `ddot` with `restrict` on all pointer arguments, but the CG loop calls them with aliased arrays (e.g. `waxpby(nrow, 1.0, r, beta, p, p)` where `y == w`, and `ddot(nrow, r, r, &rtrans)` where `x == y`). This is undefined behavior and enables the compiler to generate incorrect code.

**Fix:** Removed the `restrict` qualifiers from the aliased pointer parameters in both the declarations (`solver.h`) and the definitions (`solver.c`).

### 3. CCRS backend was non-functional
`src/matrix-CCRS.c` contained two serious bugs:

- `convertMatrix` assigned to the local pointer parameter instead of `*sm`:
  ```c
  sm = (Matrix *)m;   // no effect on caller
  ```
  The caller's `Matrix` therefore remained uninitialized, causing a segmentation fault on first use.
- Even after fixing the assignment, `GMatrix` and `Matrix` have different layouts (`GMatrix.entries` is at offset 64, `Matrix.entries` at offset 40), so a blind struct copy read the wrong pointer.

Additionally, `CGSolver.c::initVectors` only had branches for `#ifdef CRS` and `#elif SCS`, leaving `b`, `x`, and `xexact` uninitialized for CCRS builds.

**Fix:**
- Rewrote `convertMatrix` to allocate a real CCRS `Matrix` and copy the data field-by-field.
- Implemented `spMMVM` for CCRS (it was declared in `matrix.h` but missing).
- Added a CCRS branch in `initVectors` that uses the same row-pointer logic as CRS.
- Updated `main.c` cleanup so CCRS `Matrix` arrays are freed once (they are now separate allocations).

**Verification:**
```bash
make MTX_FMT=CCRS
./sparseBench-CCRS-GCC -x 10 -y 10 -z 10 -i 20 -t cg
# Residual now matches CRS/SCS: ~2.3e-15
```

---

## Other Correctness & Robustness Fixes

### SPMV initialization for non-square matrices
`main.c` allocated the input vector `x` with `m.nc` elements but only initialized `i < m.nr` elements. For non-square matrices this either left entries uninitialized or wrote past the end of `x`.

**Fix:** Initialize `x[0 .. m.nc-1]` and `y[0 .. m.nr-1]` separately.

### Matrix Market file reading lost the number of columns
`MMMatrix` only stored `nr` (rows), not `nc` (columns). `matrixConvertfromMM` always set `m->nc = mm->nr`, so non-square matrices were processed with the wrong column count.

**Fix:**
- Added `nc` to `MMMatrix`.
- Stored `N` from `mm_read_mtx_crd_size`.
- Propagated `nc` through `commDistributeMatrix` and `matrixConvertfromMM`.

### `commAbort` caused help output to exit with failure
`-h` triggered `commAbort(comm, "Finish write matrix")`, printing `Abort: Finish write matrix` and returning `EXIT_FAILURE`.

**Fix:** `-h` now calls `commFinalize` and exits with `EXIT_SUCCESS`.

### Memory leaks
- `solveCG` never freed `r`, `p`, `ap`, `x`, `b`, or `xexact`.
- SPMV/SPMMV test paths never freed their vectors.
- The generated/converted `GMatrix` and `Matrix` arrays were never released.

**Fix:** Added corresponding `deallocate` calls in `solveCG`, SPMV/SPMMV, and at the end of `main`.

### Parameter file parsing
`readParameter` used a `while (!feof(fp))` loop, which can process the last line twice, and used `strncmp` with computed lengths that could match prefixes instead of exact names. It also ignored `blockwidth`, `verbose`, `C`, and `Sigma`.

**Fix:**
- Loop on `fgets(...) != NULL`.
- Use exact `strcmp` matching.
- Added parsing for `blockwidth`, `verbose`, and SCS `C`/`Sigma`.

### `allocate.c` maybe-uninitialized warning
`posix_memalign` only sets `ptr` on success; GCC warned that `ptr` could be used uninitialized.

**Fix:** Initialize `void *ptr = NULL`.

---

## Profiler Reporting Fixes

The profiler multiplied a hard-coded "base words/flops" value by the per-iteration factor from `main.c`. For SPMV this was overridden to the correct byte count, but for WAXPBY, DDOT, and SPMMV the multiplication produced inflated numbers.

**Fix:**
- Set `Regions[i].words = facWords[i]` directly for all regions.
- Corrected the base flop counts:
  - `WAXPBY`: 6 → 3 flops per vector element.
  - `DDOT`: 4 → 2 flops per vector element.
- Corrected `factorWords` in `main.c`:
  - `DDOT`: `3/2 * sizeof * totalNr` → `2 * sizeof * totalNr` (reads `x` and `y`).

Reported rates are now consistent with the actual work performed.

---

## Warning Cleanup

A clean build with `-Wall -Wextra -Wpedantic` produced numerous warnings. The following categories were addressed:

- **Sign-compare:** Loops in `matrix.c`, `matrix-CRS.c`, `matrix-CCRS.c`, `CGSolver.c`, `solver.c`, `comm.c` now use `CG_UINT`/`size_t` consistently.
- **Unused parameters:** Non-MPI stub functions in `comm.c` cast unused parameters to `void`.
- **Unused functions:** `scanMM` is now guarded with `#ifdef _MPI`; unused `dumpMMMatrix` was removed.
- **Unknown OpenMP pragmas:** Pragmas in `solver.c`, `matrix-CRS.c`, `profiler.c`, `profiler.h` are now guarded with `#ifdef _OPENMP` so they do not warn when OpenMP is disabled.
- **Empty translation unit:** `matrixBinfile.c` now provides a dummy typedef when MPI is disabled.
- **External `mmio.c`:** Silenced unused-parameter / unused-variable warnings with `(void)` casts.

**Verification:**
```bash
make CFLAGS='-O3 -ffast-math -Wall -Wextra -Wpedantic'
# 0 warnings
```

---

## Build Matrix

The following configurations were built and run successfully:

| Format | OpenMP | MPI | Result |
|--------|--------|-----|--------|
| CRS    | off    | off | ✅ cg, spmv, spmmv, help |
| CRS    | on     | off | ✅ cg |
| SCS    | off    | off | ✅ cg |
| CCRS   | off    | off | ✅ cg, spmmv |

All configurations above also pass AddressSanitizer leak detection for the tested commands.

### Notes
- MPI-enabled builds could not be exercised in this environment because no MPI implementation is installed.
- The existing unit tests in `tests/` pass the split-SpMV tests. The data-driven solver tests are SCS-specific (they reference SELL chunk/sigma parameters) and are not expected to run meaningfully with CRS; they hang with CRS, which is a pre-existing test-suite limitation rather than a regression from these changes.

---

## Files Modified

- `Makefile`
- `config.mk`
- `src/CGSolver.c`
- `src/affinity.c`
- `src/allocate.c`
- `src/cli.c`
- `src/comm.c`
- `src/main.c`
- `src/matrix-CCRS.c`
- `src/matrix-CRS.c`
- `src/matrix.c`
- `src/matrix.h`
- `src/matrixBinfile.c`
- `src/mmio.c`
- `src/parameter.c`
- `src/profiler.c`
- `src/profiler.h`
- `src/solver.c`
- `src/solver.h`

## Reproduction Script

`run_tests.sh` builds the project with strict warnings and AddressSanitizer and exercises the three benchmark modes plus the help output.
