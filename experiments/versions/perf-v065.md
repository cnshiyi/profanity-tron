# perf-v065 - all-exact suffix2-12 两阶段尾部索引计算（已采纳）

时间：2026-05-21

## 目标

继续优化真实可用路径 `matchingCount > 1 && prefixCount <= 1 && suffixCount in [2,12]`，
不引入 benchmark-only 逻辑，并保持实际命中与私钥可独立推导地址链路不变。

## 新方向

在已采纳 `perf-v063`（`suffixTail2Allowed` 早拒绝）基础上，减少被拒绝样本的尾部计算开销：

- all-exact 分支不再先计算完整 `suffixCount` 位；
- 先做 2 轮 base58 除法，仅得到 `tailIndices[0..1]`；
- 用 `suffixTail2Allowed` 判定，不命中直接返回；
- 仅在命中时继续计算 `tailIndices[2..suffixCount-1]`，再进入 `base58_tail_match_indexed` 循环。

该改动只作用于真实多目标 `suffix2-12` all-exact 路径，generic 路径保持原行为。

## Benchmark（可复现）

统一命令：

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 5
```

基线（perf-v063 逻辑）：
- Final: `349.297 MH/s`
- Initialization: `22s`

候选（perf-v065 两阶段尾部索引）：
- Final: `360.180 MH/s`
- Initialization: `22s`

结果：`+3.12%`，采纳。

## 实际命中测试（采纳路径）

命令：

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --quit-count 3 --output .\runtime\perf_v065_hits.txt
```

结果：
- `Validated hits: 3`
- `Saved hits: 3`

## private 独立推导校验

对 `runtime\perf_v065_hits.txt` 的 3 条命中使用独立流程 `ecdsa + Keccak-256 + Base58Check` 复算：
- `verified=3`
- `mismatched=0`

## 结论

- `perf-v065` 在真实可用多目标 suffix5 路径上有可复现提升，已采纳。
- 该优化不改变命中语义；实际命中与 `private -> TRON address` 独立推导一致。
