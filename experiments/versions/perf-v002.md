# perf-v002

## 状态

已完成

## 优化目标

把自动调优从偏单一设备，改成更通用的跨设备策略，并建立 benchmark 自动落表能力。

## 涉及文件

- `profanity.cpp`
- `benchmark.ps1`
- `experiments/versions/perf-v002.md`

## 具体改动

1. 为 NVIDIA / AMD / Intel 设定不同的保守默认 work size
2. 自动把候选 work size 收敛到设备允许范围
3. 为不同厂商设置不同的 inverse-multiple 默认值
4. 增加 benchmark 脚本，把测速结果自动追加到汇总表

## 适用设备范围

- NVIDIA 独显
- AMD 独显
- Intel 集显 / 核显
- 其他 OpenCL 设备走保守兜底值

## 构建方式

Windows + Visual Studio 开发者命令环境 + 本地 OpenCL 导入库

## 测试命令

```powershell
.\profanity.x64.exe --matching TUqEg3dzVEJNQSVW2HY98z5X8SBdhmao8D --prefix-count 1 --suffix-count 1 --quit-count 1
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
- 速度: 60.128 MH/s

## 与上一版对比

- `perf-v001`: 59.312 MH/s
- `perf-v002`: 60.128 MH/s
- 提升: 0.816 MH/s，约 1.38%

## 结论

这版优化的重点不是激进提速，而是把自动调优改成更适合多厂商设备的策略，同时保留了当前机器上的小幅增益。后续可以继续沿着 kernel score 阶段减负、内存访问和迭代批量策略推进更明显的优化。
