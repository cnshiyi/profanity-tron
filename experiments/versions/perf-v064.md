# perf-v064 - all-exact suffix2-12 两字符分桶子集扫描（未采纳）

时间：2026-05-21

## 目标

继续优化真实可用路径 `matchingCount > 1 && prefixCount <= 1 && suffixCount in [2,12]`，
不引入 benchmark-only 逻辑，且保持真实地址命中与私钥独立推导链路不变。

## 新方向

在已采纳 `perf-v063` 的 all-exact 分支（先做 `suffixTail2Allowed` 早拒绝）上继续减少目标循环：
- Host 侧按后缀前两位 `(idx0, idx1)` 对 all-exact 目标分桶；
- Kernel 侧命中允许表后只遍历对应桶内目标，而不是遍历全部 `matchingCount`；
- 仅触发于真实多目标 `suffix2-12` 的 all-exact 路径。

## Benchmark（可复现）

统一命令：

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 5
```

基线（保留实现 perf-v063）：
- Final: `349.896 MH/s`
- Initialization: `22s`

候选（perf-v064 分桶子集扫描）：
- Final: `349.385 MH/s`
- Initialization: `22s`

结果：`-0.15%`，未达到基线，拒绝采纳。

## 回退与实际命中测试（保留路径）

回退到保留实现后执行：

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --quit-count 3 --output .\runtime\perf_v064_revert_hits.txt
```

结果：
- `Validated hits: 3`
- `Saved hits: 3`

## private 独立推导校验

对 `runtime\perf_v064_revert_hits.txt` 使用独立流程 `ecdsa + Keccak-256 + Base58Check` 复算：
- `verified=3`
- `mismatched=0`

## 结论

- `perf-v064` 在真实可用路径上负收益，已拒绝并回退，不保留失败实现。
- 当前保留实现继续为 `perf-v063`，真实命中与 `private -> TRON address` 独立推导校验通过。
- 失败原因：分桶索引与额外内存读取开销抵消了减少循环的收益。
