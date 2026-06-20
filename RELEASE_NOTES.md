# v1.0.8

## 中文

### 发布内容

- Windows 图形启动器：`start.exe`
- 原生 OpenCL 生成器：`shiyi.exe`
- 发布资产文件：`shiyi-v1.0.8.zip`
- 运行时 OpenCL 内核：`kernels/*.cl`
- 默认目标文件：`profanity.txt` 和 `runtime/targets.txt`
- 启动辅助脚本：`start.bat`
- 包说明和版本信息：`README.md`、`VERSION.txt`

### 本次更新

- 默认目标地址修正为 0 到 9 各一行，移除误写入的 `[银行卡]` 占位文本。
- 启动器读取旧 `runtime/targets.txt` 时，如果检测到 `[银行卡]` 旧内容，会自动回退到新的默认目标，避免旧运行目录把错误内容带回来。
- 继续保留 v1.0.7 的目标文件 UTF-8 无 BOM 保存、BOM/空白清理、发布包 README UTF-8 模板复制。
- 继续保留 v1.0.5 的范围模式恢复、`shiyi.exe` 命名、内核源码打包、Smart App Control 误拦截修复。
- 本版本不发布未验证的性能参数调整；随机/default 模式 400 MH/s 目标继续迭代。

### 验证

- 已从源码干净构建 Windows 发布包。
- `profanity.txt`、`dist/profanity.txt`、`dist/runtime/targets.txt` 均为 10 行 0-9 默认目标。
- 三份目标文件内容完全一致，首字节均为 `54 54 54`，无 UTF-8 BOM。
- 发布 zip 内的 `profanity.txt` 和 `runtime/targets.txt` 均为 0-9 十行目标。
- 构建后未发现残留 `shiyi`、旧 `profanity*`、`TronStudio` 或 `start` 进程。

### 已知状态

400 MH/s 目标在指定初始私钥范围测试中已达到，但随机/default 模式仍低于目标，需要继续优化。

## English

### Release Content

- Windows launcher: `start.exe`
- Native OpenCL generator: `shiyi.exe`
- Release asset file: `shiyi-v1.0.8.zip`
- Runtime OpenCL kernels: `kernels/*.cl`
- Default target files: `profanity.txt` and `runtime/targets.txt`
- Launcher helper: `start.bat`
- Package metadata: `README.md` and `VERSION.txt`

### Changes

- Corrected the default target list to one line for each digit from 0 to 9, removing the accidental `[银行卡]` placeholder text.
- When the launcher reads an old `runtime/targets.txt`, it falls back to the new defaults if legacy `[银行卡]` text is detected, preventing old runtime directories from bringing the bad targets back.
- Keeps the v1.0.7 UTF-8 no-BOM target saving, BOM/whitespace cleanup, and UTF-8 package README template copy.
- Keeps the v1.0.5 range-mode restore, `shiyi.exe` naming, packaged kernel sources, and Smart App Control false-positive fix.
- This release does not ship the unverified performance parameter experiment; the 400 MH/s target for random/default modes remains active.

### Verification

- Clean Windows build from source.
- `profanity.txt`, `dist/profanity.txt`, and `dist/runtime/targets.txt` all contain the 10-line 0-9 default target list.
- All three target files match exactly, start with bytes `54 54 54`, and have no UTF-8 BOM.
- The release zip contains the corrected 0-9 target list in both `profanity.txt` and `runtime/targets.txt`.
- No leftover `shiyi`, old `profanity*`, `TronStudio`, or `start` processes were found after the build.

### Known Status

The 400 MH/s target is reached for the explicit initial-key range test, but random/default modes are still below target and need further optimization.
