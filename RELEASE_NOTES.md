# v1.0.12

## 中文

### 发布内容

- Windows 图形启动器：`start.exe`
- 原生 OpenCL 生成器：`shiyi.exe`
- 发布资产文件：`shiyi-v1.0.12.zip`
- 运行时 OpenCL 内核：`kernels/*.cl`
- 默认目标文件：`profanity.txt` 和 `runtime/targets.txt`
- 启动辅助脚本：`start.bat`
- 包说明和版本信息：`README.md`、`VERSION.txt`

### 本次更新

- 单目标后缀 3-12 位匹配增加后 2 位 GPU 粗筛，减少完整 Base58 尾部计算。
- 保留 v1.0.11 的 NVIDIA 默认参数分档，64 CU 及以上显卡仍不会被固定成 RTX 3070 参数。
- 保留可选代码签名构建通道。

### 验证

- 参数矩阵测试均串行运行，避免 benchmark 脚本互相清理进程造成假结果。
- `v1.0.12` 正式构建成功并生成 `shiyi-v1.0.12.zip`。
- 默认调参输出确认：`work = 32`，`inverse-multiple = 196608`，`work-max = 50135040`。
- 正式构建后默认参数复测：
  - 随机单目标后 8 位 10 秒：354.943 MH/s
  - 全 0 初始私钥、后 16 位向上、后 8 位地址测试 10 秒：357.349 MH/s
- 正确性验证命中地址 `TJSWuhAyiDQyb7E8ne1AitDV62H8HXRnvU`，私钥 `0000000000000000000000000000000000000000000000000000000002fcf6a3`。
- `--inverse-size` 扫描未突破默认 `255` 的当前上限。
- 构建和测试后未发现残留 `shiyi`、旧 `profanity*`、`TronStudio` 或 `start` 进程。

### 性能状态

400 MH/s 目标尚未达成。本版本是一次后缀匹配粗筛优化；下一轮仍需继续优化 inverse/iterate 主内核。

## English

### Release Content

- Windows launcher: `start.exe`
- Native OpenCL generator: `shiyi.exe`
- Release asset file: `shiyi-v1.0.12.zip`
- Runtime OpenCL kernels: `kernels/*.cl`
- Default target files: `profanity.txt` and `runtime/targets.txt`
- Launcher helper: `start.bat`
- Package metadata: `README.md` and `VERSION.txt`

### Changes

- Added a last-2-character GPU coarse filter for single-target suffix matching from 3 to 12 chars, reducing full Base58 tail computation.
- Keeps the v1.0.11 NVIDIA default tuning tiers, so GPUs with 64 or more compute units are still not pinned to RTX 3070-class parameters.
- Keeps the optional code-signing build path.

### Verification

- Parameter matrix tests were run serially to avoid false results from benchmark process cleanup.
- The official `v1.0.12` build succeeded and produced `shiyi-v1.0.12.zip`.
- Default tuning output confirms `work = 32`, `inverse-multiple = 196608`, and `work-max = 50135040`.
- Official-build default-parameter retests:
  - Random single-target last-8, 10 seconds: 354.943 MH/s
  - All-zero initial key, last-16 upward, address last-8 test, 10 seconds: 357.349 MH/s
- Correctness verification hit address `TJSWuhAyiDQyb7E8ne1AitDV62H8HXRnvU` with private key `0000000000000000000000000000000000000000000000000000000002fcf6a3`.
- The `--inverse-size` scan did not beat the default `255` ceiling.
- No leftover `shiyi`, old `profanity*`, `TronStudio`, or `start` processes were found after build and tests.

### Performance Status

The 400 MH/s target has not been reached yet. This version is a suffix-matching coarse-filter optimization; the next pass still needs deeper inverse/iterate kernel optimization.
