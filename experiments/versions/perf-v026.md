# perf-v026

## Status

Rejected. This round tested a specialized SHA-256 helper for the existing single-target, one-character suffix fast path. The probe built and ran, but measured `122.477 MH/s`, well below the retained `perf-v023`/`perf-v025` default around `174.6 MH/s`, so the code change was reverted.

## Optimization target

Reduce per-work-item setup in `base58_last_index_from_ethhash` by replacing the local `hash0[21]` buffer plus generic `sha256(sizeof(hash0), hash0, hash1)` input packing with a fixed 21-byte TRON payload SHA helper.

## Files changed

- `experiments/benchmark_summary.csv`
- `experiments/versions/perf-v026.md`

The temporary probe touched `kernel_sha256.hpp` and `kernel_profanity.hpp`, then was reverted because it slowed the hot score path.

## Changes tested

1. Added a `sha256_tron_payload` helper that packed the fixed `0x41 || ethhash[20]` payload directly into the SHA message schedule.
2. Wired `base58_last_index_from_ethhash` to call that helper in the existing single-suffix fast path.
3. Kept scheduling defaults unchanged at `work=128` and `inverse-multiple=237568`.
4. Reverted the helper after measurement because throughput regressed sharply.

## Device scope

The probe was cross-device compatible in principle: it used standard OpenCL integer operations and changed no memory allocation, local work size, vendor tuning, or matching semantics. It was not retained because the local NVIDIA benchmark showed a clear regression.

## Build

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1 -Clean
```

Build passed with Visual Studio C++ tools via `build.ps1`.

## Benchmark command

The equivalent direct benchmark command was run after the build:

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
- inverse-multiple: 237568
- work-max: 60579840
- initialization: 22 seconds
- speed: 122.477 MH/s
- build: passed

## Comparison with previous version

- `perf-v023`: 174.588 MH/s, initialization 20 seconds, `work=128`, `inverse-multiple=237568`
- `perf-v025`: 174.587 MH/s, initialization 20 seconds, `work=128`, `inverse-multiple=237568`
- `perf-v026` fixed TRON payload helper probe: 122.477 MH/s, initialization 22 seconds, `work=128`, `inverse-multiple=237568`
- Change vs `perf-v025`: -52.110 MH/s, about -29.85%

## Probes during this round

1. Fixed TRON payload SHA helper probe measured `122.477 MH/s` with 22 seconds initialization.
2. After reverting the helper and rebuilding, the default code still built and ran. Two noisy sanity runs measured `95.977 MH/s` with 22 seconds initialization and `100.091 MH/s` with 21 seconds initialization, so they were treated as environment/load sanity checks rather than retained optimization results.

## Conclusion

Do not keep or retry this helper shape. It overlaps with the negative direction recorded in `perf-v021` and likely increases register pressure or harms the OpenCL compiler's handling of the SHA path enough to dominate the small input-packing saving. Future runs should avoid fixed TRON payload SHA rewrites unless they also reduce SHA schedule storage or prove lower register pressure.
