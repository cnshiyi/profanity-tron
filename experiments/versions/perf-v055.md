# perf-v055 - 多目标 suffix2-12 复用 tailIndices[0] 位图早筛（未采纳）

时间：2026-05-21

## 目标

在真实可用生成路径 `matchingCount > 1 && prefixCount <= 1 && suffixCount in [2,12]` 中减少无效目标比较：
- 仅在已计算 `tailIndices` 的前提下做低成本早筛。
- 不引入只能 benchmark、不能实际产出地址的路径。

## 新方向说明

相对 perf-v051，本轮尝试：
- Host 侧把 `suffix1Allowed` 预计算条件从 `suffixCount == 1` 扩展到 `suffixCount >= 1`。
- Kernel 在多目标 `suffix2-12` 分支中，先计算 `tailIndices` 后复用 `tailIndices[0]`：
  - 若 `suffix1Allowed[tailIndices[0]] == 0`，直接返回；
  - 否则进入逐目标 `base58_tail_match_data`。

该方向避免了 perf-v052 中额外调用 `base58_last_index_from_ethhash` 的额外哈希计算，属于真实生成路径内优化尝试。

## Benchmark（可复现）

A/B 命令（保持一致）：

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 5
```

基线（保留实现 / perf-v051 路径）：
- Final: `385.372 MH/s`
- Initialization: `20s`

候选（perf-v055）：
- Final: `385.262 MH/s`
- Initialization: `20s`

结果：`-0.03%`，轻微回退。

## 实际命中测试（保留实现）

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --quit-count 3 --output .\runtime\perf_v055_revert_hits.txt
```

结果：
- `Validated hits: 3`
- `Saved hits: 3`

## 私钥独立推导校验

对 `runtime\perf_v055_revert_hits.txt` 使用独立 Python（`ecdsa + Keccak-256 + base58check`）校验：
- `verified=3`
- `mismatched=0`

## 失败原因

`suffix2-12` 分支在当前 GPU 编译结果下，新增的位图早筛判断未带来可测收益，分支与内存访问开销抵消了潜在减少的逐目标比较成本。

## 结论

- perf-v055 方向记录为失败，不采纳。
- 保留当前真实可用代码（perf-v051 路径）。
