# Tron Profanity

基于 GPU / OpenCL 的 Tron 靓号地址生成工具。

当前仓库按“源码优先、可审计优先”的方式维护，重点是尽量减少不透明产物和高风险能力。

## 当前状态

- Windows 命令行生成器输出为 `shiyi.exe`。
- Windows 桌面启动程序输出为 `start.exe`。
- 仓库只保留源码，不跟踪可执行文件、目标文件、库文件和 OpenCL 二进制缓存。
- 结果上传入口已移除，避免把私钥或地址材料发往外部。
- AMD 路径已移除，当前只维护 NVIDIA / 通用 OpenCL 主线。

## 功能范围

- 本地生成 Tron 靓号地址。
- 支持单地址或多地址匹配。
- 支持前缀、后缀、命中数量和输出文件配置。
- 不包含链上 RPC 调用、合约调用、交易广播逻辑或钱包导入导出自动化。

## 构建说明

运行前需要宿主机具备可用的 OpenCL 开发 / 运行环境。

### Windows

使用 Visual Studio C++ 工具链构建：

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1 -Clean
powershell -ExecutionPolicy Bypass -File .\build-client.ps1
```

构建完成后会生成：

```text
shiyi.exe
start.exe
```

### Linux

```bash
g++ Dispatcher.cpp KernelSources.cpp Mode.cpp precomp.cpp profanity.cpp SpeedSample.cpp -I./OpenCL/include -lOpenCL -O2 -o shiyi
```

### macOS

```bash
make
```

## 使用方法

Windows 命令行示例：

```powershell
.\shiyi.exe --matching profanity.txt --suffix-count 6 --quit-count 10 --output hits.txt
```

桌面客户端：

```powershell
.\start.exe
```

## 默认客户端配置

- 默认目标地址列表为 `TTTTTTTTTTTTTTTTTTTT11111111111111` 到 `TTTTTTTTTTTTTTTTTTTT99999999999999`。
- 默认后缀位数为 5。
- 默认生成数量为 999。
- 输出会写入本次临时文件，并追加到长期保存文件。

## 私钥范围控制

`--range-start`、`--range-end`、`--range-direction` 可以全部留空；留空时使用系统安全随机种子。

填写范围参数时，程序会按指定方向控制私钥前 64 位中的连续变化窗口。生成结果仍会做 TRON 地址格式和匹配条件校验；使用前仍建议独立校验 `private` 是否能推导出对应 `address`。

## 运行建议

- 如果机器上同时有集成显卡和独立显卡，可尝试 `--skip 1`，避免选错设备。
- 新机器或新驱动首次验证时，建议先用较低的 `--suffix-count`。
- 在真实资产场景下，务必独立验证生成出的密钥材料。
- 持有资产前，优先使用多签和独立校验流程。

## 仓库洁净策略

仓库默认不跟踪以下文件：

- 可执行文件
- 目标文件
- 静态库或导入库
- OpenCL 二进制缓存
- 截图和基准临时文件
- 明文结果文件
