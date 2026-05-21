# perf-v074 - NVIDIA worksize retune to 256 (rejected)

Date: 2026-05-22

## Goal

Continue optimizing only the real usable TRON generation path:

`--matching .\\runtime\\multi10_targets.txt --prefix-count 0 --suffix-count 5`

Constraints kept:
- no benchmark-only retained behavior
- must keep real-hit validity and independent `private -> TRON address` derivation

## Pre-run Checks

- Checked `git status --short --branch` first.
- Checked residual `profanity` / `TronStudio` processes first.
- Only terminated repo-local matched `profanity.x64.exe` processes.

## New Direction

Try a new direction not previously recorded as failed:
- keep retained path logic unchanged
- retune runtime launch parameter from `--work 128` to `--work 256`
- keep `--inverse-multiple 245760` fixed

Reason: this targets scheduler occupancy on the same real usable path without introducing benchmark-only behavior.

## Build

```powershell
powershell -ExecutionPolicy Bypass -File .\\build.ps1
```

Result: build succeeded, `profanity.x64.exe` rebuilt.

## Reproducible Benchmark

Baseline:

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 5 --work 128 --inverse-multiple 245760
```

Observed:
- Final: `347.202 MH/s`
- Initialization: `23s`
- Evidence log: `runtime/perf_v073_base_w128_i245760.run.txt`

Candidate:

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 5 --work 256 --inverse-multiple 245760
```

Observed:
- command unstable, `EXITCODE:-1`
- did not produce valid `Final`
- evidence log: `runtime/perf_v074_hit_run.log`

Decision:
- **Rejected**

Failure reason:
- `work=256` candidate is not reproducibly stable in this environment on the real path.

## Real Hit Test (retained path)

Run:

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --quit-count 1 --work 128 --inverse-multiple 245760 --output .\runtime\perf_v074_hits.txt
```

Observed:
- `Validated hits: 1`
- `Saved hits: 1 -> .\runtime\perf_v074_hits.txt`

## Independent private->address Verification

Used independent verifier (`ecdsa + Keccak-256 + Base58Check`) on `runtime\perf_v074_hits.txt`:

- `verified=1`
- `mismatched=0`

## Conclusion

- New direction (`work=256` with fixed inverse-multiple) is rejected.
- Retained path remains `work=128 --inverse-multiple 245760`.
- Real hit generation and independent private-to-address derivation remain valid.
