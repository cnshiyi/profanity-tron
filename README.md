# Tron Profanity

基于 GPU / OpenCL 的 Tron 靓号地址生成工具。

本仓库当前按“源码优先”的方式维护，重点是尽量减少不透明产物和高风险能力。

## 当前状态

- 仓库只保留源码，不再跟踪预编译可执行文件、库文件、批处理脚本或 OpenCL 二进制缓存。
- 自动联网回传结果的能力已禁用，因为它可能泄露私钥。
- 明文私钥写入文件的能力已移除。
- 程序命中结果时默认只在控制台输出地址，不再直接打印私钥。

## 功能范围

- 本地生成 Tron 靓号地址
- 不包含链上 RPC 调用
- 不包含合约调用
- 不包含交易广播逻辑
- 不包含钱包导入导出自动化

## 安全边界

虽然高风险入口已经尽量移除，但不能把任何 C++ / OpenCL 项目描述成“绝对零风险”。仍需注意：

- GPU 驱动、OpenCL 运行时、编译器和系统环境本身属于外部信任边界
- 终端历史、日志、截图、录屏仍可能暴露敏感运行信息
- 如果涉及真实资金，必须做独立验证和多签隔离

## 构建说明

运行前需要宿主机已经具备可用的 OpenCL 开发 / 运行环境。

### Windows

使用 Visual Studio C++ 工具链构建，并从系统 SDK 或显卡驱动环境中提供 OpenCL 导入库。

如果当前环境里无法解析 `OpenCL.lib`，说明宿主机还没有准备好对应的 OpenCL 开发 / 链接环境；这不是仓库内置文件缺失，而是本机构建前置条件未满足。

### Linux

示例：

```bash
g++ Dispatcher.cpp Mode.cpp precomp.cpp profanity.cpp SpeedSample.cpp -I./OpenCL/include -lOpenCL -O2 -o profanity.x64
```

### macOS

示例：

```bash
make
```

## 使用方法

程序主要参数如下：

```text
Usage: ./profanity [OPTIONS]

  Help:
    --help              Show help information

  Modes with arguments:
    --matching          Matching input, file or single address.

  Matching configuration:
    --prefix-count      Minimum number of prefix matches, default 0
    --suffix-count      Minimum number of suffix matches, default 6
    --quit-count        Exit the program when the generated number is greater than, default 0

  Device control:
    --skip              Skip GPU device given by index
```

示例：

```bash
./profanity --matching profanity.txt
./profanity --matching profanity.txt --skip 1
./profanity --matching profanity.txt --suffix-count 6
./profanity --matching profanity.txt --prefix-count 2 --suffix-count 6 --quit-count 1
./profanity --matching TUqEg3dzVEJNQSVW2HY98z5X8SBdhmao8D --prefix-count 2 --suffix-count 4 --quit-count 1
```

## 匹配输入

`--matching` 支持两种输入：

- 单个 Tron 地址
- 每行一个模式的文本文件

示例匹配文件见 [profanity.txt](C:/Users/399/Desktop/profanity-tron/profanity.txt)。

## 运行建议

- 如果机器上同时有集成显卡和独立显卡，可以尝试 `--skip 1`，避免选错设备。
- 新机器或新驱动环境第一次验证时，建议先用较低的 `--suffix-count`。
- 在真实资金场景下，务必独立验证生成出的密钥材料。
- 如果真要持有资产，优先使用多签和独立校验流程。

## 仓库洁净策略

本仓库有意不跟踪以下文件：

- 可执行文件
- 目标文件
- 静态库或导入库
- 批处理启动脚本
- OpenCL 二进制缓存
- 基准测试日志
- 明文结果文件
