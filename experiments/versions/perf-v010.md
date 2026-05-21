# perf-v010

## Status

Completed, with a clear positive benchmark result on the RTX 3070 test device.

## Optimization target

Reduce score-kernel work for the common benchmark shape already specialized in `perf-v009`: one matching target, no meaningful prefix requirement, and a one-character suffix requirement.

## Files changed

- `kernel_profanity.hpp`
- `experiments/benchmark_summary.csv`
- `experiments/versions/perf-v010.md`

## Changes

1. Added `base58_last_char` in the OpenCL kernel source.
2. Moved the existing single-target one-character suffix fast path before full Base58 address generation.
3. For that fast path, compute only the final Base58 character by reducing the 25-byte TRON payload modulo 58.
4. Preserve full `base58_encode` and the generic matching loop for multi-target, prefix, and longer suffix searches.

## Device scope

This is cross-device friendly for OpenCL GPUs. It uses simple integer arithmetic and constant alphabet lookup, does not change workgroup sizing, memory allocation, command queues, or vendor-specific tuning, and only narrows work in a mode-specific branch.

## Build

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1 -Clean
```

Build passed with Visual Studio C++ tools via `build.ps1`.

## Benchmark command

`benchmark.ps1` was attempted first:

```powershell
powershell -ExecutionPolicy Bypass -File .\benchmark.ps1 -Version perf-v010 -Notes "single-suffix Base58 last-char fast path"
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
- speed: 102.485 MH/s
- build: passed

## Comparison with previous version

- `perf-v009`: 76.059 MH/s, initialization 4 seconds, `inverse-multiple=49152`
- `perf-v010`: 102.485 MH/s, initialization 4 seconds, `inverse-multiple=49152`
- Change: +26.426 MH/s, about +34.74%

## Conclusion

This round has clear value for the single-target one-character suffix path. Computing only the final Base58 character avoids full address string generation for the benchmark mode while preserving the generic path for broader searches. The result is a material throughput increase with unchanged initialization time and tuning parameters.
