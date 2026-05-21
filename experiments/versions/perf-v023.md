# perf-v023

## Status

Completed. This round has a repeatable scheduling gain on the benchmark RTX 3070: raising the high-memory NVIDIA automatic batch size from `inverse-multiple=221184` to `237568` measured `174.588 MH/s` versus `172.405 MH/s` recorded for `perf-v022`, with initialization increasing from 18 seconds to 20 seconds.

## Optimization target

Improve sustained throughput on high-memory NVIDIA devices by nudging the default amount of work submitted per inverse/iterate/score cycle past the `perf-v022` setting while keeping the local work size and kernel code unchanged.

## Files changed

- `profanity.cpp`
- `experiments/benchmark_summary.csv`
- `experiments/versions/perf-v023.md`

## Changes

1. Raised the high-memory NVIDIA automatic `inverse-multiple` target from `221184` to `237568`.
2. Preserved the high-memory NVIDIA local work size at `128`.
3. Preserved lower-memory NVIDIA, AMD, Intel, OpenCL kernel logic, matching semantics, memory layout, and result scoring behavior.
4. Kept the change limited to the existing large-batch memory bucket, currently devices reporting at least roughly 7.5 GiB of OpenCL global memory.

## Device scope

This is intended for NVIDIA GPUs in the existing large-batch memory bucket. Smaller NVIDIA devices and non-NVIDIA devices keep their previous tuning.

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
- initialization: 20 seconds
- speed: 174.588 MH/s
- build: passed

## Comparison with previous version

- `perf-v022`: 172.405 MH/s, initialization 18 seconds, `work=128`, `inverse-multiple=221184`
- `perf-v023`: 174.588 MH/s, initialization 20 seconds, `work=128`, `inverse-multiple=237568`
- Change: +2.183 MH/s, about +1.27%

## Probes during this round

1. Current default `--work 128 --inverse-multiple 221184` measured `171.380 MH/s` with 18 seconds initialization.
2. `--work 128 --inverse-multiple 229376` measured `172.513 MH/s` with 19 seconds initialization.
3. `--work 128 --inverse-multiple 237568` measured `173.603 MH/s` with 20 seconds initialization during probing.
4. `--work 128 --inverse-multiple 245760` measured `176.575 MH/s` with 20 seconds initialization during probing, but did not reproduce after becoming the default.
5. `--work 128 --inverse-multiple 253952` measured `138.111 MH/s` with 21 seconds initialization and was rejected.
6. A repeat `--work 128 --inverse-multiple 237568` probe measured `173.605 MH/s` with 20 seconds initialization.
7. The final default run after changing auto-tune selected `work=128`, `inverse-multiple=237568` and measured `174.588 MH/s`.

## Conclusion

This round is worth keeping. The `237568` batch size is more conservative than the noisy `245760` probe and reproduced consistently in the same session before the final default benchmark. It improves throughput over `perf-v022` at the cost of about two additional initialization seconds, with the change scoped to high-memory NVIDIA scheduling only.
