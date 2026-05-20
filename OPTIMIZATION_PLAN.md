# 性能优化与版本回溯规范

本项目的性能优化按“可复现、可比较、可回退”的方式推进。

## 分支策略

- 主优化分支: `codex/perf-lab`
- 所有性能相关提交都保留在该分支上
- 当某一轮优化证明无收益或有回退价值时，保留提交记录，不直接覆盖历史

## 版本编号

从当前基线开始，按顺序推进:

- `perf-v001`
- `perf-v002`
- `perf-v003`
- ...

每一轮优化都必须:

1. 有唯一版本编号
2. 有对应提交
3. 有实验记录
4. 有测速结果
5. 有结论

## 每轮记录要求

每个版本在 `experiments/versions/` 下保留一份记录文件，文件名格式:

- `perf-v001.md`
- `perf-v002.md`

每份记录至少包含:

1. 优化目标
2. 涉及文件
3. 具体改动
4. 适用设备范围
5. 构建方式
6. 测试命令
7. 测试环境
8. 测速结果
9. 与上一版对比
10. 结论

## 提交规范

提交信息格式:

- `perf(v001): 建立优化基线与记录体系`
- `perf(v002): 调整跨设备 work size 自动调优`
- `perf(v003): 减少 score 阶段不必要计算`

## 结果汇总

总表维护在:

- `experiments/benchmark_summary.csv`

字段:

- version
- date
- device_name
- opencl_vendor
- command
- worksize_local
- inverse_multiple
- worksize_max
- speed_mhs
- init_seconds
- build_ok
- notes

## 约束

- 不允许覆盖历史结果
- 不允许跳号复用版本名
- 不允许只改代码不留记录
- 不允许只记结果不提交代码

## 当前基线定义

当前基线为:

- 安全收紧完成
- Windows 本机构建打通
- Tron 地址合法性已验证

后续所有性能优化都以这个基线为起点继续推进。
