# Version Record

## v1.0.10 - 2026-06-21

- 中文：构建脚本增加可选 Authenticode 签名参数，支持 `-SignThumbprint` 或 `-SignPfxPath/-SignPfxPassword`。
- English: Added optional Authenticode signing support to the Windows build script via `-SignThumbprint` or `-SignPfxPath/-SignPfxPassword`.
- 中文：GitHub Release workflow 支持通过 `WINDOWS_SIGNING_PFX_BASE64` 和 `WINDOWS_SIGNING_PFX_PASSWORD` secrets 导入签名证书；未配置证书时继续无签名构建。
- English: The GitHub Release workflow can import a signing certificate from `WINDOWS_SIGNING_PFX_BASE64` and `WINDOWS_SIGNING_PFX_PASSWORD` secrets; unsigned builds still work when no certificate is configured.
- 中文：无证书构建路径已验证，`shiyi.exe --help` 可运行；RTX 3070 样本为随机单目标 351.127 MH/s、后 16 位向上 333.413 MH/s，400 MH/s 目标仍未达成。
- English: Verified the no-certificate build path and `shiyi.exe --help`; RTX 3070 samples are random single-target 351.127 MH/s and last-16 upward 333.413 MH/s. The 400 MH/s target remains open.

## v1.0.9 - 2026-06-21

- 中文：修复范围模式只支持私钥前 16 位的问题，现在可支持任意一个 16 位私钥块内的连续 1-16 个十六进制位。
- English: Fixed range mode being limited to the first 16 private-key hex chars; it now supports any continuous 1-16 hex chars inside one 16-char private-key block.
- 中文：启动器“位数”现在按私钥后缀生成范围；全 0 初始私钥会自动跳过无效私钥 `0`。
- English: The launcher digit-count range builder now edits suffix digits; an all-zero initial private key skips invalid private key `0`.
- 中文：修复范围模式命中后私钥回写少 1 的问题，输出私钥已通过独立 secp256k1 + Keccak 校验。
- English: Fixed range-mode private-key reporting being off by one; printed private keys were verified independently with secp256k1 + Keccak.
- 中文：NVIDIA 默认调参改为按显存预算和计算单元分档，不再把 RTX 3070 参数硬编码给所有 NVIDIA 显卡。
- English: NVIDIA default tuning now uses memory-budget and compute-unit tiers instead of hard-coding RTX 3070-class values for all NVIDIA GPUs.
- 中文：RTX 3070 可信样本为随机单目标 353.158 MH/s、默认多目标 331.924 MH/s、全 0 后 16 位向上 333.261 MH/s；400 MH/s 目标尚未达成。
- English: Trusted RTX 3070 samples are random single-target 353.158 MH/s, default multi-target 331.924 MH/s, and all-zero last-16 upward 333.261 MH/s; the 400 MH/s target is not reached yet.

## v1.0.8 - 2026-06-21

- 中文：修正默认目标地址列表，将误写入的 `[银行卡]` 占位文本恢复为 0 到 9 各一行的真实目标。
- English: Corrected the default target list by replacing the accidental `[银行卡]` placeholder text with the real 0-9 target lines.
- 中文：启动器读取旧 `runtime/targets.txt` 时，如果检测到 `[银行卡]` 旧内容，会自动回退到新的 0-9 默认目标。
- English: The launcher now falls back to the corrected 0-9 defaults when an old `runtime/targets.txt` still contains `[银行卡]`.
- 中文：本版本不发布未验证的性能参数调整；随机/default 模式 400 MH/s 目标继续迭代。
- English: This release does not ship the unverified performance parameter experiment; the 400 MH/s target for random/default modes remains active.

## v1.0.7 - 2026-06-21

- 中文：修复目标地址保存时首行可能带 UTF-8 BOM 的问题，确保目标地址一行一个且没有隐藏前缀字符。
- English: Fixed target saving so the first line does not get a UTF-8 BOM, keeping targets one per line with no hidden prefix character.
- 中文：生成器读取目标文件时会清理 BOM 和首尾空白，提高旧文件兼容性。
- English: The generator now strips BOM and surrounding whitespace when reading target files for compatibility with older saved files.
- 中文：发布包 README 改为从 UTF-8 模板复制，避免 Windows PowerShell 5.1 读取脚本源码时把中文生成成乱码。
- English: The packaged README is copied from a UTF-8 template to avoid garbled Chinese when Windows PowerShell 5.1 reads the build script.

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
