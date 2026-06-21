# v1.0.22

## 中文

### 发布内容

- Windows 图形启动器：`start.exe`
- 原生 OpenCL 生成器：`shiyi.exe`
- 发布资产：`shiyi-v1.0.22.zip`
- 运行时 OpenCL 内核：`kernels/*.cl`
- 默认目标文件：`profanity.txt` 和 `runtime/targets.txt`
- 启动辅助脚本：`start.bat`
- 包说明和版本信息：`README.md`、`VERSION.txt`

### 本次更新

- 修正有限 range 模式的速度计数：只把第一次新增覆盖、且仍在用户指定窗口内的候选计入速度样本。
- 继续保留 v1.0.21 的窗口外结果保护，避免小窗口保存越界私钥。
- 修正结果文件格式：移除 `score=`，并将 `time=` 放到每行最后。
- 扩展 `scripts/test-range-runtime.ps1`，同时验证小窗口边界、已知后 8 位命中、输出格式和 range 真实覆盖计数。

### 验证

- `scripts/test-range-planner.ps1` 通过。
- `scripts/test-range-runtime.ps1` 通过。
- 构建通过：`dist/shiyi-v1.0.22.zip`。
- 随机后 8 位 60 秒：`352.092 MH/s`。
- 全 0 初始私钥、后 16 位向上 range 60 秒：`7.001 H/s`，这是修正重复覆盖计数后的真实新增覆盖速度。

### 性能状态

本版本不声明 400 MH/s 目标已达成。修正后的 range 数据证明当前 `+G` 滑动迭代会大量重复覆盖，下一步需要做 GPU 迭代步长架构优化，而不是继续相信重复计数的虚高 MH/s。

## English

### Release Content

- Windows launcher: `start.exe`
- Native OpenCL generator: `shiyi.exe`
- Release asset: `shiyi-v1.0.22.zip`
- Runtime OpenCL kernels: `kernels/*.cl`
- Default target files: `profanity.txt` and `runtime/targets.txt`
- Launcher helper: `start.bat`
- Package metadata: `README.md` and `VERSION.txt`

### Changes

- Corrected finite range speed accounting: only newly covered candidates still inside the requested window are sampled.
- Kept the v1.0.21 out-of-window result guard so small windows do not save private keys outside the requested range.
- Fixed saved result format: removed `score=` and moved `time=` to the end of each line.
- Extended `scripts/test-range-runtime.ps1` to verify small-window bounds, the known suffix-8 hit, output format, and corrected range coverage accounting.

### Verification

- `scripts/test-range-planner.ps1` passed.
- `scripts/test-range-runtime.ps1` passed.
- Build passed: `dist/shiyi-v1.0.22.zip`.
- Random last-8, 60 seconds: `352.092 MH/s`.
- All-zero initial key, last-16 upward range, 60 seconds: `7.001 H/s` under the corrected newly-covered-candidate metric.

### Performance Status

This release does not claim the 400 MH/s target. The corrected range data proves the current `+G` sliding iterator repeats coverage heavily, so the next optimization step must change the GPU iterator stride architecture instead of trusting inflated duplicate-count MH/s.
