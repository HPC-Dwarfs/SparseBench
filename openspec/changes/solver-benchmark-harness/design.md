# Design

## Context

Motivation is in proposal.md. Behaviour is specified in
`specs/solver-variant-selection`, `specs/solver-stopping-criteria` and
`specs/solver-result-reporting`. This document covers how the code gets
there.

Current state:

- **Dispatch.** `main.c` switches on the global `BenchType`, calls
  `solveCG` or `solveGMRES` directly, and picks the profiler region sequence
  itself (`seqCgPlain`/`seqCgOverlap`). The sequence depends on the
  compile-time `USE_OVERLAP_SPMVM`.
- **CLI and parameters.** CLI parsing (`cli.c`, getopt string
  `BASE_ARGS_COMMON`) and parameter file parsing (`parameter.c`,
  `PARSE_KEY`) both write into `Parameter`. `-f` is processed in command line
  order, so later options override earlier ones.
- **CG loop.** It runs `for (k = 1; k < itermax && normr > eps; k++)`: that is
  `itermax - 1` iterations, the count reported is `itermax`, and the test uses
  an absolute tolerance. With `eps = 0` it always runs to the limit. There is
  no guard against `rtrans == 0`, which gives `beta = 0/0`.
- **GMRES.** It counts Arnoldi steps exactly. It uses absolute `eps` both on
  the Givens estimate and on the recomputed residual at each restart. It
  detects lucky breakdown but then starts another restart cycle, whose
  normalization divides by a residual that is (almost) zero. It already
  prints a final true residual.
- **Reporting.** Each solver prints its own ad-hoc lines. Only
  `solverCheckResidual` prints `max|x - xexact|`.
- **Tests.** `tests/runTests.c` has a suite table, but there is no CG or
  GMRES suite. Tests are single-rank only.
- **GPU builds** are single-rank (`main.c:99`). `DDOTFUNC` maps to
  `gpu_ddot_sync`.

## Goals / Non-Goals

**Goals:**

- One place, a registry, that knows which variants exist, what each supports,
  and which profiler regions it reports. Later changes should add a variant
  by adding a function and one table row.
- Keep the orthogonalization axis separate from the GMRES algorithm, so
  change `gmres-block-basis-cgs2` only adds an orthogonalization routine.
- Shared helpers for stopping and result reporting, so every current and
  future variant behaves the same way.

**Non-Goals:**

- No new algorithms or kernels. Only the existing CG and MGS-GMRES are
  registered.
- No preconditioner axis yet. `-p` stays unused until
  `solver-preconditioning`.
- No machine-readable output (JSON/CSV). The summary is plain text with
  stable field names, which keeps that option open.
- No MPI or GPU test coverage. The test runner stays single-rank.

## Decisions

### D1: Static registry table per solver type

```c
typedef enum { STOP_CONVERGED, STOP_ITERMAX, STOP_BREAKDOWN,
               STOP_EXACT, STOP_ZERO_RHS } StopReason;

typedef struct {
  int iterations;
  StopReason reason;
  double solveTime;          /* iteration loop only */
  CG_FLOAT relResEstimate;   /* recurrence / Givens estimate, relative */
  CG_FLOAT relResTrue;       /* ||b - Ax|| / ||b||, computed after timer */
  CG_FLOAT errMax;           /* max|x - xexact|, < 0 if unknown */
} SolverResult;

typedef int (*SolveFn)(CommType *, Parameter *, Matrix *, SolverResult *);

typedef struct {
  const char *name;
  SolveFn solve;
  unsigned supports;         /* bitmask: SUPPORT_COMPLEX, SUPPORT_GPU, ... */
  const int *profSeq;        /* regions to print */
  int numProfSeq;
} SolverVariant;
```

- One `static const SolverVariant` table each for CG and GMRES, plus a
  table of orthogonalization schemes for GMRES. They live in a new
  `src/solverRegistry.c`.
- Profiler sequences stay compile-time selected (`#ifdef USE_OVERLAP_SPMVM`)
  inside the table initializer. This keeps the existing guarantee in
  `solver.h` that `main.c` and the solver cannot disagree.
- The `supports` mask is computed from the build macros when the entry is
  checked, not at runtime.

*Alternatives considered:*

- **Function pointer in `Parameter`.** It mixes configuration with dispatch
  and cannot carry metadata.
- **Compile-time variant macros.** Rejected by the user in favour of runtime
  selection.
- **Flat names such as `gmres-cgs2`.** The number of names grows with the
  product of the axes.

### D2: Names stored as strings, resolved once after parsing

- `Parameter` gains `char *solverVariant` and `char *gmresOrtho`, owned
  the same way as `filename` and freed in `freeParameter`.
- The defaults are `"standard"` and `"mgs"`.
- `main.c` resolves both names against the registry immediately after
  `parseArguments` and before matrix setup. It aborts through `commAbort` if
  a name is unknown or unsupported by the build, and the message lists the
  valid names.

Resolving late, rather than inside getopt, matters because `-t` can come after
`-a`, and a parameter file can set the variant before `-t` is seen.

The same place validates `eps >= 0` and `itermax >= 1`.

The complex-build GMRES rejection moves from the start of `solveGMRES` into
this check, through the `supports` mask.

*Alternative considered:* enums in `Parameter`. That would duplicate the
list of names outside the registry.

### D3: One `solver_variant` key, not one key per solver

Each run executes exactly one benchmark type, so a single `-a` /
`solver_variant` is enough. Names are looked up in the table for the selected
`-t`.

