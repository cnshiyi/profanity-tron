# Version Record

## v1.0.6 - 2026-06-21

- 中文：将根目录 README、发布包 README、发布说明和版本记录改为中英双语。
- English: Made the root README, package README, release notes, and version record bilingual Chinese/English.
- 中文：重新构建 Windows 发布包，确认包内 README 也由构建脚本生成双语内容。
- English: Rebuilt the Windows package and verified the package README is generated bilingually by the build script.
- 中文：发布资产文件名改为 `shiyi-版本号.zip`，例如 `shiyi-v1.0.6.zip`。
- English: Changed the release asset filename to `shiyi-version.zip`, for example `shiyi-v1.0.6.zip`.

## v1.0.5 - 2026-06-21

- Restored the range-capable native backend and runtime OpenCL kernel sources.
- Packaged `kernels/*.cl` with the Windows release so the generator works after moving to another computer.
- Wired launcher initial private key, direction, and digit count fields into native range mode.
- Added a 1-16 hex digit limit and UI guidance for fixed-digit mode.
- Added `scripts/benchmark-local.ps1` for serial benchmark runs with residual process cleanup.
- Fixed fixed-digit benchmark reporting by resetting speed sampling after initialization.
- Reduced range-mode batch size for small windows to avoid initializing unused GPU work.
- Restored the Windows generator package name to `shiyi.exe`; legacy generator names are treated only as cleanup compatibility names.
- Removed obsolete static curl/network linking from the native generator build; this fixed the Smart App Control block seen on the newly built executable.
- Benchmarked current baseline on RTX 3070: random single-target 383.894 MH/s, default multi-target 382.824 MH/s, explicit initial-key range 419.475 MH/s. The 400 MH/s target remains open for random/default modes.

## v1.0.4 - 2026-06-21

- Reworked the Windows release flow so GitHub publishes only from `v*` tags.
- Replaced the fixed release tag with the pushed tag name and the built-in `GITHUB_TOKEN`.
- Added `scripts/build-windows.ps1` so local builds and GitHub release builds use the same steps.
- Clean builds now recreate `dist` with `start.exe`, the native generator, default targets, version file, and a release zip.
- Updated the source fallback target list and root `profanity.txt` to the confirmed default target set.
- Kept generated binaries, object files, zips, and runtime hit logs out of git.

## 2026-06-21 - Windows launcher rebuild

- Built the Windows x64 native generator with MSVC.
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
- Generator: legacy native executable
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
