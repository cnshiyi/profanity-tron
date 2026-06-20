# profanity-tron / 波场靓号生成工具

中文：

这是一个 Windows / OpenCL 的 Tron 靓号地址生成工具。Windows 图形启动器是 `start.exe`，本地 OpenCL 生成器是 `shiyi.exe`。

English:

This is a Windows / OpenCL Tron vanity address generator. The Windows launcher is `start.exe`; the native OpenCL generator is `shiyi.exe`.

## 使用 / Usage

中文：

1. 下载发布包并解压。
2. 双击 `start.exe` 打开图形界面。
3. 目标地址可以在界面里的“目标地址编辑”区域修改。
4. 初始私钥、方向、位数都可以留空；留空时使用随机模式。
5. 指定位数模式只允许 1-16 个十六进制位，避免输入过大的范围导致任务不可控。

English:

1. Download and extract the release package.
2. Run `start.exe` to open the graphical launcher.
3. Edit targets in the target editor area.
4. Initial private key, direction, and digit count are optional; leave them blank for random mode.
5. Fixed-digit mode is limited to 1-16 hex digits to prevent oversized ranges.

## 发布包内容 / Package Contents

- `start.exe`: 图形启动器 / graphical launcher
- `shiyi.exe`: OpenCL 生成器 / OpenCL generator
- `runtime/targets.txt`: 默认目标列表 / default target list
- `profanity.txt`: 默认目标列表副本 / copy of the default target list
- `kernels/*.cl`: OpenCL 内核源码 / OpenCL kernel sources
- `README.md`, `VERSION.txt`: 包说明和版本信息 / package documentation and version metadata

## 本地构建 / Local Build

中文：

在仓库根目录执行：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build-windows.ps1 -Version v1.0.6
```

English:

From the repository root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build-windows.ps1 -Version v1.0.6
```

生成结果位于 `dist/`，发布 zip 为 `dist/shiyi-v1.0.6.zip`。

Build outputs are written to `dist/`; the release zip is `dist/shiyi-v1.0.6.zip`.

## 验证 / Verification

中文：

当前基线在 RTX 3070 上测试：

- 随机单目标：383.894 MH/s
- 默认多目标：382.824 MH/s
- 指定初始私钥范围：419.475 MH/s

English:

Current RTX 3070 baseline:

- Random single-target: 383.894 MH/s
- Default multi-target: 382.824 MH/s
- Explicit initial-key range: 419.475 MH/s

400 MH/s 目标在指定初始私钥范围测试中已达到，但随机/默认模式仍需要继续优化。

The 400 MH/s target is reached for the explicit initial-key range test, while random/default modes still need optimization.
