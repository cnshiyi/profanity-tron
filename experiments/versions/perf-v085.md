# perf-v085 - optional OpenCL CPU assist

Date: 2026-05-22

## Goal

Let an otherwise idle CPU participate in low-rate generation without moving the hot GPU candidate stream back to CPU memory and without implementing a separate unverified CPU wallet algorithm.

## Change

Added default-off CLI options:

```powershell
--cpu-assist
--cpu-assist-inverse-multiple 128
```

When enabled, the program looks for OpenCL CPU devices on the same platform as the selected GPU devices. Any found CPU device is added as a separate OpenCL device with a small per-device batch size.

Important behavior:

- GPU tuning is still based only on GPU devices.
- GPU devices keep their existing large batch size and local work size.
- CPU assist devices use a separate small `inverse-multiple`.
- Output labels distinguish `GPU0` and `CPU0`.
- If no compatible OpenCL CPU device is available, the program prints a message and safely continues GPU-only.

This reuses the existing OpenCL generation, matching, validation, and private-key derivation path. It does not introduce a separate CPU secp256k1 implementation.

## Compatibility Smoke Test

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --cpu-assist --cpu-assist-inverse-multiple 16 --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 10
```

Observed on this machine:

- `CPU assist requested, but no OpenCL CPU device was found on the selected GPU platform`
- Final: `355.212 MH/s`
- The run safely degraded to GPU-only.

## Real Hit Test

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --cpu-assist --cpu-assist-inverse-multiple 16 --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --quit-count 3 --output .\runtime\perf_v085_hits.txt
```

Observed:

- `Validated hits: 3`
- `Saved hits: 3`

Independent verification:

- `verified=3`
- `mismatched=0`

## Decision

Adopted as a default-off feature. This is not a measured speed win on the current RTX 3070 machine because no compatible OpenCL CPU device is exposed here, but it gives compatible systems a controlled low-rate CPU generation lane while preserving the real wallet-generation path.
