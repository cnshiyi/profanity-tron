# perf-v087 - rejected time-gated speed logging

Date: 2026-05-22

## Goal

Test whether reducing runtime speed-line updates lowers host callback/output overhead enough to improve the real multi-target suffix5 path.

## Candidate

Changed `printSpeed()` to print at most once per second based on wall-clock time instead of printing after every device callback cycle.

This does not alter generation, matching, result validation, or private-key output.

## 60 Second Benchmark

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 60
```

Observed:

- Final: `355.219 MH/s`
- GPU0: `355.219 MH/s`
- Initialization: `23 seconds`

Comparison:

- Retained `perf-v083`: `355.734 MH/s`
- Delta: `-0.14%`

## Real Hit Test

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --quit-count 3 --output .\runtime\perf_v087_hits.txt
```

Observed:

- `Validated hits: 3`
- `Saved hits: 3`

Independent verification:

- `verified=3`
- `mismatched=0`

## Decision

Rejected and reverted. The change was functionally correct, but it did not beat the retained `perf-v083` real generation path. The result suggests speed-line output is not the limiting factor for the current GPU path.

