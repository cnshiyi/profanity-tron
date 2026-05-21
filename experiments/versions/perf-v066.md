# perf-v066 - all-exact suffix2 唯一目标直达分支（未采纳）

时间：2026-05-21

## 目标

继续优化真实可用路径 `matchingCount > 1 && prefixCount <= 1 && suffixCount in [2,12]`，
不引入 benchmark-only 逻辑，且保持实际命中与 `private -> TRON address` 独立推导链路不变。

## 新方向

在已采纳 `perf-v065`（两阶段尾部索引 + `suffixTail2Allowed` 早拒绝）基础上，尝试减少 all-exact 命中后的目标扫描：
- Host 侧为每个两字符桶 `(idx0, idx1)` 预计算唯一目标索引（唯一则记录目标号，冲突则标记多目标）；
- Kernel all-exact 分支在通过 `suffixTail2Allowed` 后：
  - 若桶内唯一目标，则直接对该目标做 `base58_tail_match_indexed`；
  - 否则回退原有全目标循环。

该尝试只作用于真实多目标 `suffix2-12` all-exact 路径。

## Benchmark（可复现）

统一命令：

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 5
```

基线（保留实现 perf-v065）：
- Final: `359.539 MH/s`
- Initialization: `21s`

候选（perf-v066 唯一目标直达）：
- Final: `359.961 MH/s`
- Initialization: `22s`

结果：`+0.12%`，属于噪声级别，未采纳。

## 回退与实际命中测试（保留路径）

回退到保留实现后执行：

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --quit-count 3 --output .\runtime\perf_v066_revert_hits.txt
```

结果：
- `Validated hits: 3`
- `Saved hits: 3`

## private 独立推导校验

对 `runtime\perf_v066_revert_hits.txt` 使用独立流程 `ecdsa + Keccak-256 + Base58Check` 复算：
- `verified=3`
- `mismatched=0`

## 结论

- `perf-v066` 仅有噪声级提升，不满足采纳标准，已拒绝并回退，不保留失败实现。
- 当前保留实现仍为 `perf-v065`，真实命中与 `private -> TRON address` 独立推导校验通过。
- 失败原因：唯一桶直达收益被新增映射读取/分支开销抵消，净增益不足。
