# perf-v027

## 状态

已完成

## 优化目标

把 `lianhao` 的“单目标 + 单后缀”快路径正式落地到主工程评分 kernel 中。

## 涉及文件

- `kernel_profanity.hpp`
- `kernel_sha256.hpp`
- `Dispatcher.cpp`
- `profanity.cpp`
- `experiments/versions/perf-v027.md`

## 具体改动

1. 为 `matchingCount == 1 && prefixCount <= 1 && suffixCount == 1` 增加专用快路径
2. 在快路径中直接基于 ethhash 推导 Base58 尾字符索引
3. 避免在该特例下走完整地址编码与通用匹配循环
4. 保留通用路径作为其他模式和更复杂条件的后备逻辑

## 适用设备范围

- 主要面向当前保留支持的 NVIDIA / Intel OpenCL 设备
- 当前收益验证基于 NVIDIA RTX 3070

## 构建方式

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1 -Clean
```

## 测试命令

```powershell
powershell -ExecutionPolicy Bypass -File .\benchmark.ps1 -Version perf-v027 -Notes "lianhao single-target single-suffix fast path"
```

## 测试环境

- GPU: NVIDIA GeForce RTX 3070
- 显存: 8589410304 bytes
- OpenCL: Windows System32 OpenCL.dll

## 测速结果

- work: 128
- inverse-multiple: 237568
- work-max: 60579840
- 初始化: 23 秒
- 速度: 120.565 MH/s

## 与上一版对比

- 方向上承接“模式专用评分 kernel”的思路
- 当前这版比历史 `58~60 MH/s` 时代明显更高，但相较当前更高的 `perf-v023 ~ 174.588 MH/s` 仍然落后

## 结论

这轮最大的价值不是“绝对最快”，而是把 `lianhao` 专用评分快路径真正落地进主工程。它证明专用模式 kernel 的方向是成立的，但当前这版快路径实现还没有击败现有高水位版本，后续需要继续优化：

1. 更精确的尾字符推导逻辑
2. 更少的 SHA / Base58 近似成本
3. 把 `shunzi / gaofang` 也拆成专用路径，避免统一评分拖累
