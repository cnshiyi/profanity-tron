# perf-v082 - Base58 divmod lookup rejected

Date: 2026-05-22

## Goal

Use GPU memory to reduce the cost of Base58 tail expansion on the real multi-target suffix5 path:

```powershell
.\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 60
```

## Candidate

Added a `58 * 256` table of packed Base58 division results:

```text
index = rem * 256 + byte
value = (quotient << 8) | nextRem
```

The table was uploaded once per device as a read-only OpenCL buffer and passed into `profanity_score_matching`.

The hot Base58 tail functions then replaced:

```c
value = (rem << 8) | byte;
quotient = value / 58;
rem = value - quotient * 58;
```

with a lookup.

## 60-second Benchmark

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 60
```

Observed:

- Final: `353.969 MH/s`
- Initialization: `23s`

Comparison:

- `perf-v078`: `355.108 MH/s`
- Delta: `-0.32%`

## Real Hit Test

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --quit-count 3 --output .\runtime\perf_v082_hits.txt
```

Observed:

- `Validated hits: 3`
- `Saved hits: 3`

Independent verification:

- `verified=3`
- `mismatched=0`

## Decision

Rejected. The memory lookup was functionally correct but slower than the retained integer arithmetic path.

Likely reason: the extra memory dependency and buffer argument cost more than the saved integer division on the RTX 3070 OpenCL compiler path.

The source was reverted to the retained `perf-v078` implementation and rebuilt after recording this result.
