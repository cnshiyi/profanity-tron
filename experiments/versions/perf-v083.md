# perf-v083 - shared suffix tail micro-bundle

Date: 2026-05-22

## Goal

Bundle a small real-path optimization for the current multi-address suffix5 workload without reintroducing rejected broad table or modulo experiments.

Primary benchmark:

```powershell
.\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 60
```

## Change

For all-exact multi-target suffix searches, the host now detects whether every target shares the same suffix characters after the first two tail Base58 digits.

For the current `runtime\multi10_targets.txt` shape:

```text
...mao8D
...mao8A
...mao8B
...
```

the first two tail digits are the variable target set, while the remaining suffix tail is common.

The kernel keeps the retained `perf-v078` suffix2 allow gate, then:

- expands only the remaining common tail digits
- returns early on the first mismatch
- emits a hit immediately after the shared tail passes
- skips the per-target exact comparison loop for this shape

The generic multi-target exact and wildcard paths remain available for target sets that do not share the same tail after suffix2.

## 60-second Benchmark

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 60
```

Observed:

- Final: `355.734 MH/s`
- Initialization: `23s`

Comparison:

- `perf-v078`: `355.108 MH/s`
- Delta: `+0.18%`

## Real Hit Test

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --quit-count 3 --output .\runtime\perf_v083_hits.txt
```

Observed:

- `Validated hits: 3`
- `Saved hits: 3`

Independent verification:

- `verified=3`
- `mismatched=0`

## Decision

Adopted cautiously. The gain is modest but the code path is real, correctness was independently verified, and the change only enables for all-exact multi-target lists with a common suffix tail after the first two tail digits.
