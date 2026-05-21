# perf-v025

## Status

Rejected. This round tested replacing the single-target one-character suffix Base58 remainder helper with a fixed-weight modulo-58 sum for the 25-byte TRON payload. It built and ran correctly, but measured `174.587 MH/s`, effectively tied with `perf-v023` at `174.588 MH/s`, so the probe was reverted.

## Optimization target

Reduce integer modulo work in the existing fast path for `matchingCount == 1`, `prefixCount <= 1`, and `suffixCount == 1`. The retained `perf-v023` path computes the final Base58 digit through grouped 32-bit chunks and several `% 58` operations. This probe attempted to pre-expand `256^n mod 58` weights and finish with one `% 58`.

## Files changed

- `experiments/benchmark_summary.csv`
- `experiments/versions/perf-v025.md`

The temporary code probe touched `kernel_profanity.hpp` during measurement, then was reverted because it did not improve throughput.

## Changes tested

1. Replaced `BASE58_MOD58_WORD` chunk accumulation in `base58_last_index_from_ethhash` with fixed byte weights for the TRON prefix, 20-byte hash, and 4-byte checksum prefix.
2. Kept the existing one-character suffix fast-path conditions unchanged.
3. Kept scheduling defaults unchanged at `work=128` and `inverse-multiple=237568`.
4. Reverted the kernel change after measurement because the result was noise-level equal to the current default.

## Device scope

The tested change was cross-device friendly in principle: it used only standard OpenCL integer arithmetic and did not change memory allocation, local work size, vendor tuning, or result semantics. It was not retained because the local RTX 3070 benchmark did not show value.

## Build

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1 -Clean
```

Build passed with Visual Studio C++ tools via `build.ps1`.

## Benchmark command

`benchmark.ps1` hit a local PowerShell `Start-Process` environment issue (`Path`/`PATH` duplicate key), so the equivalent executable command was run directly and the summary row was appended manually:

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
- initialization: 20 seconds
- speed: 174.587 MH/s
- build: passed

## Comparison with previous version

- `perf-v023`: 174.588 MH/s, initialization 20 seconds, `work=128`, `inverse-multiple=237568`
- `perf-v024` best rejected probe: 174.094 MH/s, initialization 20 seconds, `work=128`, `inverse-multiple=239616`
- `perf-v025` weighted remainder probe: 174.587 MH/s, initialization 20 seconds, `work=128`, `inverse-multiple=237568`
- Change vs `perf-v023`: -0.001 MH/s, effectively no change

## Conclusion

Do not keep the weighted Base58 remainder rewrite. The compiler and GPU appear to handle the existing grouped modulo path well enough that the fixed-weight form provides no measurable throughput gain on this device. Future work should look at higher-impact paths such as reducing score-kernel SHA pressure, profiling kernel occupancy/register pressure, or improving host scheduling around result reads.
