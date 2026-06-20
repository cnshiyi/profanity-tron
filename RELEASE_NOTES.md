# v1.0.5

## Release Content

- Windows launcher: `start.exe`
- Native OpenCL generator: `shiyi.exe`
- Runtime OpenCL kernels: `kernels/*.cl`
- Default target files: `profanity.txt` and `runtime/targets.txt`
- Launcher helper: `start.bat`
- Package metadata: `README.md` and `VERSION.txt`

## Changes

- Restored the range-capable native backend and packaged runtime kernel sources so the release works after moving to another computer.
- Connected the launcher fields for initial private key, direction, and digit count to native range mode.
- Limited fixed-digit mode to 1-16 hex digits and added launcher guidance.
- Added serial local benchmark tooling with residual process cleanup.
- Reset speed sampling after initialization and reduced range-mode batch size for small windows.
- Replaced native CLI help with ASCII text to reduce Windows Application Control false positives.
- Restored the packaged generator name to `shiyi.exe` and kept legacy generator names only in cleanup checks.
- Removed obsolete static curl/network link dependencies from the native generator build; this fixed the Smart App Control block seen on the new executable.

## Verification

- Clean Windows build from source.
- `shiyi.exe --help` runs without Windows Application Control blocking.
- Release zip contains `kernels/` and `runtime/` directories.
- Launcher smoke test: start and close.
- Benchmark residual checks: no leftover `shiyi`, old `profanity*`, `TronStudio`, or `start` processes after runs.
- Current measured baseline on RTX 3070:
  - Random single-target: 383.894 MH/s.
  - Default multi-target: 382.824 MH/s.
  - Explicit initial-key range: 419.475 MH/s.

## Known Status

- The 400 MH/s target is reached for the explicit initial-key range test, but random/default modes are still below target. This release records the restored baseline and next optimization point.
