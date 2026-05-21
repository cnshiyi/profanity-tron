# perf-v019

## Status

Completed. This round has clear value on the benchmark RTX 3070: raising the high-memory NVIDIA automatic batch size measured `169.948 MH/s` versus `158.991 MH/s` recorded for `perf-v018`, with initialization increasing from 13 seconds to 17 seconds.

## Optimization target

Improve sustained throughput on large-memory NVIDIA devices by increasing the amount of work submitted per inverse/iterate/score cycle while preserving the already tuned local work size.

## Files changed

- `profanity.cpp`
- `experiments/benchmark_summary.csv`
- `experiments/versions/perf-v019.md`

## Changes

1. Raised the large-memory NVIDIA automatic `inverse-multiple` target from `155648` to `204800`.
2. Preserved the large-memory NVIDIA local work size of `256`.
3. Preserved the lower-memory NVIDIA fallback at `32768`.
4. Left AMD, Intel, OpenCL kernel logic, matching semantics, and result scoring unchanged.

## Device scope

This is intended for NVIDIA GPUs in the existing large-batch memory bucket, currently devices reporting at least roughly 7.5 GiB of OpenCL global memory. Smaller NVIDIA devices and non-NVIDIA devices keep their previous tuning.

## Build

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1 -Clean
```

Build passed with Visual Studio C++ tools via `build.ps1`.

## Benchmark command

The equivalent direct benchmark command was run after the build:

```powershell
.\profanity.x64.exe --matching TUqEg3dzVEJNQSVW2HY98z5X8SBdhmao8D --prefix-count 1 --suffix-count 1 --quit-count 1
```

## Test environment

- GPU: NVIDIA GeForce RTX 3070
- OpenCL vendor: NVIDIA
- OpenCL reported memory: 8589410304 bytes
- Compute units: 46
- OS/build: Windows, Visual Studio C++ build tools via `build.ps1`

## Benchmark result

- work: 256
- inverse-multiple: 204800
- work-max: 52224000
- initialization: 17 seconds
- speed: 169.948 MH/s
- build: passed

## Comparison with previous version

- `perf-v018`: 158.991 MH/s, initialization 13 seconds, `inverse-multiple=155648`
- `perf-v019`: 169.948 MH/s, initialization 17 seconds, `inverse-multiple=204800`
- Change: +10.957 MH/s, about +6.89%

## Probes during this round

1. Current default `--work 256 --inverse-multiple 155648` measured `160.256 MH/s` with 13 seconds initialization.
2. `--work 256 --inverse-multiple 172032` measured `163.951 MH/s` with 14 seconds initialization.
3. `--work 256 --inverse-multiple 188416` measured `167.166 MH/s` with 16 seconds initialization.
4. `--work 256 --inverse-multiple 204800` measured `169.970 MH/s` during probing and `169.948 MH/s` in the final default run.
5. `--work 256 --inverse-multiple 221184` measured `150.712 MH/s`, so the larger batch was rejected.

## Conclusion

This round is worth keeping. `204800` is the best measured high-memory NVIDIA batch size in this local probe set and gives a clear throughput improvement over `perf-v018`. The tradeoff is a longer initialization phase and higher GPU memory pressure, so the change remains limited to the existing large-memory NVIDIA bucket.
