# Tron Profanity

基于 GPU / OpenCL 的 Tron 靓号地址生成工具。

当前仓库按“源码优先、可审计优先”的方式维护，重点是尽量减少不透明产物和高风险能力。

## 当前状态

- 仓库只保留源码，不再跟踪可执行文件、目标文件、库文件和 OpenCL 二进制缓存。
- 结果上传入口已移除，避免把私钥或地址材料发往外部。
- 默认只在控制台输出生成结果，不写入明文结果文件。
- AMD 路径已移除，当前只维护 NVIDIA / 通用 OpenCL 主线。

## 功能范围

- 本地生成 Tron 靓号地址
- 不包含链上 RPC 调用
- 不包含合约调用
- 不包含交易广播逻辑
- 不包含钱包导入导出自动化

## 安全边界

这个项目已经尽量收紧了高风险入口，但不能把任何 C++ / OpenCL 项目描述成“绝对零风险”。仍需注意：

- GPU 驱动、OpenCL 运行时、编译器和操作系统都属于外部信任边界
- 终端历史、日志、截图和录屏仍可能泄露敏感运行信息
- 涉及真实资金时，必须做独立验证和多签隔离

## 构建说明

运行前需要宿主机具备可用的 OpenCL 开发 / 运行环境。

### Windows

使用 Visual Studio C++ 工具链构建：

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1 -Clean
```

如果当前环境无法解析 `OpenCL.lib`，说明宿主机的 OpenCL 开发 / 链接环境未准备好，不是仓库内文件缺失。

### Linux

```bash
g++ Dispatcher.cpp KernelSources.cpp Mode.cpp precomp.cpp profanity.cpp SpeedSample.cpp -I./OpenCL/include -lOpenCL -O2 -o profanity.x64
```

### macOS

```bash
make
```

## 使用方法

```text
Usage: ./profanity [OPTIONS]

  帮助:
    --help              显示帮助信息

  运行模式:
    --matching          匹配输入，文件或单个地址

  匹配配置:
    --prefix-count      最少前缀匹配位数，默认 0
    --suffix-count      最少后缀匹配位数，默认 6
    --quit-count        当满足条件时退出，默认 0
    --output            将验证通过的命中结果追加写入文件

  私钥范围控制:
    --range-start       起始私钥，64 位十六进制；留空则使用安全随机模式
    --range-end         结束私钥，64 位十六进制；必须和 --range-start 同时填写
    --range-direction   搜索方向，up 或 down；留空默认为 up

  设备控制:
    --skip              按索引跳过 GPU
    --no-cache          禁用 OpenCL 二进制缓存
```

示例：

```bash
./profanity --matching profanity.txt
./profanity --matching profanity.txt --skip 1
./profanity --matching profanity.txt --suffix-count 6
./profanity --matching profanity.txt --prefix-count 2 --suffix-count 6 --quit-count 1
./profanity --matching TUqEg3dzVEJNQSVW2HY98z5X8SBdhmao8D --prefix-count 2 --suffix-count 4 --quit-count 1
./profanity --matching TUqEg3dzVEJNQSVW2HY98z5X8SBdhmao8D --prefix-count 0 --suffix-count 1 --quit-count 3 --range-start 58958afa84300000e8ce89f9a5b808e8ee611ad00deaf3745a5305e0a3e1d688 --range-end 58958afa844fffffe8ce89f9a5b808e8ee611ad00deaf3745a5305e0a3e1d688 --range-direction up --output hits.txt
```

## 私钥范围控制

`--range-start`、`--range-end`、`--range-direction` 可以全部留空；留空时仍然使用系统安全随机种子。

填写范围参数时，程序会按指定方向控制私钥前 64 位中的连续变化窗口。当前版本要求变化窗口结束在第 16 个 hex 位，适合控制类似 `58958afa84300000...` 到 `58958afa844fffff...` 这种形态。

生成结果仍会做 TRON 地址格式和匹配条件校验；使用前仍建议独立校验 `private` 是否能推导出对应 `address`。

## 匹配输入

`--matching` 支持两种输入：

- 单个 Tron 地址
- 每行一个模式的文本文件

示例匹配文件见 [profanity.txt](/C:/Users/399/Desktop/profanity-tron/profanity.txt)。

## 运行建议

- 如果机器上同时有集成显卡和独立显卡，可尝试 `--skip 1`，避免选错设备
- 新机器或新驱动首次验证时，建议先用较低的 `--suffix-count`
- 在真实资产场景下，务必独立验证生成出的密钥材料
- 持有资产前，优先使用多签和独立校验流程

## 仓库洁净策略

仓库默认不跟踪以下文件：

- 可执行文件
- 目标文件
- 静态库或导入库
- OpenCL 二进制缓存
- 截图和基准临时文件
- 明文结果文件
