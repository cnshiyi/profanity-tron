# perf-v008

## Status

Completed, with a very small positive result that should be treated as near run-to-run noise.

## Optimization target

Reduce repeated index arithmetic inside `profanity_score_matching` without changing matching semantics.

## Files changed

- `kernel_profanity.hpp`
- `experiments/benchmark_summary.csv`
- `experiments/versions/perf-v008.md`

## Changes

1. Hoisted the per-template score data base index from each prefix/suffix loop into `const uint dataBase = j * 20`.
2. Replaced repeated `j * 20 + i` calculations with `dataBase + i`.
3. Preserved the direct Base58 address indexing from `perf-v006` and the early loop exits from `perf-v007`.

## Device scope

This is cross-device friendly for OpenCL GPUs. It does not change workgroup sizing, memory allocation, queueing, vendor tuning, or result semantics. The change only simplifies integer address calculation in the score kernel.

## Build

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1 -Clean
```

Build passed with Visual Studio C++ tools via `build.ps1`.

## Benchmark command

`benchmark.ps1` was attempted first:

```powershell
powershell -ExecutionPolicy Bypass -File .\benchmark.ps1 -Version perf-v008 -Notes "hoist score data base index in matching kernel"
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
- speed: 76.048 MH/s
- build: passed

## Comparison with previous version

- `perf-v007`: 76.019 MH/s, initialization 4 seconds, `inverse-multiple=49152`
- `perf-v008`: 76.048 MH/s, initialization 4 seconds, `inverse-multiple=49152`
- Change: +0.029 MH/s, about +0.04%

## Conclusion

This round is a valid low-risk cleanup in a hot score-kernel path, but the measured gain on the RTX 3070 is very small and should be considered effectively flat. It is worth keeping as a cross-device-friendly simplification, but it is not strong evidence of a material throughput improvement by itself.
