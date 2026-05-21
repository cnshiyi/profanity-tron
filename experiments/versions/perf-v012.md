# perf-v012

## Status

Completed. This round has clear value on the benchmark RTX 3070: increasing the large-memory NVIDIA batch size lifted measured throughput from `102.531 MH/s` to `119.165 MH/s`, with initialization rising from 4 seconds to 5 seconds.

## Optimization target

Reduce scheduling overhead and improve sustained GPU occupancy on large-memory NVIDIA devices by increasing the default `inverse-multiple` for the high-memory NVIDIA tuning bucket.

## Files changed

- `profanity.cpp`
- `experiments/benchmark_summary.csv`
- `experiments/versions/perf-v012.md`

## Changes

1. Raised the large-memory NVIDIA automatic `inverse-multiple` target from `49152` to `65536`.
2. Preserved the existing `work=512` NVIDIA local work size.
3. Preserved the lower-memory NVIDIA fallback at `inverse-multiple=32768`.
4. Left AMD, Intel, OpenCL kernel logic, memory layouts, and result semantics unchanged.

## Device scope

This is intended for NVIDIA GPUs with enough memory for the larger batch. In the current tuning code, the high-memory bucket starts at roughly 7.5 GiB of reported OpenCL global memory. Smaller NVIDIA devices still use the existing lower-memory setting, and non-NVIDIA devices keep their prior tuning.

## Build

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1 -Clean
```

Build passed with Visual Studio C++ tools via `build.ps1`.

## Benchmark command

`benchmark.ps1` was attempted first:

```powershell
powershell -ExecutionPolicy Bypass -File .\benchmark.ps1 -Version perf-v012 -Notes "raise NVIDIA large-batch inverse multiple to 65536"
```

The script still fails in this PowerShell environment because `Start-Process` receives duplicate `Path/PATH` environment keys. The equivalent command was run directly and the same fields were recorded manually:

```powershell
.\profanity.x64.exe --matching TUqEg3dzVEJNQSVW2HY98z5X8SBdhmao8D --prefix-count 1 --suffix-count 1 --quit-count 1
```

After changing the default auto-tune value, the same command without overrides measured `119.165 MH/s`.

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
- speed: 119.165 MH/s
- build: passed

## Comparison with previous version

- `perf-v011`: 102.531 MH/s, initialization 4 seconds, `inverse-multiple=49152`
- `perf-v012`: 119.165 MH/s, initialization 5 seconds, `inverse-multiple=65536`
- Change: +16.634 MH/s, about +16.22%

## Rejected attempts during this round

Three kernel-side candidates were tested and not kept:

1. Comparing the single-character Base58 suffix by computed Base58 index instead of loading `alphabet[rem]`: `102.524 MH/s`.
2. Reducing SHA256 private schedule storage from 80 words to 64 words: `102.493 MH/s`.
3. A transient run with an incorrect SHA256 compression-loop edit measured higher throughput, but that build was invalid and was discarded before the final benchmark.

The valid kernel-side candidates were below `perf-v011`, and the invalid transient build was discarded. None of these kernel experiments are included in the final code change.

## Conclusion

This round is worth keeping. On the RTX 3070, a larger NVIDIA batch size improves sustained throughput materially while adding only about one second of initialization time. The change is scoped to the existing high-memory NVIDIA tuning bucket, so it does not change matching correctness or non-NVIDIA device behavior.
