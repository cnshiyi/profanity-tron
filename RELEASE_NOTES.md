# v1.0.10

## 中文

### 发布内容

- Windows 图形启动器：`start.exe`
- 原生 OpenCL 生成器：`shiyi.exe`
- 发布资产文件：`shiyi-v1.0.10.zip`
- 运行时 OpenCL 内核：`kernels/*.cl`
- 默认目标文件：`profanity.txt` 和 `runtime/targets.txt`
- 启动辅助脚本：`start.bat`
- 包说明和版本信息：`README.md`、`VERSION.txt`

### 本次更新

- 构建脚本增加可选 Authenticode 签名参数：`-SignThumbprint` 或 `-SignPfxPath/-SignPfxPassword`。
- GitHub Release workflow 支持通过 `WINDOWS_SIGNING_PFX_BASE64` 和 `WINDOWS_SIGNING_PFX_PASSWORD` secrets 导入签名证书。
- 未配置证书时仍可正常无签名构建，并会明确打印 `Code signing skipped`。
- 保留 v1.0.9 的后 16 位范围修复、全 0 私钥跳过、私钥回写修复和 NVIDIA 自适应调参。

### 验证

- 无证书构建：`powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build-windows.ps1 -Version v1.0.10-test`，退出码 0。
- 构建日志确认未配置证书时跳过签名：`Code signing skipped: no certificate configured.`
- `Get-AuthenticodeSignature` 确认本地测试构建的 `shiyi.exe` 和 `start.exe` 为 `NotSigned`。
- `shiyi.exe --help` 正常运行。
- 构建和测试后未发现残留 `shiyi`、旧 `profanity*`、`TronStudio` 或 `start` 进程。

### 性能状态

RTX 3070 当前可信样本：

- 随机单目标 3 秒：351.127 MH/s
- 全 0 初始私钥、后 16 位向上、后 8 位地址测试 3 秒：333.413 MH/s

400 MH/s 目标尚未达成。本版本主要补齐签名构建通道，便于后续在 Application Control / 企业策略环境中稳定运行和测速。

## English

### Release Content

- Windows launcher: `start.exe`
- Native OpenCL generator: `shiyi.exe`
- Release asset file: `shiyi-v1.0.10.zip`
- Runtime OpenCL kernels: `kernels/*.cl`
- Default target files: `profanity.txt` and `runtime/targets.txt`
- Launcher helper: `start.bat`
- Package metadata: `README.md` and `VERSION.txt`

### Changes

- Added optional Authenticode signing parameters to the Windows build script: `-SignThumbprint` or `-SignPfxPath/-SignPfxPassword`.
- The GitHub Release workflow can import a signing certificate from `WINDOWS_SIGNING_PFX_BASE64` and `WINDOWS_SIGNING_PFX_PASSWORD` secrets.
- Builds still work unsigned when no certificate is configured, and clearly print `Code signing skipped`.
- Keeps the v1.0.9 last-16 range fix, zero-key skip, private-key reporting fix, and NVIDIA adaptive tuning.

### Verification

- No-certificate build: `powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build-windows.ps1 -Version v1.0.10-test`, exit code 0.
- Build log confirms unsigned fallback: `Code signing skipped: no certificate configured.`
- `Get-AuthenticodeSignature` confirms the local test `shiyi.exe` and `start.exe` are `NotSigned`.
- `shiyi.exe --help` runs successfully.
- No leftover `shiyi`, old `profanity*`, `TronStudio`, or `start` processes were found after build and tests.

### Performance Status

Current trusted RTX 3070 samples:

- Random single-target, 3 seconds: 351.127 MH/s
- All-zero initial key, last-16 upward, address last-8 test, 3 seconds: 333.413 MH/s

The 400 MH/s target has not been reached yet. This version focuses on the signing build path so future runs and benchmarks are more stable under Application Control or enterprise policy.
