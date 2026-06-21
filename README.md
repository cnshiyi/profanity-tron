# profanity-tron / 波场靓号生成工具

## 中文

这是一个 Windows / OpenCL 的 Tron 靓号地址生成工具。Windows 图形启动器是 `start.exe`，本地 OpenCL 生成器是 `shiyi.exe`。

### 使用

1. 下载并解压发布包。
2. 双击 `start.exe` 打开图形界面。
3. 在“目标地址编辑”区域维护目标地址，每行一个目标。
4. 初始私钥、方向、位数都可以留空；留空时使用随机模式。
5. 指定位数模式只允许 1-16 个十六进制位。全 0 初始私钥会自动跳过无效私钥 `0`。
6. 初始私钥留空但填写位数/方向时，会使用随机前缀并按位数对齐完整窗口；方向留空时只在任务开始时随机一次，自动续跑沿用该方向。

### 发布包内容

- `start.exe`：图形启动器
- `shiyi.exe`：OpenCL 生成器
- `runtime/targets.txt`：默认目标列表
- `profanity.txt`：默认目标列表副本
- `kernels/*.cl`：OpenCL 内核源码
- `README.md`、`VERSION.txt`：包说明和版本信息

### 本地构建

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build-windows.ps1 -Version v1.0.21
```

如果 Windows Application Control / 企业策略要求签名，可传入 `-SignThumbprint` 或 `-SignPfxPath/-SignPfxPassword`。

生成结果位于 `dist/`，发布 zip 为 `dist/shiyi-v1.0.21.zip`。开发 profiling 构建可额外传入 `-DebugNative`。

### 当前验证

RTX 3070 当前可信样本：

- 随机单目标后 8 位 60 秒：383.908 MH/s
- 全 0 初始私钥、后 16 位向上、后 8 位地址测试 60 秒：383.174 MH/s
- Debug profiling 样本：iterate 约 66.1 ms，inverse 约 38.5 ms，score 约 24.7 ms
- 默认调参输出：`work = 32`，`inverse-multiple = 196608`
- v1.0.17 最终合法目标 5 秒冒烟：352.834 MH/s；非法 `0` 目标会被拒绝
- v1.0.18 修复 range 重复扫描和启动器随机位数/方向续跑逻辑；本机新编译 exe 被 Application Control 拦截，未声明 1 分钟性能结果
- v1.0.19 修正 v1.0.18 的 range stride 风险，恢复与 GPU 迭代一致的私钥重建公式
- v1.0.21 修复有限 range 模式窗口外结果污染，新增小窗口 runtime 回归；随机后 8 位 60 秒为 371.654 MH/s，后 16 位向上 range 为 370.560 MH/s，400 MH/s 目标仍未达成

400 MH/s 目标尚未达成，需要继续优化。

手动设置 `--inverse-multiple` 时，程序会按 60% GPU 显存预算做安全校验；一般建议留空使用自动调参。

## English

This is a Windows / OpenCL Tron vanity address generator. The Windows launcher is `start.exe`; the native OpenCL generator is `shiyi.exe`.

### Usage

1. Download and extract the release package.
2. Run `start.exe` to open the graphical launcher.
3. Edit targets in the target editor area, one target per line.
4. Initial private key, direction, and digit count are optional; leave them blank for random mode.
5. Fixed-digit mode is limited to 1-16 hex digits. An all-zero initial private key automatically skips invalid private key `0`.
6. When the initial key is blank but digit count or direction is set, the launcher uses a random prefix and aligns a full digit window; blank direction is randomized once at task start and reused by auto-continued windows.

### Package Contents

- `start.exe`: graphical launcher
- `shiyi.exe`: OpenCL generator
- `runtime/targets.txt`: default target list
- `profanity.txt`: copy of the default target list
- `kernels/*.cl`: OpenCL kernel sources
- `README.md`, `VERSION.txt`: package documentation and version metadata

### Local Build

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build-windows.ps1 -Version v1.0.21
```

If Windows Application Control or an enterprise policy requires signing, pass `-SignThumbprint` or `-SignPfxPath/-SignPfxPassword`.

Build outputs are written to `dist/`; the release zip is `dist/shiyi-v1.0.21.zip`. Add `-DebugNative` for a development profiling build.

### Current Verification

Current trusted RTX 3070 samples:

- Random single-target last-8, 60 seconds: 383.908 MH/s
- All-zero initial key, last-16 upward, address last-8 test, 60 seconds: 383.174 MH/s
- Debug profiling sample: iterate about 66.1 ms, inverse about 38.5 ms, score about 24.7 ms
- Default tuning output: `work = 32`, `inverse-multiple = 196608`
- v1.0.17 final valid-target 5-second smoke: 352.834 MH/s; invalid `0` targets are rejected
- v1.0.18 fixes range duplicate scanning and launcher random digit/direction auto-continue behavior; the freshly built local exe is blocked by Application Control, so no 1-minute performance result is claimed
- v1.0.19 corrects the v1.0.18 range stride risk and restores private-key reconstruction to the GPU-compatible iterator formula
- v1.0.21 fixes finite range result contamination outside the requested window and adds a small-window runtime regression; random last-8 reached 371.654 MH/s over 60 seconds, last-16 upward range reached 370.560 MH/s, and the 400 MH/s target remains open

The 400 MH/s target has not been reached yet and needs further optimization.

Manual `--inverse-multiple` values are checked against a 60% GPU memory budget; leaving it empty for automatic tuning is recommended.

`scripts/benchmark-local.ps1` returns `RUNNING` if the target directory already has a benchmark process. Pass `-StopExisting` only when that cleanup is intentional.
