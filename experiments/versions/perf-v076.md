# perf-v076 - direct mod-3364 suffix2 gate

Date: 2026-05-22

## Goal

Improve the real usable multi-target suffix path:

```powershell
--matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5
```

This round keeps the `perf-v075` rule: adoption requires a 60-second benchmark plus real-hit validation.

## Change

For the all-exact multi-target suffix2-12 path, the previous retained path computed the final two Base58 tail indices by constructing the 25-byte TRON payload and running two full Base58 division passes.

This version computes the 25-byte payload modulo `58 * 58` directly:
- `mod % 58` gives the final Base58 digit
- `(mod / 58) % 58` gives the second-final Base58 digit

Only candidates passing that two-digit allow table continue to full tail expansion.

## Build

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1
```

Result: build succeeded.

## 60-second Benchmark

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 60
```

Observed:
- Final: `353.247 MH/s`
- Initialization: `23s`

Baseline:
- `perf-v075-60s-baseline`: `350.996 MH/s`

Delta:
- `+0.64%`

## Real Hit Test

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --quit-count 3 --output .\runtime\perf_v076_hits.txt
```

Observed:
- `Validated hits: 3`
- `Saved hits: 3`

Independent verification:
- `verified=3`
- `mismatched=0`

## Decision

Adopted. The improvement is modest but measured with the required 60-second benchmark and preserves real wallet generation correctness.
