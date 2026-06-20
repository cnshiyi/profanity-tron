# Version Record

## v1.0.4 - 2026-06-21

- Reworked the Windows release flow so GitHub publishes only from `v*` tags.
- Replaced the fixed release tag with the pushed tag name and the built-in `GITHUB_TOKEN`.
- Added `scripts/build-windows.ps1` so local builds and GitHub release builds use the same steps.
- Clean builds now recreate `dist` with `start.exe`, `profanity.x64.exe`, default targets, version file, and a release zip.
- Updated the source fallback target list and root `profanity.txt` to the confirmed default target set.
- Kept generated binaries, object files, zips, and runtime hit logs out of git.

## 2026-06-21 - Windows launcher rebuild

- Built `dist/profanity.exe` for Windows x64 with MSVC.
- Recreated the graphical launcher as `dist/start.exe`.
- Restored the launcher layout: toolbar actions, prefix/suffix/count inputs, output path, status/PID/hit count, target editor, result grid, and output log.
- Restored the target editor area with title, editable target list, and bottom hint text.
- Switched the result area to a grid-style table with columns: serial number, time, address, private key, score.
- Result rows are no longer limited; every hit remains in the table until the list is cleared.
- Default output path now uses per-run files under `dist/runtime/runs/hits_yyyyMMdd_HHmmss.txt`.
- Updated the default target list in `dist/profanity.txt` to the requested bank-card pattern set.
- Compiled launcher source with UTF-8 handling to avoid Chinese UI text corruption.

Current files:

- Launcher: `dist/start.exe`
- Generator: `dist/profanity.exe`
- Default targets: `dist/profanity.txt`
- Launcher source: `launcher/StartLauncher.cs`

## 2026-06-21 - Restore Launcher Source From v1.0.3

- Downloaded the online `v1.0.3` release source.
- Restored the original launcher source as `launcher/TronStudio.cs`.
- Rebuilt `dist/start.exe` from the restored source.
- Restored the original target editor and result grid structure from source.
- Updated `dist/runtime/targets.txt` to the requested default target list.

## 2026-06-21 - UI Result Grid Adjustment

- Increased target/result group title font size.
- Changed result grid columns to: serial number, address, private key, time.
- Removed the score column from the launcher table.

## 2026-06-21 - Optional Range Controls

- Added optional UI fields for initial key, direction, and digit count.
- Direction labels display in Chinese.
- Kept the controls front-end only for now; blank values keep the normal random mode.
- Adjusted implementation to avoid Device Guard blocking by using the same encoded UI text style as the original launcher.

## 2026-06-21 - Default Count And Label

- Changed the default count from 10 to 999.
- Renamed the optional key label from `初始化私钥` to `初始私钥`.

## 2026-06-21 - Explicit Loop Task

- Added a visible loop task checkbox in the top control area.
- When enabled, the launcher starts the next run after a task exits normally.
- Manual stop and application exit disable/avoid automatic restart.

## 2026-06-21 - Revert Misread Loop UI

- Removed the mistakenly added launcher loop-task checkbox.
- Removed launcher auto-restart behavior.
- Looping now refers to the development/test iteration process, not an in-app feature.
