## profanity-tron

- 当前仓库按源码优先维护，请不要信任来源不明的预编译可执行文件。
- 构建完成后请直接运行程序本体，例如：`shiyi.exe --matching profanity.txt --suffix-count 6`
- 如果机器同时有集成显卡和独立显卡，可以尝试 `--skip 1`，避免选错设备。
- `--post` 已移除，不再支持任何结果上传入口。
- OpenCL 二进制缓存默认启用，仅用于减少重复编译时间；可通过 `--no-cache` 禁用。
- 如果 Windows 提示 `vcruntime140_1.dll` 相关错误，请安装 Visual Studio 运行时：[https://visualstudio.microsoft.com/zh-hans/vs/](https://visualstudio.microsoft.com/zh-hans/vs/)
