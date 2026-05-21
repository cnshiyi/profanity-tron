# perf-v015

## Status

Completed. This round has clear value on the benchmark RTX 3070: raising the high-memory NVIDIA automatic batch size measured `148.076 MH/s` versus `120.846 MH/s` for `perf-v014`, with initialization increasing from 5 seconds to 11 seconds.

## Optimization target

Improve sustained GPU throughput on large-memory NVIDIA devices by reducing host scheduling overhead and increasing the default amount of work submitted per inverse/iterate/score cycle.

## Files changed

- `profanity.cpp`
- `experiments/benchmark_summary.csv`
- `experiments/versions/perf-v015.md`

## Changes

1. Raised the large-memory NVIDIA automatic `inverse-multiple` target from `65536` to `131072`.
2. Preserved the existing NVIDIA local work size of `512`.
3. Preserved the lower-memory NVIDIA fallback at `32768`.
4. Left AMD, Intel, OpenCL kernel logic, matching semantics, and result scoring unchanged.

## Device scope

This is intended for NVIDIA GPUs in the existing large-batch memory bucket, currently devices reporting at least roughly 7.5 GiB of OpenCL global memory. The tested RTX 3070 reports 8589410304 bytes and completed the larger allocation. Smaller NVIDIA devices and non-NVIDIA devices keep their previous tuning.

## Build

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1 -Clean
```

Build passed with Visual Studio C++ tools via `build.ps1`.

## Benchmark command

`benchmark.ps1` was attempted first:

```powershell
powershell -ExecutionPolicy Bypass -File .\benchmark.ps1 -Version perf-v015 -Notes "raise NVIDIA large-batch inverse multiple to 131072"
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
- inverse-multiple: 131072
- work-max: 33423360
- initialization: 11 seconds
- speed: 148.076 MH/s
- build: passed

## Comparison with previous version

- `perf-v014`: 120.846 MH/s, initialization 5 seconds, `inverse-multiple=65536`
- `perf-v015`: 148.076 MH/s, initialization 11 seconds, `inverse-multiple=131072`
- Change: +27.230 MH/s, about +22.53%

## Rejected attempts during this round

1. A score-kernel helper that computed only the first word of the second SHA256 digest for the single-character suffix fast path built and ran correctly, but measured `120.846 MH/s`, which was indistinguishable from `perf-v014`. It was reverted before finalizing this version.
2. A command-line probe with `--inverse-multiple 98304` measured `137.714 MH/s` with 8 seconds of initialization. This was better than `perf-v014`, but below the `131072` default selected for this round.

## Conclusion

This round is worth keeping. The larger NVIDIA high-memory batch materially improves measured throughput on the RTX 3070 while keeping the change scoped to automatic scheduling parameters. The tradeoff is a longer one-time initialization phase and higher GPU memory pressure, so the change remains limited to the existing large-memory NVIDIA bucket.
