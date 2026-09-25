# solver-result-reporting Specification

## Purpose

Gives every CG and GMRES run the same result summary, so variants can be
compared on both time-to-solution and time per iteration.

## Requirements

### Requirement: Common result summary
After every CG or GMRES solve, the master rank SHALL print a result summary
with the same field names and order for both solvers and all variants. It
SHALL contain:
- solver and variant (plus the orthogonalization scheme for GMRES)
- stopping mode (`fixed` or `tolerance`, with the tolerance value)
- stop reason (`converged`, `itermax`, `breakdown`, `exact solution`, or `zero rhs`)
- number of iterations performed
- solve wall time, and wall time per iteration
- final relative residual estimate from the recurrence
- final true relative residual `||b - Ax|| / ||b||`

#### Scenario: CG summary in tolerance mode
- **WHEN** CG converges in tolerance mode
- **THEN** the summary shows mode `tolerance` with the value of `eps`, reason `converged`, the iteration count, the solve time, the time per iteration, and both relative residuals

#### Scenario: GMRES summary in fixed mode
- **WHEN** GMRES finishes in fixed-iteration mode
- **THEN** the summary shows mode `fixed`, reason `itermax`, the orthogonalization scheme, and the same fields as for CG

### Requirement: True residual is computed outside the timed region
The true residual `||b - Ax||` reported in the summary SHALL be computed after
the solve timer has stopped, so it does not affect solve time or time per
iteration.

#### Scenario: Timing excludes verification
- **WHEN** a solve finishes
- **THEN** the reported solve time covers only the iteration loop, and the true residual computation is not included

### Requirement: Error against the exact solution
When the matrix is generated with a known exact solution (`generate`,
`generate7P`), the summary SHALL also contain the maximum absolute difference
between the computed and the exact solution. For other matrix sources this
field SHALL be omitted.

#### Scenario: Generated matrix
- **WHEN** CG is run on the `generate` matrix
- **THEN** the summary contains the maximum error versus the exact solution

#### Scenario: Matrix Market input
- **WHEN** CG is run on a Matrix Market file
- **THEN** the summary contains no exact-solution error field

### Requirement: Existing profiler output is preserved
The per-kernel profiler table SHALL continue to be printed after the summary.
The regions it lists SHALL be the ones the selected variant actually executes.

#### Scenario: Standard CG profiler regions
- **WHEN** the `standard` CG variant runs in a build without communication overlap
- **THEN** the profiler table lists the DDOT, WAXPBY and SPMVM regions, as it does today
