# v1.0.15

## 中文

### 发布内容

- Windows 图形启动器：`start.exe`
- 原生 OpenCL 生成器：`shiyi.exe`
- 发布资产文件：`shiyi-v1.0.15.zip`
- 运行时 OpenCL 内核：`kernels/*.cl`
- 默认目标文件：`profanity.txt` 和 `runtime/targets.txt`
- 启动辅助脚本：`start.bat`
- 包说明和版本信息：`README.md`、`VERSION.txt`

### 本次更新

- 修复默认目标地址：TRON/Base58 地址不包含 `0`，默认 10 行目标改为 `1-9 + A`；`profanity.txt` 和 `runtime/targets.txt` 已同步。
- 修复重复命中计数：GPU 每轮清空结果缓冲，单批最多回传 120 个命中槽，CPU 按私钥去重后再统计和保存。
- `--quit-count` 现在表示唯一验证命中条数，不再作为 GPU 分数阈值使用；默认数量 999 不会再溢出成错误阈值。
- 修复 `scripts/benchmark-local.ps1` 的超时处理：超时后会强制结束进程、等待退出、保存 stdout/stderr，并返回结构化 `TIMEOUT` 结果。
- 继续保留 v1.0.14 的手动 `--inverse-multiple` 显存安全校验、v1.0.13/v1.0.12 的后缀粗筛路径和可选代码签名构建通道。
- 本轮没有发布无收益 kernel 实验；拆分模乘归约分支和 `modHigher` 常量区实验均已拒绝。

### 验证

- benchmark 脚本正常退出路径验证通过：2 秒随机后 8 位测试为 349.926 MH/s。
- benchmark 脚本超时路径验证通过：`TimeoutSeconds=1` 时返回 `TIMEOUT`，并保存初始化阶段 stdout/stderr。
- 新编译 `shiyi.exe` 在本机被 Smart App Control / Code Integrity 拦截，系统日志显示未满足企业签名级别要求；本轮未完成本地 1 分钟性能实测。
- 官方 v1.0.14 发布包参考复测：随机单目标后 8 位 20 秒为 357.353 MH/s。
- 官方 v1.0.14 发布包参考复测：全 0 初始私钥、后 16 位向上、后 8 位地址测试 20 秒为 356.712 MH/s。
- 构建/测试后未发现残留 `shiyi`、旧 `profanity*`、`TronStudio` 或 `start` 进程。

### 性能状态

400 MH/s 目标尚未达成。本版本主要修复默认目标和重复计数，并提升长跑 benchmark 的诊断可靠性；后续性能结论需要在签名或策略允许的构建产物上做 1 分钟实测。

## English

### Release Content

- Windows launcher: `start.exe`
- Native OpenCL generator: `shiyi.exe`
- Release asset file: `shiyi-v1.0.15.zip`
- Runtime OpenCL kernels: `kernels/*.cl`
- Default target files: `profanity.txt` and `runtime/targets.txt`
- Launcher helper: `start.bat`
- Package metadata: `README.md` and `VERSION.txt`

### Changes

- Fixed the default targets: TRON/Base58 addresses do not contain `0`, so the default 10-line set is now `1-9 + A`; `profanity.txt` and `runtime/targets.txt` are synchronized.
- Fixed duplicate hit counting: the GPU result buffer is cleared every round, each batch can return up to 120 hit slots, and the CPU deduplicates by private key before counting and saving.
- `--quit-count` now means unique validated hit count, not a GPU score threshold; the default count of 999 no longer overflows into a wrong threshold.
- Fixed `scripts/benchmark-local.ps1` timeout handling: timed-out runs now kill the process, wait for exit, preserve stdout/stderr, and return a structured `TIMEOUT` result.
- Keeps v1.0.14 manual `--inverse-multiple` memory safety checks, v1.0.13/v1.0.12 suffix coarse-filter paths, and the optional code-signing build path.
- Does not ship unproven kernel experiments; the split modular-multiply reduction branch and `modHigher` constant-memory experiment were both rejected.

### Verification

- Benchmark script normal-exit path passed: 2-second random last-8 test measured 349.926 MH/s.
- Benchmark script timeout path passed: `TimeoutSeconds=1` returned `TIMEOUT` and preserved initialization stdout/stderr.
- The freshly built `shiyi.exe` was blocked locally by Smart App Control / Code Integrity because it did not meet the enterprise signing level; this round did not complete local 1-minute performance verification.
- Official v1.0.14 package reference retest: random single-target last-8, 20 seconds, 357.353 MH/s.
- Official v1.0.14 package reference retest: all-zero initial key, last-16 upward range, address last-8 test, 20 seconds, 356.712 MH/s.
- No leftover `shiyi`, old `profanity*`, `TronStudio`, or `start` processes were found after build and tests.

### Performance Status

The 400 MH/s target has not been reached yet. This version fixes default targets and duplicate counting, and improves long-run benchmark diagnostics; future performance claims need 1-minute verification on a signed or policy-allowed build.
