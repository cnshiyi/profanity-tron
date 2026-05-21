# perf-v009

## Status

Completed, with a very small positive benchmark result that should be treated as close to run-to-run noise.

## Optimization target

Reduce score-kernel overhead for the common single-target benchmark shape: `matchingCount == 1`, `prefix-count <= 1`, and `suffix-count == 1`.

## Files changed

- `kernel_profanity.hpp`
- `experiments/benchmark_summary.csv`
- `experiments/versions/perf-v009.md`

## Changes

1. Added a narrow fast path in `profanity_score_matching` after Base58 address generation.
2. For single-target one-character suffix matching, the kernel now checks `data1[19]` and `data2[19]` directly against `tron_hash_address[33]`.
3. The fast path returns immediately after the direct check, avoiding the generic per-target loop, prefix/suffix counters, index arithmetic, and loop branches.
4. The generic path remains unchanged for multiple targets, longer suffix searches, and prefix-heavy searches.

## Device scope

This is cross-device friendly for OpenCL GPUs. It does not change workgroup sizing, memory allocation, command queue behavior, or vendor tuning. It only shortens a hot score-kernel branch for a common matching mode.

## Build

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1 -Clean
```

Build passed with Visual Studio C++ tools via `build.ps1`.

## Benchmark command

`benchmark.ps1` was attempted first:

```powershell
powershell -ExecutionPolicy Bypass -File .\benchmark.ps1 -Version perf-v009 -Notes "single-match suffix fast path in score kernel"
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
- speed: 76.059 MH/s
- build: passed

## Comparison with previous version

- `perf-v008`: 76.048 MH/s, initialization 4 seconds, `inverse-multiple=49152`
- `perf-v009`: 76.059 MH/s, initialization 4 seconds, `inverse-multiple=49152`
- Change: +0.011 MH/s, about +0.01%

## Conclusion

This round keeps a low-risk specialization for a common single-target suffix benchmark path. The RTX 3070 measurement is slightly positive but effectively flat, so the value is primarily reduced branch and counter work in the score kernel rather than a clearly material throughput gain.
