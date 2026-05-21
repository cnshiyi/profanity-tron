# perf-v056 - 多目标 suffix2-12 按最后一位候选分桶（未采纳）

时间：2026-05-21

## 目标

在真实可用路径 `matchingCount > 1 && prefixCount <= 1 && suffixCount in [2,12]` 中，
减少每个 work-item 的逐目标全量匹配成本。

约束：
- 不引入仅 benchmark 可用路径。
- 保持私钥到地址生成与验证链路不变。

## 新方向

尝试把多目标后缀匹配按 Base58 最后一位做候选分桶：
- Host 侧预计算 `suffixIndex(0..57) -> candidate target indices`。
- Kernel 在该分支先算 `tailIndices[0]`，只遍历该桶候选目标，再做完整尾部匹配。

与 perf-v052/perf-v055 不同点：
- 不是“布尔早退过滤”；而是“缩小逐目标遍历集合”。

## Benchmark（可复现）

A/B 命令（完全一致）：

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 5
```

基线（保留实现 / perf-v051 路径）：
- Final: `386.239 MH/s`
- Initialization: `20s`

候选（perf-v056）：
- Final: `338.231 MH/s`
- Initialization: `20s`

结果：`-12.43%`，拒绝。

## 回退后真实命中测试

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --quit-count 3 --output .\runtime\perf_v056_revert_hits.txt
```

结果：
- `Validated hits: 3`
- `Saved hits: 3`

## 私钥独立推导校验

对 `runtime\perf_v056_revert_hits.txt` 使用独立 Python（`ecdsa + Keccak-256 + base58check`）校验：
- `verified=3`
- `mismatched=0`

## 失败原因

候选分桶引入了额外常量内存读取和索引间接访问，GPU 上收益不足以覆盖开销，整体吞吐明显下降。

## 结论

- perf-v056 方向记录为失败，不采纳。
- 代码已回退并保留 perf-v051 已采纳实现。
