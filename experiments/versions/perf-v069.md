# perf-v069 - NVIDIA large-memory inverse-multiple retune to 245760 (rejected)

Date: 2026-05-21

## Goal

Continue optimizing the real production path:

`--matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5`

Constraints:
- No benchmark-only path.
- Keep real hit flow valid and keep independent `private -> TRON address` derivation consistent.

## New Direction

Avoid repeating recently rejected kernel branch experiments (`perf-v066` to `perf-v068`).
Use a new direction: retune launch parameters for the same real path.

Candidate:
- Keep `work=128`.
- Retune NVIDIA large-memory `inverse-multiple` from `237568` to `245760`.
- Do not keep the change if gain is only noise-level.

## Reproducible Benchmark

Baseline (old parameter):

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 5 --work 128 --inverse-multiple 237568
```

- Final: `356.464 MH/s`
- Initialization: `22s`

Candidate (new parameter):

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 5 --work 128 --inverse-multiple 245760
```

- Final: `356.889 MH/s`
- Initialization: `23s`

Result:
- Delta: `+0.12%`
- Decision: **not adopted** (noise-level gain; below adoption bar).

Failure reason:
- Slight throughput increase is within run-to-run noise and does not justify changing retained defaults.

## Real Hit Test (retained path)

Run:

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --quit-count 3 --work 128 --inverse-multiple 245760 --output .\runtime\perf_v069_hits.txt
```

Result:
- `Validated hits: 3`
- `Saved hits: 3`

## Independent private->address Verification

Used an independent verifier (`ecdsa + Keccak-256 + Base58Check`) on `runtime\perf_v069_hits.txt`:

- `verified=3`
- `mismatched=0`

## Conclusion

- This round tried a new direction (parameter retune) on the real usable generation path.
- Candidate did not meet adoption threshold and is rejected.
- Retained implementation remains `perf-v065` behavior and retained default auto-tune (`237568`).
- Real hit and independent private-to-address verification passed.
