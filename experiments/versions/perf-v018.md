# perf-v018

## Status

Completed. This round has clear value on the benchmark RTX 3070: raising the high-memory NVIDIA automatic batch size measured `158.991 MH/s` versus `156.215 MH/s` for `perf-v017`, with initialization increasing from 11 seconds to 13 seconds.

## Optimization target

Improve sustained throughput on large-memory NVIDIA devices by increasing the amount of work submitted per inverse/iterate/score cycle while keeping the already tuned local work size.

## Files changed

- `profanity.cpp`
- `experiments/benchmark_summary.csv`
- `experiments/versions/perf-v018.md`

## Changes

1. Raised the large-memory NVIDIA automatic `inverse-multiple` target from `131072` to `155648`.
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

`benchmark.ps1` was attempted first:

```powershell
powershell -ExecutionPolicy Bypass -File .\benchmark.ps1 -Version perf-v018 -Notes "raise NVIDIA large-batch inverse multiple to 155648"
```

The script still fails in this PowerShell environment because `Start-Process` receives duplicate `Path/PATH` environment keys. The equivalent command was run directly and the same fields were recorded manually:

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
- inverse-multiple: 155648
- work-max: 39690240
- initialization: 13 seconds
- speed: 158.991 MH/s
- build: passed

## Comparison with previous version

- `perf-v017`: 156.215 MH/s, initialization 11 seconds, `inverse-multiple=131072`
- `perf-v018`: 158.991 MH/s, initialization 13 seconds, `inverse-multiple=155648`
- Change: +2.776 MH/s, about +1.78%

## Probes during this round

1. A fixed-length TRON checksum SHA fast path built and ran, but measured `153.514 MH/s`, so it was rejected.
2. A word-level checksum chaining variant measured `154.844 MH/s`, below `perf-v017`, so it was rejected.
3. `--work 128 --inverse-multiple 131072` measured `152.036 MH/s`.
4. `--work 512 --inverse-multiple 131072` measured `149.371 MH/s`.
5. `--work 256 --inverse-multiple 114688` measured `147.682 MH/s`.
6. `--work 256 --inverse-multiple 147456` measured `155.601 MH/s`.
7. `--work 256 --inverse-multiple 139264` measured `155.878 MH/s`.
8. `--work 256 --inverse-multiple 155648` measured `160.251 MH/s` during probing and `158.991 MH/s` in the final default run.

## Conclusion

This round is worth keeping. The larger NVIDIA high-memory batch improves measured throughput on the RTX 3070 while preserving the previously tuned local work size and all kernel semantics. The tradeoff is a slightly longer initialization phase and higher GPU memory pressure, so the change remains limited to the existing large-memory NVIDIA bucket.
