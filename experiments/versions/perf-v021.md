# perf-v021

## Status

Rejected. This round tested a fixed-length SHA-256 input helper for the TRON checksum path, but the benchmark measured `169.970 MH/s`, below `perf-v020` at `171.064 MH/s`. The implementation was reverted after measurement, while the negative result was retained for comparison.

## Optimization target

Reduce per-candidate work in the single-target one-character suffix score path by avoiding construction of the 21-byte `0x41 + ethhash[20]` buffer before the first checksum SHA-256 call.

## Files changed

- `kernel_sha256.hpp` during the rejected probe, then reverted
- `kernel_profanity.hpp` during the rejected probe, then reverted
- `experiments/benchmark_summary.csv`
- `experiments/versions/perf-v021.md`

## Changes tested

1. Added a fixed `sha256_tron_ethhash` helper that directly initialized the SHA-256 message schedule from `ethhash`.
2. Updated `ethhash_to_tronhash` to use the fixed helper instead of filling a private `hash0[21]` buffer.
3. Updated the single-character suffix fast path to use the same fixed helper.
4. Reverted the code changes after benchmark because the measured throughput was lower than the previous version.

## Device scope

The attempted change was OpenCL cross-device friendly in principle because it used standard integer operations and fixed-size message schedule initialization. It affected the score kernel checksum path on all devices, including the current single-suffix benchmark fast path.

## Build

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1 -Clean
```

Build passed with Visual Studio C++ tools via `build.ps1`.

## Benchmark command

`benchmark.ps1` failed in this shell with a PowerShell `Start-Process` duplicate `Path/PATH` environment-key error before recording data, so the equivalent direct command was run and the parsed result was appended manually:

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

- work: 128
- inverse-multiple: 204800
- work-max: 52224000
- initialization: 17 seconds
- speed: 169.970 MH/s
- build: passed

## Comparison with previous version

- `perf-v020`: 171.064 MH/s, initialization 17 seconds, `work=128`, `inverse-multiple=204800`
- `perf-v021`: 169.970 MH/s, initialization 17 seconds, `work=128`, `inverse-multiple=204800`
- Change: -1.094 MH/s, about -0.64%

## Conclusion

This round is not worth keeping as code. The fixed 21-byte SHA input helper removed an apparent private-buffer setup step, but it likely increased register pressure or reduced compiler optimization quality in the hot score path. Keep the measurement as a negative result and avoid retrying this exact helper shape unless paired with a lower-register SHA schedule rewrite.
