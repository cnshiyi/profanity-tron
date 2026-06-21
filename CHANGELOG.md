# Version Record

## v1.0.16 - 2026-06-21

- 中文：为 Windows 构建脚本增加 `-DebugNative` 开关，用于编译带 `PROFANITY_DEBUG` 的 profiling 版本，不影响默认发布构建。
- English: Added a `-DebugNative` switch to the Windows build script for compiling a `PROFANITY_DEBUG` profiling build without changing the default release build.
- 中文：debug profiling 现在同时输出 inverse、iterate 和 score 三段 OpenCL kernel 时间；RTX 3070 样本显示 iterate 约 66.1 ms、inverse 约 38.5 ms、score 约 24.7 ms。
- English: Debug profiling now prints inverse, iterate, and score OpenCL kernel timings; RTX 3070 samples measured roughly 66.1 ms for iterate, 38.5 ms for inverse, and 24.7 ms for score.
- 中文：补做线上 v1.0.15 发行包 1 分钟串行基线：随机后 8 位 383.908 MH/s，后 16 位向上 range 383.174 MH/s，均未达到 400 MH/s。
- English: Added serial 1-minute reference baselines from the online v1.0.15 release package: random last-8 reached 383.908 MH/s, and last-16 upward range reached 383.174 MH/s; neither reached 400 MH/s.
- 中文：本轮拒绝无稳定收益实验：benchmark 跳过结果清零、SHA256 局部清零、OpenCL build options、`negativeGx` 常量区实验均未带来 60 秒稳定提升。
- English: Rejected experiments without stable gains: skipping result clearing in benchmark mode, partial SHA256 clearing, OpenCL build options, and the `negativeGx` constant-memory experiment did not produce stable 60-second improvement.

## v1.0.15 - 2026-06-21

- 中文：修复默认目标地址，移除 TRON/Base58 地址不支持的 `0` 目标，改为 `1-9 + A` 的 10 行默认目标；源码、`dist/profanity.txt` 和 `dist/runtime/targets.txt` 已保持一致。
- English: Fixed the default targets by removing the `0` target that TRON/Base58 addresses cannot contain, replacing the default 10-line set with `1-9 + A`; source, `dist/profanity.txt`, and `dist/runtime/targets.txt` now match.
- 中文：修复命中结果重复计数：GPU 结果缓冲每轮清空，单批最多回传 120 个命中槽，CPU 按私钥去重后再统计 `--quit-count` 和保存结果。
- English: Fixed duplicate hit counting: the GPU result buffer is cleared every round, each batch can return up to 120 hit slots, and the CPU deduplicates by private key before counting `--quit-count` and saving hits.
- 中文：`--quit-count` 现在按“唯一验证命中条数”退出，不再误用为 GPU 分数阈值；默认数量 999 不会再因 `cl_uchar` 溢出而变成错误阈值。
- English: `--quit-count` now exits by unique validated hit count instead of being misused as a GPU score threshold; the default count of 999 no longer overflows a `cl_uchar` into a wrong threshold.
- 中文：修复 `scripts/benchmark-local.ps1` 的超时路径；现在 benchmark 超时也会保存 stdout/stderr，并返回结构化 `TIMEOUT` 结果，便于定位长跑卡住位置。
- English: Fixed the timeout path in `scripts/benchmark-local.ps1`; timed-out benchmark runs now preserve stdout/stderr and return a structured `TIMEOUT` result for diagnosing long-run stalls.
- 中文：本地新编译 `shiyi.exe` 被 Smart App Control / Code Integrity 策略拦截，日志显示未满足企业签名级别要求；因此本轮无法在本机完成 1 分钟性能实测，400 MH/s 目标不作达成声明。
- English: The freshly local-built `shiyi.exe` was blocked by Smart App Control / Code Integrity because it did not meet the enterprise signing level; therefore this round could not complete local 1-minute performance verification and does not claim the 400 MH/s target.
- 中文：本轮拒绝两个 kernel 实验：拆分模乘归约分支降至 310.538 MH/s，`modHigher` 常量区实验为 357.988 MH/s，均不发布为性能优化。
- English: Rejected two kernel experiments this round: splitting the modular-multiply reduction branch dropped to 310.538 MH/s, and the `modHigher` constant-memory experiment measured 357.988 MH/s; neither is shipped as a performance optimization.
- 中文：官方 v1.0.14 发布包 20 秒基线仍作为参考：随机后 8 位为 357.353 MH/s，全 0 初始私钥后 16 位向上为 356.712 MH/s。
- English: The official v1.0.14 package 20-second baselines remain reference data: random last-8 reached 357.353 MH/s, and all-zero last-16 upward range reached 356.712 MH/s.

## v1.0.14 - 2026-06-21

