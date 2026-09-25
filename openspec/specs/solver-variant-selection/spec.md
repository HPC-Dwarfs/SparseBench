# solver-variant-selection Specification

## Purpose

Lets users pick, at runtime, which algorithm the CG and GMRES benchmarks run
and which orthogonalization scheme GMRES uses, so variants can be compared
with a single binary.

## Requirements

### Requirement: Solver algorithm selectable at runtime
The system SHALL accept a solver variant name through the `-a <name>` command
line option and through the `solver_variant` parameter file key. The variant
SHALL apply to the benchmark type chosen with `-t`. When no variant is given,
the system SHALL use `standard`. When the key and the option are both given,
the value processed last in command line order SHALL win, consistent with how
other options interact with `-f`.

#### Scenario: Default variant
- **WHEN** the user runs `-t cg` without `-a` and without `solver_variant`
- **THEN** the CG benchmark runs the `standard` variant

#### Scenario: Variant from the command line
- **WHEN** the user runs `-t gmres -a standard`
- **THEN** the GMRES benchmark runs the `standard` variant

#### Scenario: Variant from the parameter file
- **WHEN** the parameter file passed with `-f` contains `solver_variant standard` and the user runs `-t cg -f <file>`
- **THEN** the CG benchmark runs the `standard` variant

### Requirement: GMRES orthogonalization selectable at runtime
The system SHALL accept a GMRES orthogonalization scheme through the
`-o <name>` command line option and through the `gmres_ortho` parameter file
key. When none is given, the system SHALL use `mgs`. The setting SHALL have
no effect on benchmark types other than GMRES.

#### Scenario: Default orthogonalization
- **WHEN** the user runs `-t gmres` without `-o`
- **THEN** GMRES uses modified Gram-Schmidt (`mgs`)

#### Scenario: Orthogonalization ignored for CG
- **WHEN** the user runs `-t cg -o mgs`
- **THEN** the CG benchmark runs normally and the orthogonalization setting has no effect

### Requirement: Invalid variant names are rejected before the solve
The system SHALL abort with a non-zero exit status before matrix setup or any
solver iteration when the selected variant or orthogonalization name is not
registered for the chosen benchmark type. The error message SHALL list the
valid names for that benchmark type.

#### Scenario: Unknown solver variant
- **WHEN** the user runs `-t cg -a doesnotexist`
- **THEN** the program exits with a non-zero status and the message lists the valid CG variants

#### Scenario: Unknown orthogonalization
- **WHEN** the user runs `-t gmres -o doesnotexist`
- **THEN** the program exits with a non-zero status and the message lists the valid orthogonalization schemes

### Requirement: Variants unsupported by the build are rejected
The system SHALL abort with a non-zero exit status before any solver iteration
when the selected variant does not support the current build configuration
(value type, matrix format, or GPU backend). The message SHALL name the variant
and the unsupported configuration.

#### Scenario: GMRES in a complex build
- **WHEN** the program is built with complex arithmetic and the user runs `-t gmres`
- **THEN** the program exits with a non-zero status and states that the selected GMRES variant does not support complex arithmetic

### Requirement: Selected variant is visible in the output
The system SHALL print the selected solver variant and, for GMRES, the
orthogonalization scheme together with the other parameters before the
solve starts.

#### Scenario: Parameter echo
- **WHEN** the user runs `-t gmres -a standard -o mgs`
- **THEN** the parameter output contains the variant `standard` and the orthogonalization `mgs`
