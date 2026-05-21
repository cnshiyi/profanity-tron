# perf-v013

## Status

Completed. This round shows a small positive result on the RTX 3070 benchmark device: `120.841 MH/s` versus `119.165 MH/s` for `perf-v012`, with the same initialization time and tuning parameters.

## Optimization target

Reduce work in the single-target, one-character suffix score fast path by avoiding construction of the full 25-byte TRON payload when only the final Base58 character is needed.

## Files changed

- `kernel_profanity.hpp`
- `experiments/benchmark_summary.csv`
- `experiments/versions/perf-v013.md`

## Changes

1. Added `base58_last_char_from_ethhash` for the narrow fast path used when `matchingCount == 1`, `prefixCount <= 1`, and `suffixCount == 1`.
2. The fast path now computes the Base58 final-character remainder directly from the 20-byte ETH hash plus the four checksum bytes.
3. Preserved the two SHA256 checksum calls required for TRON address correctness.
4. Kept the generic `ethhash_to_tronhash` plus full `base58_encode` path unchanged for multi-target, prefix-heavy, and longer suffix searches.

## Device scope

This is cross-device friendly for OpenCL GPUs. It uses standard integer arithmetic, private arrays already required by the checksum path, and global-memory reads from the existing candidate hash. It does not change workgroup sizing, command queues, memory allocation sizes, or vendor-specific tuning.

## Build

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1 -Clean
```

Build passed with Visual Studio C++ tools via `build.ps1`.

## Benchmark command

`benchmark.ps1` was attempted first:

```powershell
powershell -ExecutionPolicy Bypass -File .\benchmark.ps1 -Version perf-v013 -Notes "direct suffix remainder from eth hash"
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
- speed: 120.841 MH/s
- build: passed

## Comparison with previous version

- `perf-v012`: 119.165 MH/s, initialization 5 seconds, `inverse-multiple=65536`
- `perf-v013`: 120.841 MH/s, initialization 5 seconds, `inverse-multiple=65536`
- Change: +1.676 MH/s, about +1.41%

## Conclusion

This round is worth keeping as a narrow score-kernel fast-path improvement. The gain is modest, but the benchmark is positive under the same scheduling parameters as `perf-v012`, and the change avoids unnecessary full TRON payload construction in the common single-character suffix benchmark mode.
