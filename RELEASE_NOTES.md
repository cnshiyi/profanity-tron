# v1.0.17

## 中文

### 发布内容

- Windows 图形启动器：`start.exe`
- 原生 OpenCL 生成器：`shiyi.exe`
- 发布资产文件：`shiyi-v1.0.17.zip`
- 运行时 OpenCL 内核：`kernels/*.cl`
- 默认目标文件：`profanity.txt` 和 `runtime/targets.txt`
- 启动辅助脚本：`start.bat`
- 包说明和版本信息：`README.md`、`VERSION.txt`

### 本次更新

- 启动器读取旧 `runtime/targets.txt` 时会校验每一行目标。
- 如果旧目标包含 `[银行卡]`、`0` 或其他 TRON/Base58 不支持的字符，界面会自动回退到合法默认目标。
- 保存目标和开始生成前会阻止非法目标，避免旧占位文本或含 `0` 的地址再次写回运行目录。
- 原生 `shiyi.exe` 对直接传入的 34 位 TRON 地址增加 Base58 校验，含 `0` 的目标会报错退出。
- Windows 构建脚本在打包前校验默认目标，防止发布包混入非法默认目标。

### 验证

- `dist/profanity.txt` 和 `dist/runtime/targets.txt` 均为 `1-9 + A` 十行默认目标。
- `shiyi-v1.0.17.zip` 包内包含 `shiyi.exe`、`start.exe`，不包含旧名 `profanity.x64.exe`。
- zip 包内 `profanity.txt` 和 `runtime/targets.txt` 均不含 `[银行卡]` 或 `0` 默认目标。
- 直接传入 `TTTTTTTTTTTTTTTTT00000000000000000` 会被 `shiyi.exe` 拒绝。
- 最终合法目标 5 秒冒烟测试通过：352.834 MH/s。
- 构建/测试后未发现残留 `shiyi`、旧 `profanity*`、`TronStudio` 或 `start` 进程。

### 性能状态

本版本修复默认目标和非法目标回灌问题，不声明 400 MH/s 已达成。400 MH/s 优化目标继续保持打开，后续仍应优先优化 iterate/inverse 主内核。

## English

### Release Content

- Windows launcher: `start.exe`
- Native OpenCL generator: `shiyi.exe`
- Release asset file: `shiyi-v1.0.17.zip`
- Runtime OpenCL kernels: `kernels/*.cl`
- Default target files: `profanity.txt` and `runtime/targets.txt`
- Launcher helper: `start.bat`
- Package metadata: `README.md` and `VERSION.txt`

### Changes

- The launcher now validates every target loaded from an existing `runtime/targets.txt`.
- If an old target contains `[银行卡]`, `0`, or any other character unsupported by TRON/Base58, the launcher falls back to the clean default target list.
- Saving targets and starting generation now reject invalid targets, preventing legacy placeholders or `0` targets from being written back to runtime files.
- Native `shiyi.exe` now validates direct 34-character TRON target arguments and rejects targets containing `0`.
- The Windows build script validates default targets before packaging, preventing invalid defaults from entering release packages.

### Verification

- `dist/profanity.txt` and `dist/runtime/targets.txt` both contain the 10-line `1-9 + A` default target set.
- `shiyi-v1.0.17.zip` contains `shiyi.exe` and `start.exe`, with no legacy `profanity.x64.exe`.
- Packaged `profanity.txt` and `runtime/targets.txt` contain no `[银行卡]` text and no `0` default target.
- Passing `TTTTTTTTTTTTTTTTT00000000000000000` directly to `shiyi.exe` is rejected.
- Final valid-target 5-second smoke test passed: 352.834 MH/s.
- No leftover `shiyi`, old `profanity*`, `TronStudio`, or `start` processes were found after build and tests.

### Performance Status

This release fixes default-target and invalid-target persistence issues. It does not claim the 400 MH/s goal has been reached. The 400 MH/s optimization target remains open, and the next optimization pass should still prioritize the iterate/inverse kernels.
