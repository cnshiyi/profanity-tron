# v1.0.16

## 中文

### 发布内容

- Windows 图形启动器：`start.exe`
- 原生 OpenCL 生成器：`shiyi.exe`
- 发布资产文件：`shiyi-v1.0.16.zip`
- 运行时 OpenCL 内核：`kernels/*.cl`
- 默认目标文件：`profanity.txt` 和 `runtime/targets.txt`
- 启动辅助脚本：`start.bat`
- 包说明和版本信息：`README.md`、`VERSION.txt`

### 本次更新

- 增加 `scripts/build-windows.ps1 -DebugNative`，用于构建带 `PROFANITY_DEBUG` 的 profiling 版本。
- Debug profiling 输出现在包含 inverse、iterate、score 三段 OpenCL kernel 时间，便于后续针对主热点优化。
- 默认发布构建不启用 debug profiling，不改变普通用户运行路径。
- 继续保留 v1.0.15 的默认目标修复、重复计数修复和 `--quit-count` 唯一命中计数语义。

### 验证

- 线上 v1.0.15 发行包 1 分钟串行基线：随机后 8 位 383.908 MH/s。
- 线上 v1.0.15 发行包 1 分钟串行基线：后 16 位向上 range 383.174 MH/s。
- Debug profiling 样本：iterate 约 66.1 ms，inverse 约 38.5 ms，score 约 24.7 ms。
- 拒绝无稳定收益实验：benchmark 跳过结果清零、SHA256 局部清零、OpenCL build options、`negativeGx` 常量区实验。
- 构建/测试后未发现残留 `shiyi`、旧 `profanity*`、`TronStudio` 或 `start` 进程。

### 性能状态

400 MH/s 目标尚未达成。本版本是 profiling 和证据补强版本，定位到下一步应优先优化 iterate/inverse 主内核，而不是继续做参数微调。

## English

### Release Content

- Windows launcher: `start.exe`
- Native OpenCL generator: `shiyi.exe`
- Release asset file: `shiyi-v1.0.16.zip`
- Runtime OpenCL kernels: `kernels/*.cl`
- Default target files: `profanity.txt` and `runtime/targets.txt`
- Launcher helper: `start.bat`
- Package metadata: `README.md` and `VERSION.txt`

### Changes

- Added `scripts/build-windows.ps1 -DebugNative` for building a `PROFANITY_DEBUG` profiling binary.
- Debug profiling now prints inverse, iterate, and score OpenCL kernel timings to guide the next hotspot optimization pass.
- The default release build does not enable debug profiling and does not change the normal user runtime path.
- Keeps the v1.0.15 default-target fix, duplicate-counting fix, and unique-hit `--quit-count` behavior.

### Verification

- Online v1.0.15 release package serial 1-minute baseline: random last-8 reached 383.908 MH/s.
- Online v1.0.15 release package serial 1-minute baseline: last-16 upward range reached 383.174 MH/s.
- Debug profiling sample: iterate about 66.1 ms, inverse about 38.5 ms, and score about 24.7 ms.
- Rejected experiments without stable gains: skipping result clearing in benchmark mode, partial SHA256 clearing, OpenCL build options, and the `negativeGx` constant-memory experiment.
- No leftover `shiyi`, old `profanity*`, `TronStudio`, or `start` processes were found after build and tests.

### Performance Status

The 400 MH/s target has not been reached yet. This is a profiling and evidence release; it shows the next pass should prioritize the iterate/inverse main kernels instead of parameter micro-tuning.
