# perf-v006

## Status

Completed.

## Optimization target

Reduce per-candidate work in `profanity_score_matching` by removing the temporary `matchingHash[20]` extraction buffer and matching directly against the Base58 address characters.

## Files changed

- `kernel_profanity.hpp`
- `experiments/benchmark_summary.csv`
- `experiments/versions/perf-v006.md`

## Changes

1. Removed the local `matchingHash[20]` array and its fill loop from the score kernel.
2. Prefix checks now read the same characters directly from `tron_hash_address[i]`.
3. Suffix checks now map the old compact matching index to the address index with `i + 14`, preserving the previous `i >= 24` suffix region.
4. Kept the existing OpenCL device tuning from `perf-v005`, including `work=512` and `inverse-multiple=49152` on large-memory NVIDIA devices.

## Device scope

This is intended to be cross-device friendly for OpenCL GPUs. It reduces private memory pressure and integer loop work in the score kernel without changing workgroup sizing, allocation sizes, or vendor-specific tuning.

## Build

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1 -Clean
```

Build passed with Visual Studio C++ tools via `build.ps1`.

## Benchmark command

`benchmark.ps1` was attempted first:

```powershell
powershell -ExecutionPolicy Bypass -File .\benchmark.ps1 -Version perf-v006 -Notes "direct Base58 address indexing in score kernel"
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
- speed: 76.025 MH/s
- build: passed

## Comparison with previous version

- `perf-v005`: 63.164 MH/s, initialization 5 seconds, `inverse-multiple=49152`
- `perf-v006`: 76.025 MH/s, initialization 4 seconds, `inverse-multiple=49152`
- Change: +12.861 MH/s, about +20.36%

## Conclusion

This round has clear value on the RTX 3070 benchmark device. Direct Base58 address indexing removes a per-candidate private buffer copy from the score kernel while preserving the existing matching layout, and it improved the measured main executable throughput from 63.164 MH/s to 76.025 MH/s.
