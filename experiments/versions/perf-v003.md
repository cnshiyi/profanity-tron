# perf-v003

## 状态

已完成

## 优化目标

尝试用更激进的方式减少初始化阶段对结果缓冲区的重复清零开销。

## 涉及文件

- `kernel_profanity.hpp`
- `profanity.cpp`
- `build.ps1`
- `benchmark.ps1`

## 具体改动

1. `profanity_init` 中不再让每个 work-item 都循环清空整个结果区
2. 改为只有前 `PROFANITY_MAX_SCORE + 1` 个 work-item 对结果区逐项清零
3. 保持跨设备自动调优逻辑
4. 修复并稳定 `build.ps1` 构建链路，方便后续连续实验

## 适用设备范围

理论上适用于所有 OpenCL 设备，但需要实测验证是否真的收益。

## 构建方式

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1 -Clean
```

## 测试命令

```powershell
powershell -ExecutionPolicy Bypass -File .\benchmark.ps1 -Version perf-v003 -BuildFirst -Notes "aggressive init clear reduction"
```

## 测试环境

- GPU: NVIDIA GeForce RTX 3070
- 显存: 8589410304 bytes
- OpenCL: Windows System32 OpenCL.dll

## 测速结果

- work: 512
- inverse-multiple: 32768
- work-max: 8355840
- 初始化: 3 秒
- 速度: 58.517 MH/s

## 与上一版对比

- `perf-v002`: 60.128 MH/s
- `perf-v003`: 58.517 MH/s
- 变化: -1.611 MH/s，约 -2.68%

## 结论

这轮激进优化在当前设备上是负收益，不应作为默认路径继续放大。它说明初始化清零并不是当前最值得优先攻击的瓶颈，后续更应该把精力放在 `score` 阶段的 Base58 / SHA256 路径裁剪，或者重新组织匹配流程，避免每个候选都走完整编码流程。
