# profanity-tron / 波场靓号生成工具

## 中文

这是一个 Windows / OpenCL 的 Tron 靓号地址生成工具。Windows 图形启动器是 `start.exe`，本地 OpenCL 生成器是 `shiyi.exe`。

### 使用

1. 下载并解压发布包。
2. 双击 `start.exe` 打开图形界面。
3. 在“目标地址编辑”区域维护目标地址，每行一个目标。
4. 初始私钥、方向、位数都可以留空；留空时使用随机模式。
5. 指定位数模式只允许 1-16 个十六进制位。全 0 初始私钥会自动跳过无效私钥 `0`。

### 发布包内容

- `start.exe`：图形启动器
- `shiyi.exe`：OpenCL 生成器
- `runtime/targets.txt`：默认目标列表
- `profanity.txt`：默认目标列表副本
- `kernels/*.cl`：OpenCL 内核源码
- `README.md`、`VERSION.txt`：包说明和版本信息

### 本地构建

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build-windows.ps1 -Version v1.0.14
```

如果 Windows Application Control / 企业策略要求签名，可传入 `-SignThumbprint` 或 `-SignPfxPath/-SignPfxPassword`。

生成结果位于 `dist/`，发布 zip 为 `dist/shiyi-v1.0.14.zip`。

### 当前验证

RTX 3070 当前可信样本：

- 随机单目标后 8 位 10 秒：354.943 MH/s
- 全 0 初始私钥、后 16 位向上、后 8 位地址测试 10 秒：357.349 MH/s
- 默认 10 目标列表 8 秒：348.527 MH/s
- 默认调参输出：`work = 32`，`inverse-multiple = 196608`

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

### Package Contents

- `start.exe`: graphical launcher
- `shiyi.exe`: OpenCL generator
- `runtime/targets.txt`: default target list
- `profanity.txt`: copy of the default target list
- `kernels/*.cl`: OpenCL kernel sources
- `README.md`, `VERSION.txt`: package documentation and version metadata

### Local Build

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build-windows.ps1 -Version v1.0.14
```

If Windows Application Control or an enterprise policy requires signing, pass `-SignThumbprint` or `-SignPfxPath/-SignPfxPassword`.

Build outputs are written to `dist/`; the release zip is `dist/shiyi-v1.0.14.zip`.

### Current Verification

Current trusted RTX 3070 samples:

- Random single-target last-8, 10 seconds: 354.943 MH/s
- All-zero initial key, last-16 upward, address last-8 test, 10 seconds: 357.349 MH/s
- Default 10-target list, 8 seconds: 348.527 MH/s
- Default tuning output: `work = 32`, `inverse-multiple = 196608`

The 400 MH/s target has not been reached yet and needs further optimization.

Manual `--inverse-multiple` values are checked against a 60% GPU memory budget; leaving it empty for automatic tuning is recommended.
