# perf-v053 - 多目标 suffix2-12 固定展开尾部匹配（未采纳）

时间：2026-05-21

## 目标

在真实可用生成路径 `matchingCount > 1 && prefixCount <= 1 && suffixCount in [2,12]` 中，
减少每个目标尾部匹配时的运行时循环与索引计算开销。

约束：
- 不改地址生成算法。
- 不新增仅 benchmark 可跑、实际不可用路径。

## 新方向说明

相对 perf-v051，本轮尝试把多目标 suffix2-12 的逐位匹配从运行时循环改为固定展开比较链：
- `base58_tail_match_data_fixed(...)` 按 `suffixCount` 走固定分支比较；
- 期望通过减少循环控制开销提升吞吐。

## 结果

该方向在 GPU 上出现严重回退（3~16 MH/s），不采纳。

代码已回退到 perf-v051 保留版本（多目标 suffix1 位图仍保留，suffix2-12 维持原实现）。

## Benchmark（可复现）

A/B 命令（保持一致）：

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 5
```

基线（保留代码 / perf-v051 路径）：
- Final: `385.372 MH/s`
- Initialization: `20s`

候选（perf-v053 尝试版）：
- Final: `3.622 MH/s`
- Initialization: `45s`

结论：`-99%` 量级回退，直接拒绝。

## 实际命中测试（保留代码）

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --quit-count 5 --output .\runtime\perf_v053_revert_hits.txt
```

结果：
- `Validated hits: 5`
- `Saved hits: 5`

## 私钥独立推导校验

对 `runtime\perf_v053_revert_hits.txt` 使用独立 Python（`ecdsa + keccak + base58check`）校验：
- `verified=5`
- `mismatched=0`

## 失败原因

固定展开比较链引入了显著寄存器/指令压力，且破坏了原路径中“先比较最后一位尽快失败返回”的轻量特性，导致真实路径吞吐大幅下降。

## 结论

- perf-v053 方向记录为失败，不采纳。
- 保留当前真实可用代码（perf-v051 路径），继续下一轮新方向。
