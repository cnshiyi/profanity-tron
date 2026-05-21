# perf-v067 - exact-tail 指针步进比较（未采纳）

时间：2026-05-21

## 目标

继续优化真实可用路径 `matchingCount > 1 && prefixCount <= 1 && suffixCount in [2,12]`，
不引入 benchmark-only 逻辑，并保持实际命中与 `private -> TRON address` 独立推导链路不变。

## 新方向

在已采纳 `perf-v065`（两阶段尾部索引 + `suffixTail2Allowed` 早拒绝）基础上，尝试降低 exact-tail 目标循环中的索引开销：
- 新增 exact-tail 指针步进比较 helper；
- 将 all-exact 分支和 mixed 分支中的 exact 匹配从 `j * 12` 基址计算改为按目标指针步进比较；
- generic 掩码匹配路径保持不变。

该尝试只作用于真实多目标 `suffix2-12` 生成路径。

## Benchmark（可复现）

统一命令：

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 5
```

基线（保留实现 perf-v065）：
- Final: `358.787 MH/s`
- Initialization: `22s`

候选（perf-v067 指针步进比较）：
- Final: `359.964 MH/s`
- Initialization: `22s`

结果：`+0.33%`，属于噪声级，未采纳。

## 回退与实际命中测试（保留路径）

回退到保留实现后执行：

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --quit-count 3 --output .\runtime\perf_v067_revert_hits.txt
```

结果：
- `Validated hits: 3`
- `Saved hits: 3`

## private 独立推导校验

对 `runtime\perf_v067_revert_hits.txt` 使用独立流程 `ecdsa + Keccak-256 + Base58Check` 复算：
- `verified=3`
- `mismatched=0`

## 结论

- `perf-v067` 仅有噪声级增益，不满足采纳标准，已拒绝并回退，不保留失败实现。
- 当前保留实现仍为 `perf-v065`，真实命中与 `private -> TRON address` 独立推导校验通过。
- 失败原因：减少索引计算的收益被额外 helper/指针读取开销抵消，净增益不足。
