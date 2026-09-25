# Tasks

## 1. Result and registry types

- [x] 1.1 Add `StopReason`, `SolverResult`, `SolveFn`, `SolverVariant` and `OrthoFn` types to `src/solver.h` (design D1, D4). Verify: the CRS, SCS and CCRS builds compile without warnings.
- [x] 1.2 Create `src/solverRegistry.c` with the CG table (`standard`), the GMRES table (`standard`), and the ortho table (`mgs`). Each entry carries its support mask and its profiler sequence (`#ifdef USE_OVERLAP_SPMVM`). Also add lookup functions that return NULL for unknown names, plus a helper that prints the valid names. Verify: builds with and without `ENABLE_OVERLAP`, with MPI enabled.

## 2. CLI and parameter file

- [x] 2.1 Add `solverVariant` and `gmresOrtho` string fields to `Parameter`, with defaults `"standard"`/`"mgs"`, freed in `freeParameter`. Add `solver_variant` and `gmres_ortho` keys to `readParameter`. Verify: a parameter file containing both keys shows them in `printParameter`.
- [x] 2.2 Add the `-a <name>` and `-o <name>` options to `BASE_ARGS_COMMON` and `parseArguments`. Update `HELPTEXT_BASE` for `-a`, `-o`, and `-e` (now "relative tolerance ||r||/||b||; 0 = fixed iterations"). Verify: `-h` shows the new text, and `-a standard -o mgs` is accepted.
- [x] 2.3 Extend `printParameter`: print the variant, and the ortho scheme for GMRES. Print the stop mode as `fixed (itermax)` or `relative tolerance <eps>`. Verify: visible in the output of `-t gmres -e 1e-6`.

## 3. Validation and dispatch in main

- [x] 3.1 After `parseArguments`, resolve the variant and ortho names against the registry for the selected `BenchType`, check the support mask, and validate `eps >= 0` and `itermax >= 1`. Abort through `commAbort` with the list of valid names (design D2). Verify: `-t cg -a nope`, `-t gmres -o nope` and `-e -1` all exit non-zero with the expected message, before the matrix-setup output appears.
- [x] 3.2 Move the complex-build GMRES rejection into the support mask of the GMRES `standard` entry. Verify: in a `USE_COMPLEX` build, `-t gmres` is rejected at validation time.
- [x] 3.3 Replace the direct `solveCG`/`solveGMRES` calls and the hard-coded `seqCgPlain`/`seqCgOverlap` selection in the CG and GMRES cases of `main.c` with a call through the resolved `SolverVariant` and its `profSeq`. Verify: the profiler table for `-t cg` and `-t gmres` lists the same regions as before this change.

## 4. Stopping logic

- [x] 4.1 Add `solverStopInit` to `src/solverCommon.c`. It computes `||b||` with `DDOTFUNC` and returns `absTol` (`eps*||b||`, or `-1` in fixed mode), plus a zero-rhs flag (design D5). Verify: unit-tested in 7.2.
- [x] 4.2 Restructure the CG loop (design D6):
  - `k` counts completed iterations, and the loop ends at `k == itermax`, at `normr <= absTol`, or at `rtrans == 0` (`STOP_EXACT`).
  - A zero rhs returns `STOP_ZERO_RHS` with `x = 0`.
  - A zero `p·Ap` with non-zero `rtrans` aborts with a "matrix not SPD" message.

  Verify: `-t cg -i 50` reports exactly 50 iterations, and the per-iteration kernel sequence is unchanged.
- [x] 4.3 Refactor the MGS loop of `GMRESSolver.c` into an `OrthoFn` implementation selected through the registry (design D4). Verify: fixed-mode `-t gmres -i 100 -r 30` gives the same residual history as before the refactor.
- [x] 4.4 Switch GMRES to `absTol` at both check points. Stop with `STOP_BREAKDOWN` after finishing the cycle in which breakdown occurs, instead of restarting (design D7). Handle a zero rhs as in CG. Verify: covered by the breakdown test in 7.2.

## 5. Result reporting

- [x] 5.1 Change `solverCheckResidual` to return `max|x - xexact|` (or `< 0` if there is no exact solution) instead of printing it. Add a helper for the true relative residual `||b - Ax||/||b||`. Both run after the timer stops. Verify: builds; values checked in 7.2.
- [x] 5.2 Fill a `SolverResult` in CG and GMRES, and remove their ad-hoc final prints. Keep the progress lines. Verify: no duplicate result lines in the output.
- [x] 5.3 Add `solverPrintResult` with the fixed field labels and order from design D8. Call it from `main.c` before `profilerPrint`. Omit `Ortho` for CG and `Max error` when it is unknown. Verify: `-t cg -e 1e-8` and `-t gmres -m <mtx file>` print the expected fields.

## 6. Documentation

- [x] 6.1 Update README:
  - usage and options: `-a`, `-o`, relative `eps`, fixed-iteration mode;
  - an example of each stop mode;
  - a note on the **BREAKING** `eps` change and the exact CG iteration count;
  - GMRES moved from "planned" to available.

  Verify: README renders and the examples run as documented.

## 7. Tests

- [x] 7.1 Add the `tests/solver/solverTestsIterative.c` suite and register it in `tests/runTests.c` (suite table, `RC_` bit) and `tests/Makefile` (`MOD2_SOURCES`). Verify: `runTests -l` lists the suite.
- [x] 7.2 Implement the cases from design D9, looping over every registered CG/GMRES variant:
  - tolerance-mode convergence;
  - exact fixed iteration counts, including across GMRES restarts;
  - zero rhs;
  - GMRES lucky breakdown;
  - rejection of unknown and unsupported names.

  Verify: the `runTests solverTestsIterative` suite passes for CRS and SCS builds.
- [x] 7.3 Manual integration check. Run the default `-t cg`, and `-t gmres` with MPI on 2 ranks, in both stop modes. Confirm that the iteration counts, the summary fields and the profiler tables match the specs. Verify: record the commands and outputs in the change's PR description.
