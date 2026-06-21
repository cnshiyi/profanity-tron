# v1.0.9

## 中文

### 发布内容

- Windows 图形启动器：`start.exe`
- 原生 OpenCL 生成器：`shiyi.exe`
- 发布资产文件：`shiyi-v1.0.9.zip`
- 运行时 OpenCL 内核：`kernels/*.cl`
- 默认目标文件：`profanity.txt` 和 `runtime/targets.txt`
- 启动辅助脚本：`start.bat`
- 包说明和版本信息：`README.md`、`VERSION.txt`

### 本次更新

- 修复指定位数范围模式只支持私钥前 16 位的问题，现在可支持任意一个 16 位私钥块内的连续 1-16 个十六进制位。
- 修复启动器“位数”生成范围时误改前 16 位的问题，现在按用户预期修改私钥后缀位数。
- 支持用户输入全 0 初始私钥；程序会自动跳过无效私钥 `0`，从 `...0001` 开始扫描。
- 修复范围模式命中后私钥回写少 1 的问题，输出私钥现在可独立还原输出地址。
- 命令行相对 `--output` 路径会自动创建父目录，避免命中后无法保存。
- NVIDIA 默认调参改为按显存预算和计算单元分档，不再把 RTX 3070 一类参数硬编码到所有 NVIDIA 显卡。

### 验证

- `shiyi.exe --help` 正常运行。
- 使用全 0 初始私钥、后 16 位向上、地址后 8 位测试，程序输出 `variable hex = 49-64`、`lane = 0`，并提示跳过无效私钥 `0`。
- 低难度命中回归：程序输出地址 `TMy8JcLsT3cAwTMdNbEBHY4QPoJ1mdrUyK` 和私钥 `...008b`；独立 secp256k1 + Keccak 校验通过。
- 相对输出路径回归：`--output experiments\tmp\relative_output_hit.txt` 可成功保存命中。
- 构建和测试后未发现残留 `shiyi`、旧 `profanity*`、`TronStudio` 或 `start` 进程。
- 正式 `v1.0.9` 构建包内容已验证；本机再次运行 `dist\shiyi.exe --help` 时被 Windows Application Control 策略拦截，Code Integrity 事件为 3033/3077，原因是未满足企业签名级别要求。

### 性能状态

RTX 3070 当前可信样本：

- 随机单目标 5 秒：353.158 MH/s
- 默认多目标 5 秒：331.924 MH/s
- 全 0 初始私钥、后 16 位向上、后 8 位地址测试 5 秒：333.261 MH/s

400 MH/s 目标尚未达成。下一轮优化应继续围绕 GPU 粗筛、CPU 细筛，以及低位范围模式的批次步进/去重策略。

## English

### Release Content

- Windows launcher: `start.exe`
- Native OpenCL generator: `shiyi.exe`
- Release asset file: `shiyi-v1.0.9.zip`
- Runtime OpenCL kernels: `kernels/*.cl`
- Default target files: `profanity.txt` and `runtime/targets.txt`
- Launcher helper: `start.bat`
- Package metadata: `README.md` and `VERSION.txt`

### Changes

- Fixed range mode being limited to the first 16 private-key hex chars. It now supports any continuous 1-16 hex chars inside one 16-char private-key block.
- Fixed the launcher digit-count range builder so it edits the private-key suffix digits as intended.
- Allows an all-zero initial private key input; the generator skips invalid private key `0` and starts from `...0001`.
- Fixed range-mode private-key reporting being off by one. Printed private keys now independently reproduce printed addresses.
- Relative command-line `--output` paths now create parent directories before saving hits.
- NVIDIA default tuning now uses memory-budget and compute-unit tiers instead of hard-coding RTX 3070-class parameters for every NVIDIA GPU.

### Verification

- `shiyi.exe --help` runs successfully.
- All-zero initial key, last-16-hex upward, address last-8 test prints `variable hex = 49-64`, `lane = 0`, and reports that invalid private key `0` was skipped.
- Low-difficulty hit regression: printed address `TMy8JcLsT3cAwTMdNbEBHY4QPoJ1mdrUyK` with private key `...008b`; independent secp256k1 + Keccak verification passed.
- Relative output regression: `--output experiments\tmp\relative_output_hit.txt` saves hits successfully.
- No leftover `shiyi`, old `profanity*`, `TronStudio`, or `start` processes were found after build and tests.
- The final `v1.0.9` package contents were verified; rerunning `dist\shiyi.exe --help` on this machine was blocked by Windows Application Control. Code Integrity events 3033/3077 report that the binary did not meet Enterprise signing level requirements.

### Performance Status

Current trusted RTX 3070 samples:

- Random single-target, 5 seconds: 353.158 MH/s
- Default multi-target, 5 seconds: 331.924 MH/s
- All-zero initial key, last-16 upward, address last-8 test, 5 seconds: 333.261 MH/s

The 400 MH/s target has not been reached yet. The next optimization pass should continue on GPU coarse filtering, CPU fine filtering, and batch stepping/deduplication for low-lane range mode.
