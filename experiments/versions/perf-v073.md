# perf-v073 - all-exact suffix5 packed compare (rejected)

Date: 2026-05-22

## Goal

Continue optimizing the real usable TRON generation path only:

`--matching .\\runtime\\multi10_targets.txt --prefix-count 0 --suffix-count 5`

Constraints:
- no benchmark-only retained behavior
- must keep real-hit validity and independent `private -> TRON address` verification

## Pre-run Guardrails

- Checked `git status --short --branch` first.
- Checked residual `profanity` / `TronStudio` processes first.
- Only terminated matched repo-local `profanity.x64.exe` process instances.

## New Direction

Try a new direction not previously recorded as failed:
- in `matchingCount > 1 && prefixCount <= 1 && suffixCount == 5 && allExact` path,
- replace per-target 5-byte indexed compare with packed compare:
  - candidate uses `(tail[0..3] packed uint) + tail[4] scalar` compare per target.
- baseline disables this candidate path with env gate on the same binary:
  - `PROFANITY_DISABLE_TAIL5_PACKED=1`.

After A/B decision, candidate logic was reverted from retained runtime path.

## Build

```powershell
powershell -ExecutionPolicy Bypass -File .\\build.ps1
```

Result: build succeeded.

## Reproducible Benchmark

Baseline (candidate disabled):

```powershell
set PROFANITY_DISABLE_TAIL5_PACKED=1 && C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 5 --work 128 --inverse-multiple 237568
```

- Final: `353.759 MH/s`
- Initialization: `23s`

Candidate (packed compare enabled):

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 5 --work 128 --inverse-multiple 237568
```

- Final: `1.305 MH/s`
- Initialization: `35s`

Result:
- Delta: `-99.63%`
- Decision: **not adopted**.

Failure reason:
- packed compare branch causes catastrophic throughput collapse on the real suffix5 path.

## Real Hit Test (retained path)

Run (retained path after revert):

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --quit-count 1 --work 128 --inverse-multiple 237568 --output .\runtime\perf_v073_revert_hits.txt
```

Result:
- `Validated hits: 1`
- `Saved hits: 1`

## Independent private->address Verification

Used independent verifier (`ecdsa + Keccak-256 + Base58Check`) on `runtime\\perf_v073_revert_hits.txt`:

- `verified=2`
- `mismatched=0`

## Conclusion

- New direction (`tail5 packed compare`) is rejected.
- Retained path remains the pre-candidate runtime path.
- Real-hit generation and independent private-to-address derivation remain valid.