- 中文：为手动 `--inverse-multiple` 增加 60% GPU 显存预算校验，避免输入过大批量导致初始化阶段异常退出。
- English: Added a 60% GPU memory-budget check for manual `--inverse-multiple` values to avoid initialization crashes from overly large batches.
- 中文：`--inverse-multiple 327680` 在 RTX 3070 上现在返回明确错误并建议使用安全上限或自动调参；默认随机和后 16 位 range 均验证可运行。
- English: `--inverse-multiple 327680` now returns a clear error on RTX 3070 and suggests the safe cap or automatic tuning; default random and last-16 range modes were verified.
- 中文：本版本是稳定性和说明补强，400 MH/s 目标仍未达成。
- English: This is a stability and documentation pass; the 400 MH/s target remains open.

## v1.0.13 - 2026-06-21

- 中文：多目标后缀 2-12 位路径增加统一后 2 位 GPU 粗筛；合法目标会登记可命中的后两位组合，含 Base58 非法字符的目标不会拖慢完整尾部比较。
- English: Added a unified last-2-character GPU coarse filter for multi-target suffix matching from 2 to 12 chars; valid targets register possible tail pairs while targets containing non-Base58 characters no longer force full-tail comparisons.
- 中文：默认 10 目标列表含 `0` 的同口径 8 秒测试从 328.545 MH/s 提升到 348.527 MH/s；合法 10 目标为 354.192 MH/s，单目标为 356.208 MH/s，后 16 位向上 range 10 秒为 356.719 MH/s。
- English: Same-method 8-second default 10-target testing with `0` entries improved from 328.545 MH/s to 348.527 MH/s; valid 10-target testing reached 354.192 MH/s, single-target reached 356.208 MH/s, and last-16 upward range mode reached 356.719 MH/s over 10 seconds.
- 中文：正确性验证仍命中 `TJSWuhAyiDQyb7E8ne1AitDV62H8HXRnvU`，私钥 `...0000000002fcf6a3`；400 MH/s 目标仍未达成。
- English: Correctness verification still hit `TJSWuhAyiDQyb7E8ne1AitDV62H8HXRnvU` with private key `...0000000002fcf6a3`; the 400 MH/s target remains open.

## v1.0.12 - 2026-06-21

- 中文：单目标后缀 3-12 位匹配增加后 2 位 GPU 粗筛；绝大多数候选在算完整 Base58 尾部前提前返回。
- English: Added a last-2-character GPU coarse filter for single-target suffix matching from 3 to 12 chars, so most candidates return before computing the full Base58 tail.
- 中文：RTX 3070 同口径 10 秒测试中，随机后 8 位从 338.995 MH/s 提升到 354.943 MH/s，固定后 16 位向上达到 357.349 MH/s；正确性命中私钥仍为 `...0000000002fcf6a3`。
- English: In same-method 10-second RTX 3070 tests, random last-8 improved from 338.995 MH/s to 354.943 MH/s, and last-16 upward range mode reached 357.349 MH/s; the correctness hit still reports private key `...0000000002fcf6a3`.
- 中文：`--inverse-size` 扫描未突破当前上限；400 MH/s 目标仍未达成，下一轮需继续优化 inverse/iterate 主内核。
- English: The `--inverse-size` scan did not break through the current ceiling; the 400 MH/s target remains open and the next pass needs deeper inverse/iterate kernel optimization.

## v1.0.11 - 2026-06-21

- 中文：根据 RTX 3070 串行矩阵测试，将 64 CU 以下 NVIDIA 默认 work 调为 32，并将该档 inverse-multiple 调为 196608。
- English: Based on RTX 3070 serial matrix tests, changed the default NVIDIA work size below 64 compute units to 32 and the tier inverse-multiple to 196608.
- 中文：v1.0.11 正式构建后默认参数复测结果为随机单目标 338.415 MH/s、全 0 后 16 位向上 338.758 MH/s，默认输出确认为 `work = 32`、`inverse-multiple = 196608`。
- English: After the official v1.0.11 build, default-parameter retests measured 338.415 MH/s for random single-target and 338.758 MH/s for all-zero last-16 upward; default output confirms `work = 32` and `inverse-multiple = 196608`.
- 中文：构建与测试后未发现残留 `shiyi`、旧 `profanity*`、`TronStudio` 或 `start` 进程；400 MH/s 目标仍未达成。
- English: No leftover `shiyi`, old `profanity*`, `TronStudio`, or `start` processes were found after build and tests; the 400 MH/s target remains open.

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

- 中文：修正默认目标地址列表，将误写入的旧占位文本恢复为数字目标。
- English: Corrected the default target list by replacing the accidental legacy placeholder text with digit target lines.
- 中文：启动器读取旧 `runtime/targets.txt` 时，如果检测到旧占位内容，会自动回退到新的默认目标。
- English: The launcher now falls back to the corrected defaults when an old `runtime/targets.txt` still contains legacy placeholder content.
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
