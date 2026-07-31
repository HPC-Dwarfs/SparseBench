# SparseBench Review: Fixes and Improvements

This document records the outcomes of a code review of SparseBench. Every
confirmed issue from the review (`.work/ISSUES.md`) is listed together with
the implemented fix. Several additional defects found *while* fixing are
documented at the end. All changes are contained in commit(s) on the current
branch.

## 1. Tests segfault when `tests/data/reported/` is missing (fresh clone)

**Problem.** On a fresh clone the git-ignored directory `tests/data/reported/`
does not exist. `fopen(path, "w")` then returned `NULL` and the test code kept
using the handle (`fprintf`/`fclose(NULL)`), segfaulting in `spmvSCS.c`,
`spmmvSCS.c` and `convertSCS.c` (the latter additionally called
`fclose(fptr)` on a possibly `NULL` expected-data handle).

**Fix.** `tests/common.h` gained two tiny helpers used by all data-driven
tests:

* `ensureDir()` creates `data/reported/` (0755, `EEXIST` tolerated) before
  anything is written.
* `xfopen()` reports a descriptive error instead of returning a silently
  `NULL` handle, and every call site now bails out cleanly.

`tests/runTests.c` additionally propagates suite results: it accumulates the
per-suite return codes and the binary now exits non-zero when any suite
failed, printing a final verdict (`All test suites passed.` /
`N test suite(s) reported failures.`).

## 2. Expected test data was never committed

**Problem.** 500 of 603 checks failed on a fresh clone because
`tests/data/expected/*.in` (`*_spmv_x_{2,3}`, all `*_spmmv_x_{1,2,3}`, and
almost all C/σ conversion references) was missing. Root cause: the
`.gitignore` line `data` ignored *every* directory named `data` anywhere in
the tree, silently swallowing `tests/data/expected/`.

**Fix.**

* `.gitignore`: `data` → `/data` (top level only), so `tests/data` is no
  longer ignored; also ignore core dumps (`core`, `core.*`) and the extra
  test binaries (`tests/runTests_asan`, `tests/haloCheck`).
* All 1159 previously missing expected data files (matrix conversion
  references for all C/σ pairs and the SpMV/SpMMV result references for all
  11 test matrices and 3 repeat counts) were added to git. A stale
  `tests/core` crash dump was removed.

## 3. Stale object files when switching `MTX_FMT`/`TOOLCHAIN` or build flags

**Problem.** Test objects lived in the `tests/` source tree and did not depend
on the build configuration at all, so switching `MTX_FMT` or `TOOLCHAIN`
linked stale objects (observed as link errors about missing CRS-only symbols
in SCS builds). The same hazard existed in the main build for *command line*
overrides: `make FLOAT_TYPE=SP` after a `FLOAT_TYPE=DP` build in the same
directory silently kept the old objects because only file mtimes were
considered (config.mk unchanged by the override).

**Fix.**

* `tests/Makefile`: test objects now live in a config-specific directory
  `build/tests-$(MTX_FMT)-$(TOOLCHAIN)/`, depend on `../config.mk` and the
  toolchain include, and the pattern rule mirrors the test source tree. A
  fresh clone without `config.mk` gets the same bootstrap message as the main
  Makefile instead of an obscure include error.
* Both `Makefile` and `tests/Makefile` now maintain a build-flags **stamp
  file** (`$(BUILD_DIR)/.build-flags`): an always-run recipe rewrites it only
  when the effective compiler/flag set actually changes, so `make
  USE_COMPLEX_ELEMENTS=true`, `FLOAT_TYPE=SP`, `ENABLE_MPI=...`, … trigger a
  full rebuild of the affected directory — and identical rebuilds trigger
  nothing. Consuming rules declare the stamp as a normal prerequisite, and
  the default goal is pinned explicitly (`.DEFAULT_GOAL`) because the stamp
  rule now precedes the link rule in the file.
