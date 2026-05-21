# perf-v022

## Status

Completed. This round has a small positive scheduling gain on the benchmark RTX 3070: raising the high-memory NVIDIA automatic batch size from `inverse-multiple=204800` to `221184` measured `172.405 MH/s` versus `171.064 MH/s` recorded for `perf-v020`, with initialization increasing from 17 seconds to 18 seconds.

## Optimization target

Improve sustained throughput on high-memory NVIDIA devices by retuning the default amount of work submitted per inverse/iterate/score cycle after `perf-v020` lowered the local work size to `128`.

## Files changed

- `profanity.cpp`
- `experiments/benchmark_summary.csv`
- `experiments/versions/perf-v022.md`

## Changes

1. Raised the high-memory NVIDIA automatic `inverse-multiple` target from `204800` to `221184`.
2. Preserved the high-memory NVIDIA local work size at `128`.
3. Preserved lower-memory NVIDIA, AMD, Intel, OpenCL kernel logic, matching semantics, and result scoring behavior.
4. Kept the change limited to the existing large-batch memory bucket, currently devices reporting at least roughly 7.5 GiB of OpenCL global memory.

## Device scope

This is intended for NVIDIA GPUs in the existing large-batch memory bucket. Smaller NVIDIA devices and non-NVIDIA devices keep their previous tuning.

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
- inverse-multiple: 221184
- work-max: 56401920
- initialization: 18 seconds
- speed: 172.405 MH/s
- build: passed

## Comparison with previous accepted version

- `perf-v020`: 171.064 MH/s, initialization 17 seconds, `work=128`, `inverse-multiple=204800`
- `perf-v022`: 172.405 MH/s, initialization 18 seconds, `work=128`, `inverse-multiple=221184`
- Change: +1.341 MH/s, about +0.78%

`perf-v021` was a rejected code probe and remains recorded as a negative result, so this comparison uses the previous accepted default.

## Probes during this round

1. Current default `--work 128 --inverse-multiple 204800` measured `171.066 MH/s` with 17 seconds initialization.
2. `--work 128 --inverse-multiple 212992` measured `172.294 MH/s` with 18 seconds initialization.
3. `--work 128 --inverse-multiple 221184` measured `173.453 MH/s` with 18 seconds initialization during probing.
4. `--work 128 --inverse-multiple 229376` measured `172.507 MH/s` with 19 seconds initialization.
5. The final default run after changing auto-tune selected `work=128`, `inverse-multiple=221184` and measured `172.405 MH/s`.

Two earlier parallel probes at `229376` and `245760` were discarded because concurrent GPU benchmark runs caused resource contention and produced unreliable results.

## Conclusion

This round is worth keeping. The `221184` batch size is the best measured point in the local serial probe set and gives a modest throughput improvement over the previous accepted default, with only a one-second initialization cost increase. The change stays scoped to the high-memory NVIDIA tuning bucket so lower-memory and non-NVIDIA devices retain their existing conservative parameters.
