# perf-v014

## Status

Completed. This round produced a near-noise positive result on the RTX 3070 benchmark device: `120.846 MH/s` versus `120.841 MH/s` for `perf-v013`, with unchanged initialization time and scheduling parameters.

## Optimization target

Reduce score-kernel work in the single-target, one-character suffix fast path by comparing the computed final Base58 digit index directly when the requested suffix character is fully specified.

## Files changed

- `Dispatcher.cpp`
- `kernel_profanity.hpp`
- `experiments/benchmark_summary.csv`
- `experiments/versions/perf-v014.md`

## Changes

1. Changed the fast-path helper to return the final Base58 alphabet index instead of immediately loading the alphabet character.
2. Added a host-side `base58IndexOf` helper and passed the requested one-character suffix index as an extra score-kernel argument.
3. For `mask == 0xff`, the score kernel now compares `suffixIndex == suffixMatchIndex` directly.
4. Kept the original character-mask comparison for partial/nibble masks so wildcard matching semantics remain unchanged.
5. Left the generic full-address path unchanged for multi-target, prefix-heavy, and longer suffix searches.

## Device scope

This is cross-device friendly for OpenCL GPUs. It uses one additional scalar kernel argument and standard integer comparison in an existing narrow branch. It does not change memory allocation sizes, workgroup sizing, command queues, or vendor-specific tuning.

## Build

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1 -Clean
```

Build passed with Visual Studio C++ tools via `build.ps1`.

## Benchmark command

`benchmark.ps1` was attempted first:

```powershell
powershell -ExecutionPolicy Bypass -File .\benchmark.ps1 -Version perf-v014 -Notes "direct full-mask suffix index compare"
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
- inverse-multiple: 65536
- work-max: 16711680
- initialization: 5 seconds
- speed: 120.846 MH/s
- build: passed

## Comparison with previous version

- `perf-v013`: 120.841 MH/s, initialization 5 seconds, `inverse-multiple=65536`
- `perf-v014`: 120.846 MH/s, initialization 5 seconds, `inverse-multiple=65536`
- Change: +0.005 MH/s, about +0.004%

## Conclusion

This round is behaviorally valid and keeps the single-character suffix fast path slightly more direct, but the measured gain is effectively run-to-run noise. The change is worth keeping as a low-risk hot-path simplification, not as evidence of a material throughput improvement.
