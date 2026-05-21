# perf-v080 - gated suffix5 modulo path rejected

Date: 2026-05-22

## Goal

Specialize the retained suffix5 workload without repeating the rejected global suffix4 bitset direction.

## Candidate

Keep the cheap `perf-v078` suffix2 gate for all candidates. Only after suffix2 passes, compute `TRON payload mod 58^5` in 32-bit chunks to derive the full five Base58 tail indices directly.

This was intended to avoid five rounds of 25-byte Base58 division for the rare suffix2 survivors.

## 60-second Benchmark

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 60
```

Observed:
- Final: `354.795 MH/s`
- Initialization: `23s`

Comparison:
- `perf-v078`: `355.108 MH/s`
- Delta: `-0.09%`

## Decision

Rejected. The extra `mod 58^5` work after suffix2 survivors did not beat the retained `perf-v078` tail expansion path on the required 60-second benchmark.

The source was reverted to the retained `perf-v078` path and rebuilt after rejection.
