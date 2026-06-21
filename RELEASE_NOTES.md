# v1.0.20

## 中文

### 发布内容

- Windows 图形启动器：`start.exe`
- 原生 OpenCL 生成器：`shiyi.exe`
- 发布资产文件：`shiyi-v1.0.20.zip`
- 运行时 OpenCL 内核：`kernels/*.cl`
- 默认目标文件：`profanity.txt` 和 `runtime/targets.txt`
- 启动辅助脚本：`start.bat`
- 包说明和版本信息：`README.md`、`VERSION.txt`

### 本次更新

- 修复 `scripts/benchmark-local.ps1` 的验证污染问题：脚本不再默认杀掉同目录已有 `shiyi`/`profanity*` 进程。
- 发现已有目标进程时，benchmark 脚本返回结构化 `RUNNING` 结果；只有显式传入 `-StopExisting` 才会清理旧进程。
- 记录并回退三条无稳定收益的优化实验：Keccak state 初始化捷径、单目标后 4 位 Base58 粗筛、`iterate+score` fused kernel。
- 本轮不把上述实验作为性能优化发布，避免后续重复实现无收益路径。

### 验证

- `scripts/test-range-planner.ps1` 通过。
- 小窗口后缀 3 校验命中 `TYfP8oUrMb2xPBdbGxHfFUrsRApFM2h999`，验证命中链路正常。
- 串行 60 秒 fused 实验结果：随机后 8 位 370.144 MH/s，后 16 位向上 range 371.519 MH/s。
- 上述速度相对当前本地基线提升太小，且未达到 400 MH/s，因此实验已回退。
- 验证后未发现残留 `shiyi`、旧 `profanity*`、`TronStudio` 或 `start` 进程。

### 性能状态

400 MH/s 目标尚未达成。本版本重点提高 benchmark 证据可靠性，并把已排除的优化路径写入版本记录，方便后续继续迭代。

## English

### Release Content

- Windows launcher: `start.exe`
- Native OpenCL generator: `shiyi.exe`
- Release asset file: `shiyi-v1.0.20.zip`
- Runtime OpenCL kernels: `kernels/*.cl`
- Default target files: `profanity.txt` and `runtime/targets.txt`
- Launcher helper: `start.bat`
- Package metadata: `README.md` and `VERSION.txt`

### Changes

- Fixed benchmark evidence contamination in `scripts/benchmark-local.ps1`: the script no longer kills existing `shiyi`/`profanity*` processes in the same directory by default.
- When a target process is already running, the benchmark script returns a structured `RUNNING` result; cleanup now requires explicit `-StopExisting`.
- Recorded and reverted three experiments without stable gains: Keccak state initialization shortcut, single-target last-4 Base58 coarse filter, and the `iterate+score` fused kernel.
- Those experiments are not shipped as performance optimizations, so future iterations do not repeat paths already ruled out.

### Verification

- `scripts/test-range-planner.ps1` passed.
- A small suffix-3 window smoke hit `TYfP8oUrMb2xPBdbGxHfFUrsRApFM2h999`, confirming the hit path still worked during the experiment.
- Serial 60-second fused experiment results: random last-8 reached 370.144 MH/s, and last-16 upward range reached 371.519 MH/s.
- The measured gain over the current local baseline was too small and did not reach 400 MH/s, so the experiment was reverted.
- No leftover `shiyi`, old `profanity*`, `TronStudio`, or `start` processes were found after verification.

### Performance Status

The 400 MH/s goal has not been reached yet. This release improves benchmark evidence reliability and records rejected optimization paths so later work can continue without repeating them.
