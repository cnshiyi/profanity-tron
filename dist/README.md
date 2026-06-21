# profanity-tron Windows package / Windows 发布包

## 中文

双击 `start.exe` 打开 Windows 图形启动器。命令行生成器文件名为 `shiyi.exe`，启动器会自动调用它。

### 文件

- `start.exe`：图形启动器
- `shiyi.exe`：OpenCL 生成器
- `runtime\targets.txt`：默认目标列表
- `profanity.txt`：默认目标列表副本
- `kernels\*.cl`：OpenCL 内核源码

### 提示

- 目标地址每行一个，保存时会自动清理空行和隐藏 BOM 字符。
- 指定位数模式限制为 1-16 个十六进制位；留空则使用随机模式。
- 全 0 初始私钥会自动跳过无效私钥 `0`。
- 当前包已移除旧的静态 curl/网络链接依赖，降低 `shiyi.exe` 被 Smart App Control 误拦截的风险。

## English

Run `start.exe` to open the Windows launcher. The native generator is `shiyi.exe` and is launched automatically.

### Files

- `start.exe`: graphical launcher
- `shiyi.exe`: OpenCL generator used by the launcher
- `runtime\targets.txt`: default target list
- `profanity.txt`: copy of the default target list
- `kernels\*.cl`: OpenCL kernels loaded by the generator

### Notes

- Targets are one per line. Blank lines and hidden BOM characters are cleaned automatically when saving.
- Fixed-digit mode is limited to 1-16 hex digits. Leave it blank for random mode.
- An all-zero initial private key automatically skips invalid private key `0`.
- Obsolete static curl/network link dependencies were removed to reduce Smart App Control false positives on the newly built `shiyi.exe`.
