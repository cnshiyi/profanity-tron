# v1.0.14

## 中文

### 发布内容

- Windows 图形启动器：`start.exe`
- 原生 OpenCL 生成器：`shiyi.exe`
- 发布资产文件：`shiyi-v1.0.14.zip`
- 运行时 OpenCL 内核：`kernels/*.cl`
- 默认目标文件：`profanity.txt` 和 `runtime/targets.txt`
- 启动辅助脚本：`start.bat`
- 包说明和版本信息：`README.md`、`VERSION.txt`

### 本次更新

- 手动 `--inverse-multiple` 增加 60% GPU 显存预算校验。
- 过大的手动批量现在会返回明确错误，而不是在初始化阶段异常退出。
- `--help` 增加 `--inverse-multiple` 安全说明。
- 保留 v1.0.13 的多目标后缀粗筛、v1.0.12 的单目标后缀粗筛和可选代码签名构建通道。

### 验证

- 参数矩阵测试均串行运行，避免 benchmark 脚本互相清理进程造成假结果。
- `v1.0.14` 正式构建成功并生成 `shiyi-v1.0.14.zip`。
- 默认调参输出确认：`work = 32`，`inverse-multiple = 196608`，`work-max = 50135040`。
- `--inverse-multiple 327680` 在 RTX 3070 上返回退出码 1，并提示安全上限或自动调参。
- 默认随机单目标后 8 位 6 秒：356.589 MH/s。
- 全 0 初始私钥、后 16 位向上、后 8 位地址测试 6 秒：356.337 MH/s。
- 构建和测试后未发现残留 `shiyi`、旧 `profanity*`、`TronStudio` 或 `start` 进程。

### 性能状态

400 MH/s 目标尚未达成。本版本补强手动参数安全性；下一轮仍需继续优化 inverse/iterate 主内核和 checksum score 路径。

## English

### Release Content

- Windows launcher: `start.exe`
- Native OpenCL generator: `shiyi.exe`
- Release asset file: `shiyi-v1.0.14.zip`
- Runtime OpenCL kernels: `kernels/*.cl`
- Default target files: `profanity.txt` and `runtime/targets.txt`
- Launcher helper: `start.bat`
- Package metadata: `README.md` and `VERSION.txt`

### Changes

- Added a 60% GPU memory-budget check for manual `--inverse-multiple` values.
- Overly large manual batches now return a clear error instead of crashing during initialization.
- Added `--inverse-multiple` safety notes to `--help`.
- Keeps the v1.0.13 multi-target suffix coarse filter, v1.0.12 single-target suffix coarse filter, and optional code-signing build path.

### Verification

- Parameter matrix tests were run serially to avoid false results from benchmark process cleanup.
- The official `v1.0.14` build succeeded and produced `shiyi-v1.0.14.zip`.
- Default tuning output confirms `work = 32`, `inverse-multiple = 196608`, and `work-max = 50135040`.
- `--inverse-multiple 327680` returns exit code 1 on RTX 3070 and suggests the safe cap or automatic tuning.
- Default random single-target last-8, 6 seconds: 356.589 MH/s.
- All-zero initial key, last-16 upward, address last-8 test, 6 seconds: 356.337 MH/s.
- No leftover `shiyi`, old `profanity*`, `TronStudio`, or `start` processes were found after build and tests.

### Performance Status

The 400 MH/s target has not been reached yet. This version improves manual-parameter safety; the next pass still needs deeper inverse/iterate kernel and checksum score-path optimization.
