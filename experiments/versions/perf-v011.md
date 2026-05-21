# perf-v011

## Status

Completed. The benchmark result is slightly positive versus `perf-v010`, but the gain is small enough to treat as a low-risk hot-path simplification rather than a clear throughput breakthrough.

## Optimization target

Reduce arithmetic in the `perf-v010` single-target one-character suffix fast path. That path only needs the final Base58 character of the fixed 25-byte TRON payload, so this round focuses on computing the payload remainder with fewer loop iterations.

## Files changed

- `kernel_profanity.hpp`
- `experiments/benchmark_summary.csv`
- `experiments/versions/perf-v011.md`

## Changes

1. Replaced the byte-by-byte `base58_last_char` helper with `base58_last_char25`.
2. Added `base58_mod58_word` to process aligned 4-byte chunks of the fixed 25-byte TRON payload.
3. Reduced the fast-path remainder update from 25 byte iterations to one leading byte plus six 32-bit chunk remainders.
4. Kept the generic full `base58_encode` path unchanged for multi-target, prefix, and longer suffix searches.

## Device scope

This is cross-device friendly for OpenCL GPUs. It uses standard integer arithmetic, constant memory lookup, and a fixed-size helper for the already fixed TRON payload length. It does not change workgroup sizing, memory allocation, command queues, vendor tuning, or result semantics outside the existing single-target one-character suffix fast path.

## Build

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1 -Clean
```

Build passed with Visual Studio C++ tools via `build.ps1`.

## Benchmark command

`benchmark.ps1` was attempted first:

```powershell
powershell -ExecutionPolicy Bypass -File .\benchmark.ps1 -Version perf-v011 -Notes "word-wise Base58 last-char remainder"
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
- speed: 102.531 MH/s
- build: passed

## Comparison with previous version

- `perf-v010`: 102.485 MH/s, initialization 4 seconds, `inverse-multiple=49152`
- `perf-v011`: 102.531 MH/s, initialization 4 seconds, `inverse-multiple=49152`
- Change: +0.046 MH/s, about +0.04%

## Conclusion

This round is worth keeping as a narrow hot-path simplification, but it should not be treated as strong evidence of a material throughput gain on the RTX 3070. The implementation reduces per-candidate remainder-loop work in the existing fast path while preserving the generic score kernel behavior.
