# perf-v071 - disable tail5 packed compare fallback check (rejected)

Date: 2026-05-22

## Goal

Continue optimization iterations on the real usable TRON generation path only:

`--matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5`

Constraints:
- no benchmark-only retained behavior
- must keep real-hit validity and independent `private -> TRON address` verification

## Pre-run Guardrails

- Checked git state first (`git status --short --branch`).
- Checked residual `profanity` / `TronStudio` processes before each run.
- Only terminated matched repo-local `profanity.x64.exe` processes.

## New Direction

Current codebase already contains a tail5 packed compare branch on all-exact suffix5 path.
This round tested a new direction not yet recorded as failed:

- candidate: disable tail5 packed branch and force fallback compare path
- control switch: `PROFANITY_DISABLE_TAIL5_PACKED=1`

No benchmark-only behavior was kept; this is runtime-path A/B on the same real command.

## Build

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1
```

Result: build succeeded (`profanity.x64.exe` rebuilt).

## Reproducible Benchmark

Baseline (current default path):

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 5 --work 128 --inverse-multiple 237568
```

Observed:
- Initialization time: `23s`
- Final: `325.498 MH/s`

Candidate (disable tail5 packed path):

```powershell
set PROFANITY_DISABLE_TAIL5_PACKED=1 && C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 5 --work 128 --inverse-multiple 237568
```

Observed:
- exited during OpenCL program build stage
- log recorded `EXITCODE:-1`
- no valid `Final` throughput produced

Decision:
- **Rejected**

Failure reason:
- disabling tail5 packed branch is unstable in this environment (runtime exits before benchmark completes), so it cannot be adopted for the usable generation path.

## Real Hit Test

Run:

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --quit-count 3 --work 128 --inverse-multiple 237568 --output .\runtime\perf_v071_hits.txt
```

Observed:
- `Validated hits: 3`
- `Saved hits: 3 -> .\runtime\perf_v071_hits.txt`

## Independent private->address Verification

Used independent verifier (`ecdsa + Keccak-256 + Base58Check`) on the last 3 hits from `runtime\perf_v071_hits.txt`:

- `verified=3`
- `mismatched=0`

## Conclusion

- New direction (`disable tail5 packed`) is rejected and logged as failed.
- Retained path remains the existing default implementation (without disabling switch).
- Real-hit generation remains valid with independent private-to-address derivation.