Trade-off: a parameter file shared between `-t cg` and `-t gmres` runs uses
the same variant name for both. Because both tables have `standard`, the
default still works for both.

`gmres_ortho` is a separate key because it is a separate axis (D4).

### D4: Orthogonalization as a strategy inside GMRES

- The MGS loop in `GMRESSolver.c` (inner `for i <= j`, then the norm)
  becomes a function behind `OrthoFn`.
- `OrthoFn` takes the basis, the step `j`, and the column of H. It returns
  `h_{j+1,j}` and the column norm used by the breakdown test.
- `solveGMRES` receives the resolved `OrthoFn` through a small context
  set by `main.c`, rather than looking up the name itself.

This is the seam that change `gmres-block-basis-cgs2` plugs CGS2 into. The
basis layout (`V_ELE **`) is deliberately not changed here. That change
moves the basis to a contiguous `DMatrix`, which will change the `OrthoFn`
signature; that is accepted.

### D5: Shared stopping helper with a precomputed threshold

`solverStopInit(comm, b, nrow, eps)` computes `||b||` with `DDOTFUNC` (one
reduction, before the timer starts) and returns:

- `absTol = eps * ||b||` in tolerance mode;
- `absTol = -1` in fixed mode, which the residual can never undercut.

Both loops compare `normr <= absTol`, so the relative test costs nothing per
iteration.

If `||b|| == 0`, the solver sets `x = 0` and returns `STOP_ZERO_RHS` without
iterating.

### D6: CG loop counts exact iterations and guards the exact solution

- The CG loop is restructured so that `k` counts completed iterations,
  0..itermax, where one iteration is one SpMV.
- The loop stops when `k == itermax`, when `normr <= absTol`, or when
  `rtrans == 0`. The last case returns `STOP_EXACT` before `beta` is formed.
- A zero `p·Ap` with non-zero `rtrans` means the matrix is not SPD. That
  case aborts with a clear message instead of producing NaN.

The algorithm itself (operation order, same kernels) stays unchanged, so
per-iteration performance is unchanged.

### D7: GMRES stops on breakdown, relative tolerance at both check points

- A lucky breakdown finishes the current cycle: back substitution, then the
  solution update. After that the solver returns `STOP_BREAKDOWN` instead of
  restarting.
- Both the inner Givens estimate check and the post-restart true-residual
  check compare against `absTol`.

### D8: Common summary printed by `solverPrintResult`

- Solvers fill a `SolverResult` and no longer print their own final lines.
  Iteration progress lines stay.
- After the timer stops, the solver computes `relResTrue`. CG gains one
  SpMV here; GMRES already did this. The solver also computes `errMax`,
  reusing the `solverCheckResidual` logic, now returning the value instead
  of printing it.
- `main.c` prints the summary with `solverPrintResult`, then prints the
  profiler table from the variant's `profSeq`.
- Field labels are fixed strings in a fixed order: `Solver`, `Variant`,
  `Ortho`, `Stop mode`, `Stop reason`, `Iterations`, `Solve time`,
  `Time/iter`, `Rel. residual (est.)`, `Rel. residual (true)`,
  `Max error`. Scripts can grep them.

### D9: End-to-end solver tests call the registry directly

- A new suite `solverTestsIterative` in `tests/solver/` builds a small
  generated 7-point stencil through the same path `main.c` uses.
- It runs each registered CG and GMRES variant through its `SolveFn`, and
  checks the returned `SolverResult`:
  - Tolerance mode with `eps = 1e-8` converges, with
    `relResTrue <= 10 * eps` and `errMax` small.
  - Fixed mode with `itermax = 17` reports exactly 17 iterations. For GMRES,
    `restart = 5` crosses restart boundaries.
  - `b = 0` gives `STOP_ZERO_RHS`, 0 iterations, and `x = 0`.
  - GMRES with `restart >= n` on a tiny system breaks down and returns
    `STOP_BREAKDOWN` with a residual at rounding level.
  - Unknown and unsupported names are rejected by the resolver function.
- Because the suite loops over the registry, every later variant gets these
  checks automatically.

## Risks / Trade-offs

- **[`eps` semantics change silently]** → The parameter echo prints
  `relative tolerance`. The README and the help text document the change.
  The summary always shows the stop mode and tolerance.
- **[CG does one more iteration than before at the same `-i`]** → A
  deliberate fix. It is stated in the proposal and in the README. Timings per
  iteration are unaffected.
- **[Extra SpMV for the CG true residual]** → It runs after the timer, so it
  does not affect reported performance, only total runtime (by 1/itermax).
- **[The `OrthoFn` signature will change with the block basis]** → Accepted.
  It is internal, with one implementation today.
- **[Fixed-mode GMRES reports fewer iterations after breakdown]** → This is
  correct behaviour, and the reason is reported. Benchmarks that need exactly
  N iterations should use matrices without early breakdown, which covers all
  realistic sizes.
- **[Test runner is single-rank]** → The MPI and GPU paths of the stopping
  logic are only covered by manual runs. The helpers use the same
  `DDOTFUNC`/`commReduction` as the existing code, so there is no new
  communication pattern.

## Migration Plan

- There are no stored artifacts to migrate.
- Users with `eps > 0` in parameter files must rescale it to a relative value.
  The new value is the old absolute value divided by `||b||`. For the default
  generated problems `||b|| = O(sqrt(n))`.
- Rollback is reverting the change, since there are no data or format
  dependencies.
