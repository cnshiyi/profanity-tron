# perf-v088 - rejected /3364 tail drop

Date: 2026-05-22

## Goal

Try to reduce work in the retained `perf-v083` all-exact multi-target shared-tail path without repeating the rejected delayed-tail-write candidate from `perf-v084`.

## Candidate

Added a helper that divides the TRON payload by `3364` once to drop the first two Base58 tail digits after the existing suffix2 gate. This replaced two consecutive `base58_next_tail_index()` passes before checking the shared suffix tail.

This change stayed on the real wallet-generation path and did not alter private-key generation, address validation, or output.

## 60 Second Benchmark

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 60
```

Observed:

- Final: `355.422 MH/s`
- GPU0: `355.422 MH/s`
- Initialization: `23 seconds`
- OpenCL output included `2 warnings generated.`

Comparison:

- Retained `perf-v083`: `355.734 MH/s`
- Delta: `-0.09%`

## Real Hit Test

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --quit-count 3 --output .\runtime\perf_v088_hits.txt
```

Observed:

- `Validated hits: 3`
- `Saved hits: 3`

Independent verification:

- `verified=3`
- `mismatched=0`

## Decision

Rejected and reverted. The candidate was functionally correct, but slower than the retained `perf-v083` path and produced OpenCL compile warnings. The retained two `/58` tail-drop passes remain better for this GPU/compiler path.

