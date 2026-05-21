# perf-v068 - all-exact suffix3 allow gate（未采纳）

时间：2026-05-21

## 目标

继续优化真实可用路径 `matchingCount > 1 && prefixCount <= 1 && suffixCount in [2,12]`，
不引入 benchmark-only 逻辑，并保持实际命中与 `private -> TRON address` 独立推导链路不变。

## 新方向

在已采纳 `perf-v065`（两阶段尾部索引 + `suffixTail2Allowed` 早拒绝）基础上，
尝试在 all-exact 分支增加第 3 位 Base58 早拒绝：
- Host 侧预构建 `suffixTail3Allowed`（`58 * 58 * 58`）允许表；
- Kernel all-exact 分支在通过 `suffixTail2Allowed` 后，先计算第 3 位尾索引；
- 若 `(idx0, idx1, idx2)` 不在允许表中则直接返回，命中时再继续计算剩余尾位与目标循环。

该尝试仅作用于真实多目标 `suffix2-12` 生成路径。

## Benchmark（可复现）

统一命令：

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 5
```

基线（保留实现 perf-v065）：
- Final: `359.855 MH/s`
- Initialization: `22s`

候选（perf-v068 suffix3 allow gate）：
- Final: `360.288 MH/s`
- Initialization: `22s`

结果：`+0.12%`，属于噪声级，未采纳。

## 回退与实际命中测试（保留路径）

回退到保留实现后执行：

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --quit-count 3 --output .\runtime\perf_v068_revert_hits.txt
```

结果：
- `Validated hits: 3`
- `Saved hits: 3`

## private 独立推导校验

对 `runtime\perf_v068_revert_hits.txt` 使用独立流程 `ecdsa + Keccak-256 + Base58Check` 复算：
- `verified=3`
- `mismatched=0`

## 结论

- `perf-v068` 仅有噪声级增益，不满足采纳标准，已拒绝并回退，不保留失败实现。
- 当前保留实现仍为 `perf-v065`，真实命中与 `private -> TRON address` 独立推导校验通过。
- 失败原因：新增 suffix3 gate 的过滤收益被额外表读取与分支成本抵消，净增益不足。
