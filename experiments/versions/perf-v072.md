# perf-v072 - inverse-multiple neighborhood retune (rejected)

Date: 2026-05-22

## Goal

Continue optimizing only the real usable TRON generation path:

`--matching .\\runtime\\multi10_targets.txt --prefix-count 0 --suffix-count 5`

Constraints kept:
- no benchmark-only retained behavior
- keep real-hit validity and independent `private -> TRON address` derivation

## Pre-run Checks

- Checked `git status --short --branch` first.
- Checked residual `profanity` / `TronStudio` processes first.
- Only terminated repo-local `profanity.x64.exe` when cleanup was needed.

## New Direction

Try a new direction not previously recorded as failed:
- keep current retained path (`perf-v069`, `inverse-multiple=245760`) as baseline
- test nearby scheduling value `inverse-multiple=241664` on the same real suffix5 multi-target path

No kernel behavior change was retained; this is parameter-only A/B on the real generation path.

## Reproducible Benchmark

Baseline (r1, r2):

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\\runtime\\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 5 --work 128 --inverse-multiple 245760
```

- r1: `356.084 MH/s`, init `23s` (`runtime/perf_v070_pair_base_r1.run.txt`)
- r2: `355.788 MH/s`, init `23s` (`runtime/perf_v070_pair_base_r2.run.txt`)
- avg: `355.936 MH/s`

Candidate (r1, r2):

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\\runtime\\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 5 --work 128 --inverse-multiple 241664
```

- r1: `354.986 MH/s`, init `22s` (`runtime/perf_v070_pair_cand_r1.run.txt`)
- r2: `355.088 MH/s`, init `23s` (`runtime/perf_v070_pair_cand_r2b.run.txt`)
- avg: `355.037 MH/s`

Result:
- delta vs baseline avg: `-0.25%`
- decision: **not adopted**

Failure reason:
- neighboring `inverse-multiple=241664` is consistently below retained `245760` on this real path.

## Real Hit Test (retained path)

Run:

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\\runtime\\multi10_targets.txt --prefix-count 0 --suffix-count 5 --quit-count 3 --work 128 --inverse-multiple 245760 --output .\\runtime\\perf_v072_revert_hits.txt
```

Result (`runtime/perf_v072_revert_hits.run.txt`):
- `Validated hits: 3`
- `Saved hits: 3 -> .\\runtime\\perf_v072_revert_hits.txt`

## Independent private->address Verification

Used independent verifier (`ecdsa + Keccak-256 + Base58Check`) on `runtime\\perf_v072_revert_hits.txt`:

- `verified=3`
- `mismatched=0`

## Conclusion

- New direction (`inverse-multiple=241664` neighborhood retune) rejected.
- Retained path remains `inverse-multiple=245760`.
- Real hit generation and private-to-address independent derivation are valid.
