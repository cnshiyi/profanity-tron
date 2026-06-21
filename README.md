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
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build-windows.ps1 -Version v1.0.9
```

生成结果位于 `dist/`，发布 zip 为 `dist/shiyi-v1.0.9.zip`。

### 当前验证

RTX 3070 当前可信样本：

- 随机单目标 5 秒：353.158 MH/s
- 默认多目标 5 秒：331.924 MH/s
- 全 0 初始私钥、后 16 位向上、后 8 位地址测试 5 秒：333.261 MH/s

400 MH/s 目标尚未达成，需要继续优化。

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
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build-windows.ps1 -Version v1.0.9
```

Build outputs are written to `dist/`; the release zip is `dist/shiyi-v1.0.9.zip`.

### Current Verification

Current trusted RTX 3070 samples:

- Random single-target, 5 seconds: 353.158 MH/s
- Default multi-target, 5 seconds: 331.924 MH/s
- All-zero initial key, last-16 upward, address last-8 test, 5 seconds: 333.261 MH/s

The 400 MH/s target has not been reached yet and needs further optimization.
