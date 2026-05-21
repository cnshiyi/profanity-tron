# perf-v007

## Status

Completed, but not a clear performance win.

## Optimization target

Reduce unnecessary score-kernel comparisons by stopping prefix and suffix matching loops as soon as the requested threshold is reached.

## Files changed

- `kernel_profanity.hpp`
- `experiments/benchmark_summary.csv`
- `experiments/versions/perf-v007.md`

## Changes

1. Added `scorePrefix < prefixCount` to the prefix loop condition.
2. Added `scoreSuffix < suffixCount` to the suffix loop condition.
3. Preserved the existing direct Base58 address indexing from `perf-v006`.

## Device scope

This is cross-device friendly for OpenCL GPUs. It does not change workgroup sizes, memory allocation, device selection, or vendor-specific tuning. It only reduces loop work when the requested prefix or suffix threshold is lower than the full available comparison span.

## Build

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1 -Clean
```

Build passed with Visual Studio C++ tools via `build.ps1`.

## Benchmark command

`benchmark.ps1` was attempted first:

```powershell
powershell -ExecutionPolicy Bypass -File .\benchmark.ps1 -Version perf-v007 -Notes "early exit score loops at requested prefix/suffix thresholds"
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

- work: 512
- inverse-multiple: 49152
- work-max: 12533760
- initialization: 4 seconds
- speed: 76.019 MH/s
- build: passed

## Comparison with previous version

- `perf-v006`: 76.025 MH/s, initialization 4 seconds, `inverse-multiple=49152`
- `perf-v007`: 76.019 MH/s, initialization 4 seconds, `inverse-multiple=49152`
- Change: -0.006 MH/s, effectively flat within run noise

## Conclusion

This round is behaviorally valid and keeps the kernel slightly more direct for low prefix/suffix thresholds, but the measured RTX 3070 result is effectively unchanged and slightly below `perf-v006`. It should not be treated as a clear throughput improvement. A commit was not created for this round because the run did not show clear value and the current sandbox ACL also denies writes to `.git`, preventing commits.
