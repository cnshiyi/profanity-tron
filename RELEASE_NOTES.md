# v1.0.13

## 中文

### 发布内容

- Windows 图形启动器：`start.exe`
- 原生 OpenCL 生成器：`shiyi.exe`
- 发布资产文件：`shiyi-v1.0.13.zip`
- 运行时 OpenCL 内核：`kernels/*.cl`
- 默认目标文件：`profanity.txt` 和 `runtime/targets.txt`
- 启动辅助脚本：`start.bat`
- 包说明和版本信息：`README.md`、`VERSION.txt`

### 本次更新

- 多目标后缀 2-12 位匹配增加统一后 2 位 GPU 粗筛。
- 默认目标列表中含 Base58 非法字符 `0` 的目标不再拖慢完整尾部比较。
- 保留 v1.0.12 的单目标后缀粗筛和 NVIDIA 默认参数分档。
- 保留可选代码签名构建通道。

### 验证

- 参数矩阵测试均串行运行，避免 benchmark 脚本互相清理进程造成假结果。
- `v1.0.13` 正式构建成功并生成 `shiyi-v1.0.13.zip`。
- 默认调参输出确认：`work = 32`，`inverse-multiple = 196608`，`work-max = 50135040`。
- 测试版同口径复测：
  - 默认 10 目标列表 8 秒：348.527 MH/s
  - 合法 Base58 10 目标列表 8 秒：354.192 MH/s
  - 随机单目标后 8 位 8 秒：356.208 MH/s
  - 全 0 初始私钥、后 16 位向上、后 8 位地址测试 10 秒：356.719 MH/s
- 正确性验证命中地址 `TJSWuhAyiDQyb7E8ne1AitDV62H8HXRnvU`，私钥 `0000000000000000000000000000000000000000000000000000000002fcf6a3`。
- 构建和测试后未发现残留 `shiyi`、旧 `profanity*`、`TronStudio` 或 `start` 进程。

### 性能状态

400 MH/s 目标尚未达成。本版本提升默认多目标路径；下一轮仍需继续优化 inverse/iterate 主内核和 checksum score 路径。

## English

### Release Content

- Windows launcher: `start.exe`
- Native OpenCL generator: `shiyi.exe`
- Release asset file: `shiyi-v1.0.13.zip`
- Runtime OpenCL kernels: `kernels/*.cl`
- Default target files: `profanity.txt` and `runtime/targets.txt`
- Launcher helper: `start.bat`
- Package metadata: `README.md` and `VERSION.txt`

### Changes

- Added a unified last-2-character GPU coarse filter for multi-target suffix matching from 2 to 12 chars.
- Targets containing non-Base58 character `0` in the default list no longer force full-tail comparisons.
- Keeps the v1.0.12 single-target suffix coarse filter and NVIDIA default tuning tiers.
- Keeps the optional code-signing build path.

### Verification

- Parameter matrix tests were run serially to avoid false results from benchmark process cleanup.
- The official `v1.0.13` build succeeded and produced `shiyi-v1.0.13.zip`.
- Default tuning output confirms `work = 32`, `inverse-multiple = 196608`, and `work-max = 50135040`.
- Test-build same-method retests:
  - Default 10-target list, 8 seconds: 348.527 MH/s
  - Valid Base58 10-target list, 8 seconds: 354.192 MH/s
  - Random single-target last-8, 8 seconds: 356.208 MH/s
  - All-zero initial key, last-16 upward, address last-8 test, 10 seconds: 356.719 MH/s
- Correctness verification hit address `TJSWuhAyiDQyb7E8ne1AitDV62H8HXRnvU` with private key `0000000000000000000000000000000000000000000000000000000002fcf6a3`.
- No leftover `shiyi`, old `profanity*`, `TronStudio`, or `start` processes were found after build and tests.

### Performance Status

The 400 MH/s target has not been reached yet. This version improves the default multi-target path; the next pass still needs deeper inverse/iterate kernel and checksum score-path optimization.
