# v1.0.18

## 中文

### 发布内容

- Windows 图形启动器：`start.exe`
- 原生 OpenCL 生成器：`shiyi.exe`
- 发布资产文件：`shiyi-v1.0.18.zip`
- 运行时 OpenCL 内核：`kernels/*.cl`
- 默认目标文件：`profanity.txt` 和 `runtime/targets.txt`
- 启动辅助脚本：`start.bat`
- 包说明和版本信息：`README.md`、`VERSION.txt`

### 本次更新

- 修复有限 range 模式重复扫描问题；range 候选现在按 `round * deviceSize + id` 推进，覆盖完当前窗口后退出。
- 启动器在初始私钥留空但填写位数或方向时，会生成随机前缀，并按位数/方向把低位窗口对齐到完整起点。
- 方向留空时只随机一次；自动续跑沿用同一方向，避免每轮重新随机导致反向或重复。
- 停止任务会关闭自动续跑，避免用户停止后又进入下一窗口。
- 增加 `scripts/test-range-planner.ps1`，长期验证 16 位向上进位、随机位数对齐、方向续用、停止禁用自动续跑，以及小于 8/等于 8/大于 8 位数分支。

### 验证

- `scripts/test-range-planner.ps1` 通过。
- `scripts/build-windows.ps1 -Version v1.0.18` 构建通过。
- `shiyi-v1.0.18.zip` 包内包含 `shiyi.exe`、`start.exe`，不包含旧名 `profanity.x64.exe`。
- `dist/profanity.txt` 和 `dist/runtime/targets.txt` 默认目标一致，均为合法 `1-9 + A` 十行。
- 构建/检查后未发现残留 `shiyi`、旧 `profanity*`、`TronStudio` 或 `start` 进程。
- 本机直接运行新编译 `dist/shiyi.exe` 被 Application Control 策略拦截；因此本轮不声明 1 分钟性能数据。

### 性能状态

本版本修复 range 行为和启动器随机位数/方向逻辑，是继续优化 400 MH/s 目标前的正确性修复。400 MH/s 目标尚未达成。

## English

### Release Content

- Windows launcher: `start.exe`
- Native OpenCL generator: `shiyi.exe`
- Release asset file: `shiyi-v1.0.18.zip`
- Runtime OpenCL kernels: `kernels/*.cl`
- Default target files: `profanity.txt` and `runtime/targets.txt`
- Launcher helper: `start.bat`
- Package metadata: `README.md` and `VERSION.txt`

### Changes

- Fixed duplicate scanning in finite range mode; range candidates now advance by `round * deviceSize + id` and stop after the current window is covered.
- When the launcher has an empty initial key but a digit count or direction is set, it now generates a random prefix and aligns the low digit window to a full start boundary.
- Blank direction is randomized once at task start; auto-continued windows keep that direction instead of re-randomizing every round.
- Stopping a task disables auto-continue so the launcher cannot restart into the next window after the user stops it.
- Added `scripts/test-range-planner.ps1` to persistently verify 16-digit upward carry, random digit alignment, direction reuse, stop disabling auto-continue, and explicit `<8`, `=8`, and `>8` digit branches.

### Verification

- `scripts/test-range-planner.ps1` passed.
- `scripts/build-windows.ps1 -Version v1.0.18` built successfully.
- `shiyi-v1.0.18.zip` contains `shiyi.exe` and `start.exe`, with no legacy `profanity.x64.exe`.
- `dist/profanity.txt` and `dist/runtime/targets.txt` match and contain the legal 10-line `1-9 + A` default target set.
- No leftover `shiyi`, old `profanity*`, `TronStudio`, or `start` processes were found after build and checks.
- Direct execution of the freshly built `dist/shiyi.exe` is blocked by Application Control on this machine, so this release does not claim 1-minute performance data.

### Performance Status

This release fixes range behavior and launcher random digit/direction handling before continuing the 400 MH/s optimization loop. The 400 MH/s target has not been reached yet.
