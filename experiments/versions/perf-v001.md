# perf-v001

## 状态

基线版本

## 优化目标

建立性能优化的版本治理、实验记录和回溯机制。

## 涉及文件

- `OPTIMIZATION_PLAN.md`
- `experiments/README.md`
- `experiments/benchmark_summary.csv`
- `experiments/versions/perf-v001.md`

## 具体改动

1. 建立版本编号规范
2. 建立 benchmark 汇总表
3. 建立逐版本实验记录模板
4. 明确提交和回溯约束

## 适用设备范围

全部设备，属于流程基础设施，不依赖特定 GPU。

## 构建方式

Windows + Visual Studio 开发者环境 + 本地 OpenCL 导入库

## 测试命令

```powershell
./profanity.x64.exe --matching TUqEg3dzVEJNQSVW2HY98z5X8SBdhmao8D --prefix-count 1 --suffix-count 1 --quit-count 1
```

## 测试环境

- GPU: NVIDIA GeForce RTX 3070
- OpenCL: 系统 OpenCL.dll

## 测速结果

- 初始化: 3 秒
- 速度: 59.312 MH/s

## 与上一版对比

首个基线版本，无上一版。

## 结论

从这一版开始，后续性能优化都必须按编号推进并保留提交记录，确保可以定位最优版本对应代码。
