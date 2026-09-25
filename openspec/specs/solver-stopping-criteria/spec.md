# solver-stopping-criteria Specification

## Purpose

Defines when the iterative solvers stop: either after a fixed number of
iterations, for per-iteration performance measurements, or at a relative
residual tolerance, for time-to-solution measurements.

## Requirements

### Requirement: Fixed-iteration mode
When `eps` is `0` (the default), the CG and GMRES solvers SHALL perform
exactly `itermax` iterations, where an iteration is one application of the
system matrix inside the iteration loop. The solvers SHALL NOT stop early
because of a small residual in this mode. The only exceptions are the
exact-solution guards.

#### Scenario: CG runs exactly itermax iterations
- **WHEN** the user runs `-t cg -i 50` with `eps` left at `0`
- **THEN** CG performs exactly 50 iterations and reports 50 iterations

#### Scenario: GMRES runs exactly itermax iterations across restarts
- **WHEN** the user runs `-t gmres -i 100 -r 30` with `eps` left at `0` on a matrix without breakdown
- **THEN** GMRES performs exactly 100 Arnoldi steps over four restart cycles and reports 100 iterations

### Requirement: Relative tolerance mode
When `eps` is greater than `0`, the CG and GMRES solvers SHALL stop at the
first iteration where the residual norm estimate satisfies
`||r|| / ||b|| <= eps`, or after `itermax` iterations, whichever comes first.
`||b||` is the 2-norm of the right-hand side over all ranks.

#### Scenario: Convergence before itermax
- **WHEN** the user runs `-t cg -e 1e-8 -i 1000` on a symmetric positive definite matrix that converges in fewer than 1000 iterations
- **THEN** CG stops at the first iteration with `||r||/||b|| <= 1e-8` and reports that it converged

#### Scenario: Tolerance not reached
- **WHEN** the user runs `-t gmres -e 1e-14 -i 10`
- **THEN** GMRES stops after 10 iterations and reports that the iteration limit was reached

#### Scenario: Tolerance independent of problem scaling
- **WHEN** the right-hand side is multiplied by a constant factor
- **THEN** the number of iterations to reach a given `eps` is unchanged, up to floating-point effects

### Requirement: Invalid tolerance and iteration limits are rejected
The system SHALL abort with a non-zero exit status before the solve when `eps`
is negative or `itermax` is smaller than `1`.

#### Scenario: Negative tolerance
- **WHEN** the user runs `-t cg -e -1e-6`
- **THEN** the program exits with a non-zero status and an error about the tolerance

### Requirement: Exact-solution guards
The CG and GMRES solvers SHALL stop before `itermax`, in either stopping mode,
when continuing would be numerically undefined because the exact solution has
been reached:
- The right-hand side is zero, `||b|| = 0`. The solver SHALL return `x = 0`
  without iterating.
- CG finds an exactly zero residual.
- GMRES detects a lucky breakdown, meaning the Krylov space is invariant.
The solver SHALL NOT produce NaN or Inf values in these cases. The stop reason
SHALL be reported.

#### Scenario: Zero right-hand side
- **WHEN** a solver is run on a system with `b = 0`
- **THEN** it performs 0 iterations, returns `x = 0`, and reports the stop reason as zero right-hand side

#### Scenario: GMRES breakdown in fixed-iteration mode
- **WHEN** GMRES is run with `eps = 0` and a lucky breakdown occurs before `itermax`
- **THEN** GMRES stops at the breakdown, reports the stop reason as breakdown with the actual iteration count, and the final true residual is at the level of rounding error

#### Scenario: CG exact solution in fixed-iteration mode
- **WHEN** CG is run with `eps = 0` and the residual becomes exactly zero before `itermax`
- **THEN** CG stops without dividing by zero and reports the stop reason as exact solution
