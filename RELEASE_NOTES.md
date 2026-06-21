# v1.0.21

## 中文

### 发布内容

- Windows 图形启动器：`start.exe`
- 原生 OpenCL 生成器：`shiyi.exe`
- 发布资产文件：`shiyi-v1.0.21.zip`
- 运行时 OpenCL 内核：`kernels/*.cl`
- 默认目标文件：`profanity.txt` 和 `runtime/targets.txt`
- 启动辅助脚本：`start.bat`
- 包说明和版本信息：`README.md`、`VERSION.txt`

### 本次更新

- 修复有限 range 模式的窗口外结果污染：GPU score 阶段现在按当前轮次偏移检查真实候选是否仍在用户指定窗口内。
- 小窗口 `1..f` 不再保存 `10`、`11` 等窗口外私钥。
- 有限 range 的退出条件同步收紧，当前可达候选耗尽后更早结束，降低小窗口测试空转和进程残留风险。
- 新增 `scripts/test-range-runtime.ps1`，自动验证小窗口边界和已知全 0 后 16 位向上 suffix-8 命中。

### 验证

- `scripts/test-range-planner.ps1` 通过。
- `scripts/test-range-runtime.ps1` 通过。
- 小窗口 `0..f` 跳过无效私钥 `0` 后，仅保存 `...0003` 到 `...000f`，不再越界保存 `...0010` 之后。
- 已知命中仍成立：目标 `TJSWuhAyiDQyb7E8ne1AitDV62H8HXRnvU`，私钥 `0000000000000000000000000000000000000000000000000000000002fcf6a3`。

### 性能状态

本版本是 range 正确性修复，不声明 400 MH/s 已达成。完整不重复、不漏扫的 range 遍历和 400 MH/s 优化仍继续迭代。

## English

### Release Content

- Windows launcher: `start.exe`
- Native OpenCL generator: `shiyi.exe`
- Release asset: `shiyi-v1.0.21.zip`
- Runtime OpenCL kernels: `kernels/*.cl`
- Default target files: `profanity.txt` and `runtime/targets.txt`
- Launcher helper: `start.bat`
- Package metadata: `README.md` and `VERSION.txt`

### Changes

- Fixed finite range result contamination outside the requested window: the GPU score stage now checks the current round offset before accepting a range candidate.
- A small `1..f` window no longer saves out-of-window keys such as `10` or `11`.
- Tightened finite range exit conditions so the currently reachable candidate window ends earlier, reducing idle loops and process-cleanup risk in small-window tests.
- Added `scripts/test-range-runtime.ps1` to verify the small-window boundary and the known all-zero last-16 upward suffix-8 hit.

### Verification

- `scripts/test-range-planner.ps1` passed.
- `scripts/test-range-runtime.ps1` passed.
- The `0..f` small-window test skips invalid private key `0` and only saves `...0003` through `...000f`; it no longer saves `...0010` or later.
- The known hit still works: target `TJSWuhAyiDQyb7E8ne1AitDV62H8HXRnvU`, private key `0000000000000000000000000000000000000000000000000000000002fcf6a3`.

### Performance Status

This is a range correctness release and does not claim the 400 MH/s target. Complete non-duplicate, non-skipping range traversal and the 400 MH/s optimization goal remain active follow-up work.
