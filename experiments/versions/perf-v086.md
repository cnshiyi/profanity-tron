# perf-v086 - rejected host CPU thread assist

Date: 2026-05-22

## Goal

Test whether an otherwise idle host CPU can run a separate low-rate generation lane while the GPU keeps the main OpenCL search path.

## Candidate

Implemented a default-off `--cpu-thread-assist` prototype with `--cpu-thread-rate`. The prototype used host-side secp256k1 scalar multiplication based on the existing precompute table, Keccak-256 public-key hashing, existing TRON Base58Check formatting, and the same real matching/output validation path.

The candidate also had a startup self-check for private key `1`:

- hash20: `7e5f4552091a69125d5dfcb7b8c2659029395bdf`
- address: `TMVQGm1qAQYVdetCeGRRkTWYYrLXuHK2HC`

## 60 Second Benchmark

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --cpu-thread-assist --cpu-thread-rate 16 --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 60
```

Observed:

- Final: `354.388 MH/s`
- GPU0: `354.388 MH/s`
- Initialization: `23 seconds`

Comparison:

- Retained `perf-v083`: `355.734 MH/s`
- Delta: `-0.38%`

## Real Hit Test

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --cpu-thread-assist --cpu-thread-rate 16 --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --quit-count 3 --output .\runtime\perf_v086_hits.txt
```

Observed:

- `Validated hits: 3`
- `Saved hits: 3`

Independent verification:

- `verified=3`
- `mismatched=0`

## Decision

Rejected and reverted. The prototype was functionally correct, but it reduced measured GPU throughput and added a large host-side wallet derivation implementation. Keep `perf-v083` as the retained fastest real generation path and keep `perf-v085` OpenCL CPU assist as the default-off compatibility path.

