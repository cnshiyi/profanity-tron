# perf-v020

## Status

Completed. This round has a small but repeatable scheduling gain on the benchmark RTX 3070: lowering the high-memory NVIDIA local work size from `256` to `128` measured `171.064 MH/s` versus `169.948 MH/s` recorded for `perf-v019`, with initialization unchanged at 17 seconds.

## Optimization target

Retune the local work size for the current high-memory NVIDIA scheduling profile after `perf-v019` raised the default batch size to `inverse-multiple=204800`.

## Files changed

- `profanity.cpp`
- `experiments/benchmark_summary.csv`
- `experiments/versions/perf-v020.md`

## Changes

1. Changed the high-memory NVIDIA automatic local work size from `256` to `128`.
2. Preserved the high-memory NVIDIA `inverse-multiple` target at `204800`.
3. Preserved the lower-memory NVIDIA fallback at `512` local work size and `32768` inverse multiple.
4. Left AMD, Intel, OpenCL kernel logic, memory allocation sizes, matching semantics, and result scoring unchanged.

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

- work: 128
- inverse-multiple: 204800
- work-max: 52224000
- initialization: 17 seconds
- speed: 171.064 MH/s
- build: passed

## Comparison with previous version

- `perf-v019`: 169.948 MH/s, initialization 17 seconds, `work=256`, `inverse-multiple=204800`
- `perf-v020`: 171.064 MH/s, initialization 17 seconds, `work=128`, `inverse-multiple=204800`
- Change: +1.116 MH/s, about +0.66%

## Probes during this round

1. Current default `--work 256 --inverse-multiple 204800` measured `168.924 MH/s` with 16 seconds initialization.
2. `--work 128 --inverse-multiple 204800` measured `169.975 MH/s` with 17 seconds initialization.
3. `--work 512 --inverse-multiple 204800` measured `165.732 MH/s` with 17 seconds initialization and was rejected.
4. A repeat `--work 128 --inverse-multiple 204800` probe measured `171.062 MH/s`.
5. The final default run after changing auto-tune measured `171.064 MH/s`, confirming `work=128` is selected automatically.

## Conclusion

This round is worth keeping. With the larger high-memory NVIDIA batch selected in `perf-v019`, a smaller local work size gives slightly better sustained throughput on the RTX 3070 without increasing initialization time or changing kernel semantics. The gain is modest, so future runs should keep checking this setting if later batch or kernel changes shift occupancy behavior again.
