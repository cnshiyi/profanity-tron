# perf-v017

## Status

Completed. This round keeps a small but measurable scoring-path improvement: the benchmark reached `156.215 MH/s` on the RTX 3070 versus `153.404 MH/s` for `perf-v016`, with initialization unchanged at 11 seconds.

## Optimization target

Reduce per-candidate work in the OpenCL scoring path used by the common single suffix benchmark. The suffix fast path only needs the first checksum word from the second SHA-256 call to compute the final Base58 character, not the full 32-byte digest.

## Files changed

- `kernel_sha256.hpp`
- `kernel_profanity.hpp`
- `experiments/benchmark_summary.csv`
- `experiments/versions/perf-v017.md`

## Changes

1. Reduced the SHA-256 message schedule storage from 80 words to the 64 words actually used by the compression loop.
2. Cleared only the initial 16 message words before loading and expanding a one-block SHA-256 message.
3. Added `sha256_first_word()` for checksum paths that only need digest word 0.
4. Updated the single-suffix Base58 fast path to use `sha256_first_word()` instead of materializing the full second checksum hash.
5. Kept the full `sha256()` output path for general address formatting and fallback matching behavior.

## Device scope

This is a cross-device OpenCL kernel optimization. It reduces private memory writes and schedule footprint in SHA-256 code used by NVIDIA, AMD, Intel, and other OpenCL implementations. The benchmarked win is on NVIDIA RTX 3070 with the existing large-batch auto-tune profile.

## Build

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1 -Clean
```

Build passed with Visual Studio C++ tools via `build.ps1`.

## Benchmark command

`benchmark.ps1` was attempted first:

```powershell
powershell -ExecutionPolicy Bypass -File .\benchmark.ps1 -Version perf-v017 -Notes "larger init chunks for large batches"
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
- speed: 156.215 MH/s
- build: passed

## Comparison with previous version

- `perf-v016`: 153.404 MH/s, initialization 11 seconds
- `perf-v017`: 156.215 MH/s, initialization 11 seconds
- Change: +2.811 MH/s, about +1.83%

## Probes during this round

1. Increasing large-batch initialization chunks from 20 to 10 measured `154.822 MH/s`, but initialization stayed at 11 seconds.
2. Increasing large-batch initialization chunks from 20 to 5 measured `153.434 MH/s`, and initialization still stayed at 11 seconds.
3. The initialization chunk experiment was rejected because it did not improve the intended metric.
4. The retained checksum-first-word scoring path measured `156.215 MH/s` with the original 20-step initialization progress behavior restored.

## Conclusion

This round is worth keeping. The checksum SHA path now computes and returns only the digest word needed by the single-suffix Base58 fast path, trimming private memory pressure and unused output stores without changing matching semantics.
