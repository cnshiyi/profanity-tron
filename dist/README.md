## profanity-tron

- 当前仓库按源码优先维护，请不要信任来路不明的预编译可执行文件。
- 构建完成后请直接运行程序本体，例如：`profanity.exe --matching profanity.txt --suffix-count 6`
- 如果机器同时有集成显卡，可以尝试 `--skip 1`，避免选错设备。
- `--post` 已因安全原因禁用，因为它可能泄露私钥。
- OpenCL 二进制缓存加载已因安全原因移除。
- 如果 Windows 提示 `vcruntime140_1.dll` 相关错误，请安装 Visual Studio 运行时：[https://visualstudio.microsoft.com/zh-hans/vs/](https://visualstudio.microsoft.com/zh-hans/vs/)
