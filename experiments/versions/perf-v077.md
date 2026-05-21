# perf-v077 - suffix4 bitset gate rejected

Date: 2026-05-22

## Goal

Try a more aggressive filter for the real multi-target suffix5 path:

```powershell
--matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5
```

## Candidate

Add a global-memory bitset for exact suffix4 combinations:
- compute `TRON payload mod 58^4`
- use that value as a bitset lookup before full Base58 tail expansion
- only candidates passing suffix4 continue to full tail matching

This was intentionally more aggressive than `perf-v076`, trading extra modulo work and a larger GPU table for stronger early rejection.

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
- Final: `349.695 MH/s`
- Initialization: `23s`

Comparison:
- `perf-v076`: `353.247 MH/s`
- Delta: `-1.01%`

## Decision

Rejected. The stronger suffix4 gate did not pay for its extra modulo and global-memory bitset cost on RTX 3070.

The source was reverted to the retained `perf-v076` path and rebuilt after rejection.
