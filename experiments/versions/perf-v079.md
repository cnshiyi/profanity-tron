# perf-v079 - direct mod3364 allow table rejected

Date: 2026-05-22

## Goal

Reduce hot-path reject cost after `perf-v078` by storing the suffix2 allow table in direct `mod3364` order.

## Candidate

Host-side table layout changed from:

```text
idx0 * 58 + idx1
```

to:

```text
idx0 + idx1 * 58
```

The kernel then checked:

```c
suffixTail2Allowed[mod3364]
```

before calculating `tailIndices[0]` and `tailIndices[1]`. This was intended to avoid two divisions/modulo operations for the vast majority of rejected candidates.

## 60-second Benchmark

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 60
```

Observed:
- Final: `354.596 MH/s`
- Initialization: `22s`

Comparison:
- `perf-v078`: `355.108 MH/s`
- Delta: `-0.14%`

## Decision

Rejected. The change is too small and measured slightly slower on the required 60-second benchmark.

The source was reverted to the retained `perf-v078` path and rebuilt after rejection.
