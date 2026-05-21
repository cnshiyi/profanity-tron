# perf-v078 - word-chunk mod-3364 tail gate

Date: 2026-05-22

## Goal

Reduce the per-candidate cost of the retained `perf-v076` suffix2 early gate on the real multi-target suffix5 path:

```powershell
--matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5
```

## Change

`perf-v076` computed `TRON payload mod 3364` byte-by-byte across the 25-byte Base58Check payload.

This version computes the same remainder in 32-bit chunks:
- five 32-bit words from the 20-byte hash
- one 32-bit checksum word
- multiplier `2^32 mod 3364 = 480`

The resulting suffix2 semantics are unchanged.

## 60-second Benchmark

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 60
```

Observed:
- Final: `355.108 MH/s`
- Initialization: `23s`

Comparison:
- `perf-v076`: `353.247 MH/s`
- Delta: `+0.53%`

## Real Hit Test

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --quit-count 3 --output .\runtime\perf_v078_hits.txt
```

Observed:
- `Validated hits: 3`
- `Saved hits: 3`

Independent verification:
- `verified=3`
- `mismatched=0`

## Decision

Adopted. This is a modest but verified improvement on a 60-second run, and the real generated private keys independently derive the reported TRON addresses.
