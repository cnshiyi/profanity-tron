# perf-v005

## 状态

已完成

## 优化目标

提高高显存 NVIDIA 设备上的持续吞吐，验证更大的 `inverse-multiple` 是否能摊薄调度开销并提升主工程速度。

## 涉及文件

- `profanity.cpp`
- `experiments/benchmark_summary.csv`
- `experiments/versions/perf-v005.md`

## 具体改动

1. 在自动调优中新增 `hasLargeBatchMemory` 档位
2. NVIDIA 且 OpenCL 报告全局内存不低于 7.5 GiB 时，将 `inverse-multiple` 从 `32768` 提高到 `49152`
3. 6 GiB 级 NVIDIA 设备仍保持 `32768`，避免把更大的 batch 推给内存较紧设备
4. 未采用 `work 1024`，因为实测触发 OpenCL `CL_OUT_OF_RESOURCES`

## 适用设备范围

主要适用于 NVIDIA 独显，且 OpenCL `CL_DEVICE_GLOBAL_MEM_SIZE` 至少约 7.5 GiB 的设备，例如 RTX 3070 8GB。AMD、Intel 和较小显存 NVIDIA 设备保持原有保守参数。

## 构建方式

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1 -Clean
```

## 测试命令

```powershell
.\profanity.x64.exe --matching TUqEg3dzVEJNQSVW2HY98z5X8SBdhmao8D --prefix-count 1 --suffix-count 1 --quit-count 1
```

说明：本轮先尝试 `benchmark.ps1`，但当前 PowerShell 环境中 `Start-Process` 因重复 `Path/PATH` 环境键失败，因此使用等效命令直接运行并解析同样字段。

## 测试环境

- GPU: NVIDIA GeForce RTX 3070
- OpenCL reported memory: 8589410304 bytes
- Compute units: 46
- OS/build: Windows, Visual Studio C++ build tools via `build.ps1`

## 测速结果

- work: 512
- inverse-multiple: 49152
- work-max: 12533760
- 初始化: 5 秒
- 速度: 63.164 MH/s
- 构建: 通过

## 与上一版对比

- `perf-v004` 主工程记录: 59.296 MH/s，初始化 3 秒，`inverse-multiple=32768`
- `perf-v005`: 63.164 MH/s，初始化 5 秒，`inverse-multiple=49152`
- 同一二进制下强制 `inverse-multiple=32768` 对照: 53.987 MH/s，初始化 3 秒
- 速度变化: +3.868 MH/s，约 +6.52%
- 代价: 初始化增加约 2 秒，单轮 batch 占用更大

## 结论

本轮有明确价值。对 RTX 3070 这类 8GB NVIDIA 卡，提高 `inverse-multiple` 到 `49152` 能提升持续吞吐，同时初始化时间仍在可接受范围内。后续可继续围绕不同显存档位扩展参数矩阵，但不应把 `work 1024` 作为默认值。
