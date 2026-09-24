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
| `-i`   | Iterations (default 150)                                                    |
| `-e`   | Convergence epsilon (default 0.0)                                           |
| `-r`   | GMRES restart dimension (default 30)                                        |
| `-w`   | Block vector width for SpMMV                                                |
| `-d`   | GPU device index (default 0)                                                |
| `-v`   | Verbose output                                                              |
| `-k/-s`| SELL-C-σ chunk size / sigma (SCS builds)                                    |

Examples:

```sh
./sparseBench-CRS-GCC -x 200 -y 200 -z 200
./sparseBench-CRS-GCC -t gmres -m matrix.mtx -e 1e-6
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
