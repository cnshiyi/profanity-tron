# perf-v084 - delayed tail index writes rejected

Date: 2026-05-22

## Goal

Trim a small amount of local array traffic from the adopted `perf-v083` shared-tail fast path.

Primary benchmark:

```powershell
.\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 60
```

## Candidate

For the all-exact multi-target shared-tail path introduced by `perf-v083`, the kernel delayed writing `tailIndices` until the generic per-target comparison loop was needed.

The intended benefit was to avoid local array writes when the shared-tail fast path would either reject early or emit immediately.

## 60-second Benchmark

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 60
```

Observed:

- Final: `355.525 MH/s`
- Initialization: `22s`

Comparison:

- `perf-v083`: `355.734 MH/s`
- Delta: `-0.06%`

## Real Hit Test

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --quit-count 3 --output .\runtime\perf_v084_hits.txt
```

Observed:

- `Validated hits: 3`
- `Saved hits: 3`

Independent verification:

- `verified=3`
- `mismatched=0`

## Decision

Rejected. The change was functionally correct but slightly slower than the retained `perf-v083` path, likely because the extra branches interfered with the compiler's local optimization.

The source was reverted to the retained `perf-v083` implementation and rebuilt after recording this result.
