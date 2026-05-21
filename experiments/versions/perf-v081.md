# perf-v081 - weighted byte-sum mod3364 rejected

Date: 2026-05-22

## Goal

Reduce the hot-path cost of the retained `perf-v078` suffix2 early gate for the real multi-target suffix5 workload:

```powershell
.\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 60
```

## Candidate

`perf-v078` computes `TRON payload mod 3364` in six 32-bit chunks.

This candidate replaced that with one weighted byte-sum over the fixed 25-byte payload:

- precomputed `256^n mod 3364` coefficients
- one final `% 3364`
- unchanged checksum and suffix2 semantics

The idea was to reduce repeated modulo operations in `tronhash_mod_3364_from_ethhash`.

## 60-second Benchmark

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 60
```

Observed:

- Final: `355.424 MH/s`
- Initialization: `23s`

Comparison:

- `perf-v078`: `355.108 MH/s`
- Delta: `+0.09%`

## Real Hit Test

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --quit-count 3 --output .\runtime\perf_v081_hits.txt
```

Observed:

- `Validated hits: 3`
- `Saved hits: 3`

Independent verification:

- `verified=3`
- `mismatched=0`

## Decision

Rejected. The 60-second result was only `+0.09%` over `perf-v078`, which is within noise for this workload.

The source was reverted to the retained `perf-v078` implementation and rebuilt after recording this result.
