# perf-v016

## Status

Completed. This round has clear value on the benchmark RTX 3070: lowering the high-memory NVIDIA local work size from `512` to `256` measured `153.404 MH/s` versus `148.076 MH/s` for `perf-v015`, with initialization unchanged at 11 seconds.

## Optimization target

Improve sustained throughput for the large-batch NVIDIA tuning bucket by selecting a local work size that better matches the current `inverse-multiple=131072` scheduling profile.

## Files changed

- `profanity.cpp`
- `experiments/benchmark_summary.csv`
- `experiments/versions/perf-v016.md`

## Changes

1. Changed the high-memory NVIDIA automatic `work` target from `512` to `256`.
2. Kept the high-memory NVIDIA `inverse-multiple` target at `131072`.
3. Preserved the lower-memory NVIDIA path at `work=512` when it has at least 6 GiB but does not enter the large-batch bucket.
4. Left AMD, Intel, OpenCL kernels, memory allocation sizes, matching semantics, and result scoring unchanged.

## Device scope

This is intended for NVIDIA GPUs in the existing large-batch memory bucket, currently devices reporting at least roughly 7.5 GiB of OpenCL global memory. Smaller NVIDIA devices and non-NVIDIA devices keep their previous automatic work-size behavior.

## Build

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1 -Clean
```

Build passed with Visual Studio C++ tools via `build.ps1`.

## Benchmark command

`benchmark.ps1` was attempted first:

```powershell
powershell -ExecutionPolicy Bypass -File .\benchmark.ps1 -Version perf-v016 -Notes "lower large-memory NVIDIA local work size to 256"
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
- inverse-multiple: 131072
- work-max: 33423360
- initialization: 11 seconds
- speed: 153.404 MH/s
- build: passed

## Comparison with previous version

- `perf-v015`: 148.076 MH/s, initialization 11 seconds, `work=512`, `inverse-multiple=131072`
- `perf-v016`: 153.404 MH/s, initialization 11 seconds, `work=256`, `inverse-multiple=131072`
- Change: +5.328 MH/s, about +3.60%

## Probes during this round

1. `--inverse-multiple 196608` with default `work=512` measured only `42.788 MH/s` with 16 seconds initialization, so increasing the 8 GiB NVIDIA batch further was rejected.
2. `--work 256 --inverse-multiple 131072` measured `154.799 MH/s` with 11 seconds initialization, showing a clear positive local-work-size candidate.
3. A same-session default `work=512 --inverse-multiple 131072` control measured `148.074 MH/s`, matching `perf-v015`.
4. `--work 128 --inverse-multiple 131072` measured `153.404 MH/s`, better than `512` but below the first `256` probe.

## Conclusion

This round is worth keeping. Under the current large-batch NVIDIA profile, `work=256` improves measured throughput on the RTX 3070 without increasing initialization time or changing kernel behavior. The change remains scoped to the existing high-memory NVIDIA tuning bucket.
