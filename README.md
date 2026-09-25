# SparseBench

A hybrid MPI+OpenMP (and CUDA/HIP) benchmark collection for iterative sparse
linear solvers and sparse matrix-vector kernels.

## Features

- **Benchmarks** (`-t`): `cg` (Conjugate Gradient), `spmv`, `spmmv` (block
  SpMV), `gmres` (restarted GMRES), `cheb` (Chebyshev Filter Diagonalization)
- **Matrix formats** (`MTX_FMT`): CRS, SCS (SELL-C-σ), CCRS
- **Backends**: CPU (GCC, Clang, ICX) with optional MPI/OpenMP; GPU via NVCC or HIP
- **Matrix sources**: generated 27-/7-point stencils, Matrix Market (`.mtx`),
  binary (`.bmx`, MPI builds), and in-process [ScaMaC](https://bitbucket.org/essex/matrixcollection)
  generators (bundled in `ext/scamac`)
- **MPI**: localized halo exchange with a single `MPI_Neighbor_alltoallv` per
  iteration, optionally overlapped with the local SpMV (see
  [MPI-Algorithm.md](MPI-Algorithm.md))
- **Instrumentation**: LIKWID markers, NVTX ranges, built-in section timers

## Build

Requires a C compiler and GNU make (≥ 4.0 for `.clangd` generation). On first
run `make` copies `mk/config-default.mk` to `config.mk`; edit it and run `make`
again. Key options:

| Option                  | Values                        | Default |
| ----------------------- | ----------------------------- | ------- |
| `TOOLCHAIN`             | GCC, CLANG, ICX, NVCC, HIP    | CLANG   |
| `MTX_FMT`               | CRS, SCS, CCRS                | CRS     |
| `ENABLE_MPI`            | true, false                   | false   |
| `ENABLE_OPENMP`         | true, false                   | false   |
| `ENABLE_OVERLAP`        | CG halo/SpMV overlap (CPU CRS + MPI) | true |
| `FLOAT_TYPE`            | SP, DP                        | DP      |
| `UINT_TYPE`             | U, ULL (64-bit indices)       | U       |
| `USE_COMPLEX_ELEMENTS`  | true, false                   | false   |
| `ENABLE_LIKWID` / `ENABLE_NVTX` / `ENABLE_SECTIMER` | true, false | false / false / true |

GPU architectures are set via `CUDA_ARCH` / `HIP_ARCH`, SELL-C-σ parameters via
`SELL_CHUNK_VALUE` / `SELL_SIGMA_VALUE`.

The binary is `sparseBench-<MTX_FMT>-<TOOLCHAIN>`; objects go to
`build/<MTX_FMT>-<TOOLCHAIN>`. Other targets: `make clean`, `make distclean`,
`make asm`, `make format`, `make info`, `make Q=` (verbose).

## Usage

```sh
./sparseBench-CRS-GCC [options]      # -h for help
```

| Option | Description                                                                 |
| ------ | --------------------------------------------------------------------------- |
| `-f`   | Parameter file (e.g. `hpcg.par`, `cheb.par`; ChebFD settings are file-only) |
| `-m`   | Matrix: `generate` (27pt), `generate7P`, `<file>.mtx/.bmx`, `scamac:<args>` |
| `-c`   | Convert a Matrix Market file to `.bmx` (MPI builds)                         |
| `-t`   | Benchmark: `cg`, `spmv`, `spmmv`, `gmres`, `cheb` (default `cg`)            |
| `-x/-y/-z` | Generated grid size (default 100)                                       |
| `-i`   | Iterations; the iteration limit for `cg`/`gmres` (default 150)              |
| `-e`   | Relative tolerance `‖r‖/‖b‖`; `0` = fixed iterations (default 0.0)          |
| `-a`   | Solver variant for `cg`/`gmres` (default `standard`)                        |
| `-o`   | GMRES orthogonalization scheme (default `mgs`)                              |
| `-r`   | GMRES restart dimension (default 30)                                        |
| `-w`   | Block vector width for SpMMV                                                |
| `-d`   | GPU device index (default 0)                                                |
| `-v`   | Verbose output                                                              |
| `-k/-s`| SELL-C-σ chunk size / sigma (SCS builds)                                    |

The parameter file keys `solver_variant` and `gmres_ortho` correspond to `-a`
and `-o`. As with all options, whatever comes last on the command line wins
(`-f` is processed in order). Unknown or unsupported names abort before the
matrix is set up and list the valid names; this build registers `standard` for
CG and GMRES and `mgs` (modified Gram-Schmidt) for GMRES. GMRES is not
available in complex builds.

### Stopping modes (`cg`, `gmres`)

- **Fixed iterations** (`-e 0`, the default): exactly `-i` iterations, for
  per-iteration performance measurements. One iteration is one SpMV (one
  Arnoldi step for GMRES, counted across restarts).
- **Relative tolerance** (`-e <eps>` with `eps > 0`): stops at the first
  iteration with `‖r‖/‖b‖ <= eps`, or after `-i` iterations, for
  time-to-solution measurements.

In both modes the solvers stop early, without producing NaNs, when the exact
solution is reached: a zero right-hand side (`zero rhs`, `x = 0`), an exactly
zero CG residual (`exact solution`) or a GMRES lucky breakdown (`breakdown`).

Every run ends with a summary in a fixed format, followed by the profiler
table:

```
Solver:                CG
Variant:               standard
Stop mode:             relative tolerance 1e-08 (itermax 1000)
Stop reason:           converged
Iterations:            35
Solve time:            9.884000e-03 s
Time/iter:             2.824000e-04 s
Rel. residual (est.):  9.989022e-09
Rel. residual (true):  9.989022e-09
Max error:             2.293839e-08
```

`Ortho` is added for GMRES. `Max error` (`max|x - xexact|`) is only shown for
the generated matrices, which have a known exact solution. The true residual
and the error are computed after the solve timer stops.

> **Breaking change:** `eps` used to be an absolute tolerance on `‖r‖`. It is
> now relative to `‖b‖`; divide old values by `‖b‖` (for the generated
> matrices `‖b‖ = O(sqrt(n))`). CG now performs exactly `-i` iterations; it
> used to perform one fewer than requested.

Examples:

```sh
./sparseBench-CRS-GCC -x 200 -y 200 -z 200               # CG, fixed 150 iterations
./sparseBench-CRS-GCC -t cg -e 1e-8 -i 1000              # CG to ||r||/||b|| <= 1e-8
./sparseBench-CRS-GCC -t gmres -i 100 -r 30              # GMRES(30), fixed 100 steps
./sparseBench-CRS-GCC -t gmres -m matrix.mtx -e 1e-6 -a standard -o mgs
./sparseBench-CRS-GCC -t cheb -f cheb.par
./sparseBench-CRS-GCC -t cheb -m scamac:Anderson,Lx=100,Ly=100,Lz=100,ranpot=2.5
mpirun -np 4 ./sparseBench-CRS-GCC -m matrix.mtx
```

Complex ScaMaC matrices need `USE_COMPLEX_ELEMENTS=true`; matrices beyond the
32-bit index range need `UINT_TYPE=ULL`.

For OpenMP runs, pin threads with `likwid-pin -C 0-7 ...` or
`OMP_PLACES=cores OMP_PROC_BIND=close`.

## Testing

```sh
cd tests
make run          # all suites
make list         # list suites
make run-<suite>  # single suite
```

## License

Copyright © NHR@FAU, University Erlangen-Nuremberg. MIT License — see
[LICENSE](LICENSE).
