# v1.0.11

## 中文

### 发布内容

- Windows 图形启动器：`start.exe`
- 原生 OpenCL 生成器：`shiyi.exe`
- 发布资产文件：`shiyi-v1.0.11.zip`
- 运行时 OpenCL 内核：`kernels/*.cl`
- 默认目标文件：`profanity.txt` 和 `runtime/targets.txt`
- 启动辅助脚本：`start.bat`
- 包说明和版本信息：`README.md`、`VERSION.txt`

### 本次更新

- 根据 RTX 3070 串行矩阵测试，将 64 CU 以下 NVIDIA 默认 `work` 调为 `32`。
- 将 64 CU 以下 NVIDIA 档位的默认 `inverse-multiple` 调为 `196608`。
- 64 CU 及以上 NVIDIA 显卡仍保留更高计算单元分档，避免把更高端显卡固定成 RTX 3070 参数。
- 保留 v1.0.10 的可选代码签名构建通道。

### 验证

- 参数矩阵测试均串行运行，避免 benchmark 脚本互相清理进程造成假结果。
- `v1.0.11` 正式构建成功并生成 `shiyi-v1.0.11.zip`。
- 默认调参输出确认：`work = 32`，`inverse-multiple = 196608`，`work-max = 50135040`。
- 正式构建后默认参数复测：
  - 随机单目标 6 秒：338.415 MH/s
  - 全 0 初始私钥、后 16 位向上、后 8 位地址测试 6 秒：338.758 MH/s
- 构建和测试后未发现残留 `shiyi`、旧 `profanity*`、`TronStudio` 或 `start` 进程。

### 性能状态

400 MH/s 目标尚未达成。本版本是一次可回溯的默认参数小幅提升；下一轮仍需继续做 GPU 粗筛、CPU 细筛或更深的内核路径优化。

## English

### Release Content

- Windows launcher: `start.exe`
- Native OpenCL generator: `shiyi.exe`
- Release asset file: `shiyi-v1.0.11.zip`
- Runtime OpenCL kernels: `kernels/*.cl`
- Default target files: `profanity.txt` and `runtime/targets.txt`
- Launcher helper: `start.bat`
- Package metadata: `README.md` and `VERSION.txt`

### Changes

- Based on RTX 3070 serial matrix tests, changed default NVIDIA `work` below 64 compute units to `32`.
- Changed the same NVIDIA tier default `inverse-multiple` to `196608`.
- NVIDIA GPUs with 64 or more compute units still keep higher compute-unit tiers, so higher-end cards are not pinned to RTX 3070-class parameters.
- Keeps the optional code-signing build path from v1.0.10.

### Verification

- Parameter matrix tests were run serially to avoid false results from benchmark process cleanup.
- The official `v1.0.11` build succeeded and produced `shiyi-v1.0.11.zip`.
- Default tuning output confirms `work = 32`, `inverse-multiple = 196608`, and `work-max = 50135040`.
- Official-build default-parameter retests:
  - Random single-target, 6 seconds: 338.415 MH/s
  - All-zero initial key, last-16 upward, address last-8 test, 6 seconds: 338.758 MH/s
- No leftover `shiyi`, old `profanity*`, `TronStudio`, or `start` processes were found after build and tests.

### Performance Status

The 400 MH/s target has not been reached yet. This version is a small, traceable default-parameter improvement; the next pass still needs GPU coarse filtering, CPU fine filtering, or deeper kernel-path optimization.
