# v1.0.19

## 中文

### 发布内容

- Windows 图形启动器：`start.exe`
- 原生 OpenCL 生成器：`shiyi.exe`
- 发布资产文件：`shiyi-v1.0.19.zip`
- 运行时 OpenCL 内核：`kernels/*.cl`
- 默认目标文件：`profanity.txt` 和 `runtime/targets.txt`
- 启动辅助脚本：`start.bat`
- 包说明和版本信息：`README.md`、`VERSION.txt`

### 本次更新

- 修正 v1.0.18 的 range stride 风险：CPU 端私钥重建恢复为与 GPU `profanity_iterate` 一致的 `foundId << shift` 加每轮 `+G` 公式。
- 保留启动器随机位数修复：初始私钥为空但填写位数/方向时，随机前缀会按方向对齐完整低位窗口。
- 保留方向修复：方向留空只在任务开始时随机一次，自动续跑沿用同一方向。
- 保留停止修复：停止任务会关闭自动续跑，避免用户停止后又进入下一窗口。
- `scripts/test-range-planner.ps1` 增加防回归检查，避免未同步修改 OpenCL 迭代步长时再次使用 `round * stride` 重建私钥。

### 验证

- `scripts/test-range-planner.ps1` 通过。
- `scripts/build-windows.ps1 -Version v1.0.19` 构建通过。
- `shiyi-v1.0.19.zip` 包内包含 `shiyi.exe`、`start.exe`，不包含旧名 `profanity.x64.exe`。
- `dist/profanity.txt` 和 `dist/runtime/targets.txt` 默认目标一致，均为合法 `1-9 + A` 十行。
- 构建/检查后未发现残留 `shiyi`、旧 `profanity*`、`TronStudio` 或 `start` 进程。
- 本机直接运行新版本 `shiyi.exe` 仍被 Application Control 策略拦截；因此本轮不声明 1 分钟性能数据。

### 性能状态

本版本是 v1.0.18 的正确性修正，避免错误的私钥重建公式进入后续优化。400 MH/s 目标尚未达成。

## English

### Release Content

- Windows launcher: `start.exe`
- Native OpenCL generator: `shiyi.exe`
- Release asset file: `shiyi-v1.0.19.zip`
- Runtime OpenCL kernels: `kernels/*.cl`
- Default target files: `profanity.txt` and `runtime/targets.txt`
- Launcher helper: `start.bat`
- Package metadata: `README.md` and `VERSION.txt`

### Changes

- Corrected the v1.0.18 range stride risk: CPU-side private-key reconstruction is restored to the GPU-compatible `foundId << shift` plus per-round `+G` formula.
- Kept the launcher random digit fix: when initial key is blank but digit count or direction is set, the random prefix is aligned to a full low digit window based on direction.
- Kept the direction fix: blank direction is randomized once at task start and reused by auto-continued windows.
- Kept the stop fix: stopping a task disables auto-continue so it cannot launch the next window after the user stops it.
- `scripts/test-range-planner.ps1` now prevents regressing to `round * stride` private-key reconstruction unless the OpenCL iterator step is changed consistently.

### Verification

- `scripts/test-range-planner.ps1` passed.
- `scripts/build-windows.ps1 -Version v1.0.19` built successfully.
- `shiyi-v1.0.19.zip` contains `shiyi.exe` and `start.exe`, with no legacy `profanity.x64.exe`.
- `dist/profanity.txt` and `dist/runtime/targets.txt` match and contain the legal 10-line `1-9 + A` default target set.
- No leftover `shiyi`, old `profanity*`, `TronStudio`, or `start` processes were found after build and checks.
- Direct execution of the new `shiyi.exe` is still blocked by Application Control on this machine, so this release does not claim 1-minute performance data.

### Performance Status

This release is a correctness correction for v1.0.18, preventing an incorrect private-key reconstruction formula from entering later optimization work. The 400 MH/s target has not been reached yet.
