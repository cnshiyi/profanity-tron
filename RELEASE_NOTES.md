# v1.0.6

## 中文

### 发布内容

- Windows 图形启动器：`start.exe`
- 原生 OpenCL 生成器：`shiyi.exe`
- 发布资产文件：`shiyi-v1.0.6.zip`
- 运行时 OpenCL 内核：`kernels/*.cl`
- 默认目标文件：`profanity.txt` 和 `runtime/targets.txt`
- 启动辅助脚本：`start.bat`
- 包说明和版本信息：`README.md`、`VERSION.txt`

### 本次更新

- 文档改为中英双语，包括根目录 README、发布包 README、发布说明和版本记录。
- GitHub Release 资产文件名改为带版本号的 `shiyi-v1.0.6.zip`。
- 保留 v1.0.5 的范围模式恢复、`shiyi.exe` 命名修复、内核源码打包和 Smart App Control 误拦截修复。
- 启动器已连接初始私钥、方向、位数到原生范围模式；指定位数限制为 1-16 个十六进制位。
- 本地 benchmark 工具会串行运行并清理同目录残留进程。

### 验证

- 已从源码干净构建 Windows 发布包。
- `shiyi.exe --help` 可运行，没有触发 Windows Application Control 拦截。
- 发布 zip 包含 `kernels/` 和 `runtime/` 目录。
- 启动器烟测：可启动并关闭。
- 进程残留检查：运行后没有残留 `shiyi`、旧 `profanity*`、`TronStudio` 或 `start` 进程。
- RTX 3070 当前基线：
  - 随机单目标：383.894 MH/s
  - 默认多目标：382.824 MH/s
  - 指定初始私钥范围：419.475 MH/s

### 已知状态

400 MH/s 目标在指定初始私钥范围测试中已达到，但随机/默认模式仍低于目标，需要继续优化。

## English

### Release Content

- Windows launcher: `start.exe`
- Native OpenCL generator: `shiyi.exe`
- Release asset file: `shiyi-v1.0.6.zip`
- Runtime OpenCL kernels: `kernels/*.cl`
- Default target files: `profanity.txt` and `runtime/targets.txt`
- Launcher helper: `start.bat`
- Package metadata: `README.md` and `VERSION.txt`

### Changes

- Documentation is now bilingual Chinese/English, including the root README, package README, release notes, and version record.
- The GitHub Release asset filename now includes the version: `shiyi-v1.0.6.zip`.
- Keeps the v1.0.5 range-mode restore, `shiyi.exe` package naming fix, packaged kernel sources, and Smart App Control false-positive fix.
- Launcher fields for initial private key, direction, and digit count are wired to native range mode. Fixed-digit mode is limited to 1-16 hex digits.
- The local benchmark helper runs serially and cleans same-folder residual generator processes.

### Verification

- Clean Windows build from source.
- `shiyi.exe --help` runs without Windows Application Control blocking.
- Release zip contains `kernels/` and `runtime/` directories.
- Launcher smoke test: start and close.
- Residual process check: no leftover `shiyi`, old `profanity*`, `TronStudio`, or `start` processes after runs.
- Current measured baseline on RTX 3070:
  - Random single-target: 383.894 MH/s
  - Default multi-target: 382.824 MH/s
  - Explicit initial-key range: 419.475 MH/s

### Known Status

The 400 MH/s target is reached for the explicit initial-key range test, but random/default modes are still below target and need further optimization.
