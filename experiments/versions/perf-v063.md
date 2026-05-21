# perf-v063 - all-exact suffix2 二维允许表早拒绝（已采纳）

时间：2026-05-21

## 目标

继续优化真实可用路径 `matchingCount > 1 && prefixCount <= 1 && suffixCount in [2,12]`，
不引入 benchmark-only 逻辑，并保持实际命中与私钥校验链路不变。

## 新方向

在已采纳路径（perf-v058）上新增 all-exact 分支早拒绝：
- Host 侧预构建 `58 x 58` 的 `suffixTail2Allowed` 表，标记所有目标尾部前两位 base58 索引组合；
- Kernel 仅在 `suffixTailAllExact=1` 且 `suffixCount>=2` 时生效：先计算 `tailIndices[0], tailIndices[1]`，若组合不在表中则直接返回；
- 表命中时再进入原有 `base58_tail_match_indexed` 目标循环，保持结果正确性和真实地址产出路径。

预期：在目标数量较多且分布稀疏时减少无效目标循环开销。

## Benchmark（可复现）

基线（禁用该门控，仅用于同轮 A/B）：

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 5
```

- Final: `347.975 MH/s`
- Initialization: `22s`

候选（开启 `suffixTail2Allowed` 门控）：

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 5
```

- Final: `349.988 MH/s`
- Initialization: `22s`

结果：`+0.58%`，采纳。

## 实际命中测试（采纳路径）

命令：

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --quit-count 3 --output .\runtime\perf_v063_hits_run2.txt
```

结果：
- `Validated hits: 3`
- `Saved hits: 3`

## private 独立推导校验

对 `runtime\perf_v063_hits_run2.txt` 的 3 条命中使用独立流程 `ecdsa + Keccak-256 + Base58Check` 复算：
- `verified=3`
- `mismatched=0`

## 结论

- `perf-v063` 在真实可用多目标 suffix5 路径上带来可复现提升，已采纳。
- 该优化不改变命中语义；实际命中与 `private -> TRON address` 独立推导一致。