* The application binary used to be relinked on every second invocation once
  the stamp existed (the stamp recipe touched the build directory's mtime);
  `$(BUILD_DIR)` is therefore an *order-only* prerequisite of the binary now.

## 4. SCS + MPI: halo exchange packed rows with the wrong ids

**Problem.** After localization, `elementsToSend[]` holds *original* local row
ids, but all solver vectors are kept in SCS-permuted order (the SCS
conversion permutes both the rows and the local column indices so that no
permutation is needed inside the CG loop). The exchange therefore sent the
values of the wrong rows (`haloCheck` measured maxErr=6). The built-in CG
self-check could not see this because `x_exact == 1` is permutation
invariant; convergence still degraded visibly (final residual 5e-16 for SCS
vs 4e-31 for CRS at iteration 59).

**Fix.**

* New function `commRemapSendIndices(CommType*, const CG_UINT *oldToNewPerm)`
  in `src/comm.c` (declared in `src/comm.h`) remaps the send indices once
  through the SCS row permutation; it is a no-op for non-MPI builds and when
  passed `NULL`.
* `src/main.c` calls it right after `convertMatrix()` under `#ifdef SCS`, i.e.
  as soon as the vectors switch to the permuted numbering.
* New regression test `tests/halo/haloCheck.c` (built via `make -C tests halo`,
  run with `mpirun -np P ./haloCheck [nx ny nz]`): it exchanges the
  non-constant field `v[i] = global row id` and compares `A*v` against the
  analytic 27-point stencil result, which immediately exposes misrouted halo
  values. Passes single-rank for CRS and SCS; the multi-rank MPI case is
  exercised by the same binary.

## 5. CCRS: link failure and no-op conversion

**Problem.** The advertised CCRS format did not link (`spMMVM` missing) and
its `convertMatrix()` was a no-op (`sm = (Matrix*)m` only assigned to the
local parameter copy), leaving the format struct uninitialized.

**Fix.** `src/matrix-CCRS.c`:

* `convertMatrix()` now copies the scalar metadata, duplicates `rowPtr`, and
  packs the `GMatrix` AoS entries into the CCRS `(col, val)` entry array
  (allocation via the project `allocate()` with `ARRAY_ALIGNMENT`).
* Added the missing `spMMVM()` kernel (block-vector SpMV, OpenMP parallel like
  the CRS version).
* `src/CGSolver.c::initVectors()` now serves CCRS through the CRS code path
  (CCRS exposes `rowPtr`), fixing sign-compare warnings there at the same
  time.

Verified: CCRS CG converges bit-identically to CRS (max deviation 1.55e-15)
and `./buildTest.sh GCC false` compiles all 3 formats × {complex,real} ×
{SP,DP} × {U,ULL} = 24 configurations successfully.

## 6. `main.c` SPMV benchmark: external vector slots and missing exchange

**Problem.** Under MPI the extended vector `x` has `m.nc` entries (locals +
externals) but only `m.nr` were initialized, and no halo refresh happened in
the benchmark loop, so the kernel read garbage/none of the external values.

**Fix.** Initialize all `m.nc` entries of `x`, call
`commExchange(&comm, sm.nr, x)` (profiled as `COMM`) in every benchmark
iteration — `commExchange` is a no-op without MPI — and register `{SPMVM,
COMM}` with the profiler in MPI builds.

## 7. `main.c` initMatrix crashes on filenames without extension

**Problem.** `strrchr(filename, '.')` returned `NULL` and `strcmp(NULL, …)`
crashed for e.g. `-m mymatrix`. Unknown extensions merely printed a message
and then continued on an uninitialized matrix.

**Fix.** Guard both `strcmp` calls with `dot != NULL` and abort cleanly via
`commAbort()` ("Unknown matrix file format (expected .mtx or .bmx)") for any
unrecognized or missing extension. `commAbort()` now prints to `stderr` per
rank and flushes all streams before calling `MPI_Abort()`, so the message is
no longer swallowed by buffered stdout.

## 8. `waxpby`/`ddot` declared `restrict` although CG aliases them on purpose

**Problem.** The CG solver calls `waxpby(n, 1.0, v, s, w, v)`-style in-place
updates and `ddot(n, v, v, &r)`; the `restrict` qualifiers made that formally
undefined behavior and triggered `-Wrestrict` at `-O2+`.

**Fix.** Dropped `restrict` from `waxpby`/`ddot` (both `src/solver.h` and
`src/solver.c`) and documented the deliberate aliasing support. (The genuine
`restrict` contracts in the SpMV kernels are unaffected.)

## 9. `buildTest.sh` passed a variable nobody consumes

**Problem.** The script passed `COMPLEX=...`, but the build system only knows
`USE_COMPLEX_ELEMENTS`; "complex" builds silently produced non-complex
binaries.

**Fix.** Pass `USE_COMPLEX_ELEMENTS="$complex"`; the compile logs now show
`-DUSE_COMPLEX` exactly for the complex combinations (verified in the 24
build logs). Additionally the script now covers CCRS (24 builds), computes the
progress total from the array sizes instead of hard-coding 16, and accepts an
optional second argument `[ENABLE_MPI=true|false]` overriding the config value
(default: keep `config.mk` behavior).

## 10. Stale `config.mk` silently disabled the CG overlap

**Problem.** A `config.mk` copied before the overlap feature lacks
`ENABLE_OVERLAP`/`OVERLAP_NUDGE_CHUNKS`; the overlap then silently fell back
to the blocking exchange.

**Fix.** `Makefile` (and identically `tests/Makefile`) now supplies the
defaults (`ENABLE_OVERLAP ?= true`, `OVERLAP_NUDGE_CHUNKS ?= 8`) and derives
the `-DENABLE_OVERLAP` / `-DOVERLAP_NUDGE_CHUNKS=` flags whenever they are not
already part of `DEFINES`/`OPTIONS`. The default `OVERLAP_NUDGE_CHUNKS` still
comes from `src/solver.h` as ultimate fallback, and
`USE_OVERLAP_SPMVM` is defined in `solver.h` (required macro combination
documented there), so `main.c`'s profiler sequence and `CGSolver.c` always
agree.

## 11. `cli.c`: `-h` aborted with a misleading message

**Problem.** `-h` printed the help text and then called
`commAbort(..., "Finish write matrix")` — exit status failure with a
nonsensical message; `-c` behaved the same after the MPI-only
matrix conversion; `cvalue` and friends were unused.

**Fix.** `-h` and `-c` now finalize communication and `exit(EXIT_SUCCESS)`,
with a proper "Finished writing binary matrix file" message for `-c`; the
unused variables and the dead `stop` logic are gone.

## 12. Cosmetic warnings

**Fix.**

* `src/allocate.c`: initialize `ptr = NULL` (maybe-uninitialized).
* `src/parameter.c`: replaced the anti-pattern `while (!feof(fp)) fgets(...)`
  with `while (fgets(...) != NULL)` and dropped the non-portable `"re"` fopen
  mode.
* `src/mmio.c`: removed the unused `error` variable in `mm_typecode_to_str()`.
* `src/main.c`, `src/CGSolver.c`: eliminated sign-compare warnings by using
  `CG_UINT` loop/index variables consistently.
* `src/comm.c`: `scanMM()` is now compiled only under `_MPI` (its sole caller
  lives there), and the debug helper `dumpMMMatrix()` only under
  `_MPI && VERBOSE` (it has no call sites); both changes also fix the
  size_t/int sign-compare warning in `dumpMMMatrix()`.
* `src/CGSolver.c`: removed unused locals (`colInd`, `colIndScs`,
  `nElemsScs`) in the SCS paths.

## Additional defects found and fixed during the review

### A. SCS heap-buffer-overflow in the SPMV/SPMMV benchmarks

`spMVM()`/`spMMVM()` in `src/matrix-SCS.c` write *all* padded row slots
(`nrPadded = nChunks*C ≥ nr`), but `main.c` sized the result vector with only
`m.nr` entries. Any run whose row count is not a multiple of `C` overflowed
the heap (caught with AddressSanitizer at `-x 7 -y 5 -z 9`, C=64, nr=315 →
nrPadded=320). `main.c` now sizes `y` with `sm.nrPadded` for SCS (both the
SPMV and the SPMMV benchmark; documented in a comment).

### B. Stack-use-after-scope of the profiler sequence arrays

The `seqCg[]`/`secSpmvm[]`/`secSpmmv[]` arrays were declared inside the
`switch` block in `main.c` while the pointer (`seq`) was used *after* the
switch by `profilerPrint()`. Their lifetime had formally ended
(AddressSanitizer: `stack-use-after-scope` in `profilerPrint`). All five
sequence arrays now live in function scope.

### C. Alternating no-op relinks

Introduced together with the flags stamp (see item 3) and fixed by making the
build directory an order-only prerequisite of the application binary.

## Verification summary (GCC 14, aarch64 Linux)

* `make MTX_FMT={CRS,SCS,CCRS} TOOLCHAIN=GCC ENABLE_MPI=false`: all build.
* Flag-change invalidation: switching only `USE_COMPLEX_ELEMENTS` recompiles
  all 15 objects; re-running with identical flags recompiles/relinks nothing.
* `make -C tests` + `./runTests`: 100/100 matrix + 3/3 split-SpMV + 300/300
  SpMV + 300/300 SpMMV pass for CRS/SCS/CCRS (SCS runs all suites, CRS/CCRS
  skip the format-specific ones by design), exit code 0; a deleted
  `data/reported/` is recreated automatically.
* `make -C tests halo && ./haloCheck nx ny nz`: PASS for CRS and SCS,
  including sizes not divisible by the SCS chunk size.
* CG converges identically for CRS and CCRS (SCS differs only by floating
  point reordering, as expected); `-h` exits with status 0; unknown matrix
  file extensions abort cleanly instead of crashing.
* `./buildTest.sh GCC false`: 24/24 combinations build successfully.
* AddressSanitizer runs of the benchmark binaries show no memory errors other
  than the pre-existing, deliberate allocate-at-startup/leak-at-exit style
  (matrix and solver vectors live for the whole program lifetime), which was
  left unchanged.
