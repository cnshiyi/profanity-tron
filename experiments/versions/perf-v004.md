# perf-v004

## 状态

已完成

## 优化目标

验证新的两段式评分架构是否比“每个候选都走完整重路径”的旧思路明显更快。

## 涉及文件

- `prototype/standalone_score_benchmark.cpp`
- `prototype/README.md`
- `experiments/versions/perf-v004.md`

## 具体改动

1. 新增独立原型 `baseline_full_score`
2. 新增独立原型 `aggressive_prefilter_score`
3. 用 GPU 上的独立基准对比“全量重路径”与“先廉价筛选再少量重路径”
4. 不改主工程匹配正确性，只验证新架构方向

## 适用设备范围

理论上适用于全部 OpenCL GPU，尤其适合那些在重计算路径上吞吐高、但分支和内存浪费明显的设备。

## 构建方式

```powershell
cmd /c "call <VsDevCmd.bat> -arch=x64 >nul && cl /nologo /EHsc /O2 /I .\OpenCL\include /Fe:prototype\standalone_score_benchmark.exe prototype\standalone_score_benchmark.cpp OpenCL.lib"
```

## 测试命令

```powershell
.\prototype\standalone_score_benchmark.exe
```

## 测试环境

- GPU: NVIDIA GeForce RTX 3070
- Vendor: NVIDIA Corporation

## 测速结果

- baseline_full_score_ms: 17.639
- aggressive_prefilter_ms: 7.005
- speedup: 2.518x

## 与上一版对比

- `perf-v003` 是主工程内的激进初始化优化尝试，结果为负收益
- `perf-v004` 是独立评分架构原型，对核心评分思路验证得到约 `2.52x` 的局部加速潜力

## 结论

这轮是目前最重要的方向性结果。它说明真正的潜力不在初始化细节，而在评分架构本身:

1. 不应该让所有候选都走完整重路径
2. 应该先做廉价筛选
3. 只有少量候选再进入完整编码 / 匹配路径

后续主工程优化应优先围绕“两段式评分”落地，而不是继续围着初始化清零做文章。
