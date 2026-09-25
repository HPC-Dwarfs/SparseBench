# Proposal

## Why

SparseBench benchmarks best-practice iterative solvers, which means the CG and
GMRES solvers will gain several algorithmic variants (fused kernels, CGS2 and
low-synchronization orthogonalization, pipelined CG, preconditioning). None of
those can be compared fairly today:

- Each benchmark type has exactly one hard-wired algorithm.
- The stopping test is absolute (`||r|| > eps`), so it cannot be compared
  across problem sizes.
- The only reported result is an iteration count plus wall time.
- There are no end-to-end regression tests for CG or GMRES.

This change builds the harness that every later variant plugs into and is
measured against.

## What Changes

- **Runtime variant selection** for the iterative solvers:
  - `-a <name>` / `solver_variant` selects the solver algorithm (CG and GMRES).
  - `-o <name>` / `gmres_ortho` selects the GMRES orthogonalization scheme.
  - Each solver keeps a variant registry. An entry records the variant's
    entry point, which build configurations it supports, and its profiler
    region sequence.
  - This change registers only the existing algorithms (`cg:standard`,
    `gmres:standard` with `mgs`).
  - Unknown names or unsupported combinations abort before the solve and
    list the valid names.
- **Two explicit stopping modes**:
  - Fixed-iteration mode (`eps = 0`, the default) runs exactly `itermax`
    iterations and never tests convergence.
  - Tolerance mode (`eps > 0`) stops at `||r||/||b|| <= eps` or at `itermax`,
    whichever comes first.
- **BREAKING**: `eps` becomes a *relative* tolerance, measured against
  `||b||`. Existing parameter files or scripts with `eps > 0` change meaning.
- The iteration count becomes exact: the CG loop currently does
  `itermax - 1` iterations and reports `itermax`.
- Exact-solution guards:
  - A zero right-hand side stops immediately.
  - A zero residual in CG stops instead of computing `0/0`.
  - A GMRES lucky breakdown stops early.
  - In each case the solver reports why it stopped, including in
    fixed-iteration mode.
- **Unified solver result report** for CG and GMRES:
  - variant
  - stopping mode and reason
  - iterations
  - solve time and time per iteration
  - estimated and true relative residual
  - error versus the exact solution, where one is known
- **End-to-end regression tests** for CG and GMRES in `tests/`. They cover
  convergence in tolerance mode, iteration counts in fixed mode, and the
  exact-solution guards.

Follow-up changes that build on this harness (out of scope here):

1. `cg-fused-kernels`: fused SpMV+dot and a two-loop CG update.
2. `gmres-block-basis-cgs2`: contiguous Krylov basis, CGS2, and a single-pass
   solution update.
3. `cg-pipelined`: Chronopoulos-Gear and Ghysels-Vanroose pipelined CG.
4. `gmres-low-sync`: one-reduce DCGS2, and optionally complex GMRES.
5. `solver-preconditioning`: Jacobi and Chebyshev polynomial preconditioners
   for both solvers, behind a new `-p` axis.

## Capabilities

### New Capabilities

- `solver-variant-selection`: runtime selection of the solver algorithm and
  the GMRES orthogonalization scheme through the CLI and the parameter file,
  including validation against the build configuration.
- `solver-stopping-criteria`: fixed-iteration and relative-tolerance stopping
  modes, exact iteration counts, and exact-solution guards.
- `solver-result-reporting`: the common result report printed by every
  iterative solver run.

### Modified Capabilities

None. There are no existing specs.

## Impact

- **Code**:
  - `src/cli.c`, `src/cli.h`: new `-a` and `-o` options and updated help text.
  - `src/parameter.c`, `src/parameter.h`: new keys and fields; `eps` is
    documented as relative.
  - `src/CGSolver.c`, `src/GMRESSolver.c`: stopping logic, guards, and result
    struct.
  - `src/solver.h`, `src/solverCommon.c`: registry, shared stopping and
    reporting helpers.
  - `src/main.c`: dispatch through the registry, and the profiler sequence
    taken from the variant.
- **Tests**: new CG/GMRES solver suite in `tests/solver/`, registered in
  `tests/runTests.c` and `tests/Makefile`.
- **Docs**: README usage section and the help text.
- **Compatibility**:
  - Default runs (`eps = 0`) keep their behaviour. CG now does exactly
    `itermax` iterations instead of `itermax - 1`.
  - Runs with `eps > 0` now use a relative criterion.
- **Dependencies**: none new.
