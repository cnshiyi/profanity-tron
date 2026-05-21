# perf-v070 - exact suffix packed-tail key prefilter (rejected)

Date: 2026-05-21

## Goal

Continue optimizing the real usable generation path only:

`--matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5`

Constraints:
- No benchmark-only behavior kept in retained code.
- Must keep real hit validity and independent `private -> TRON address` derivation.

## New Direction

Try a new direction not previously recorded as failed:
- Build a packed tail key for all-exact multi-target suffix path (`suffix2-10`).
- Pre-compute unique target keys on host and push to kernel.
- In kernel, after existing `suffixTail2Allowed` gate, pack computed tail indices and early-return if key not in precomputed set.

This direction was tested with an A/B gate (`PROFANITY_DISABLE_PACKED_TAIL`) for reproducible comparison, then fully reverted after decision.

## Reproducible Benchmark

Baseline (packed-tail disabled, same binary):

```powershell
$env:PROFANITY_DISABLE_PACKED_TAIL='1'
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 5 --work 128 --inverse-multiple 237568
Remove-Item Env:PROFANITY_DISABLE_PACKED_TAIL
```

- Final: `354.793 MH/s`
- Initialization: `22s`

Candidate (packed-tail enabled):

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 5 --work 128 --inverse-multiple 237568
```

- Final: `354.596 MH/s`
- Initialization: `22s`

Result:
- Delta: `-0.06%`
- Decision: **not adopted**.

Failure reason:
- Packed-tail key prefilter adds overhead and regresses slightly on this real path.

## Real Hit Test (retained path)

Run:

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --quit-count 3 --work 128 --inverse-multiple 237568 --output .\runtime\perf_v070_revert_hits.txt
```

Result:
- `Validated hits: 3`
- `Saved hits: 3`

## Independent private->address Verification

Used independent verifier (`ecdsa + Keccak-256 + Base58Check`) on `runtime\perf_v070_revert_hits.txt`:

- `verified=3`
- `mismatched=0`

## Conclusion

- New direction (`packed tail key` prefilter) is rejected.
- Retained implementation remains the previously adopted `perf-v065` logic path.
- Real-hit generation and independent private-to-address derivation remain valid.
