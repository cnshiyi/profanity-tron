# perf-v062 - exact-tail 比较 switch 固定展开（未采纳）

时间：2026-05-21

## 目标

继续优化真实可用路径 `matchingCount > 1 && prefixCount <= 1 && suffixCount in [2,12]`，
不引入 benchmark-only 逻辑，并保持实际命中与私钥校验链路不变。

## 新方向

在已采纳路径（perf-v058）上，针对 exact-tail 索引比较尝试固定形状展开：
- 新增 `base58_tail_match_indexed_fixed(...)`，用 `switch + fallthrough` 代替运行时 `for` 循环；
- 多目标 `suffix2-12` exact-tail 分支改为调用该固定展开比较。

预期：降低循环控制和索引开销。

## Benchmark（可复现）

统一命令：

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 5
```

候选（perf-v062）：
- Final: `348.488 MH/s`
- Initialization: `22s`

回退后保留实现（revert）：
- Final: `348.686 MH/s`
- Initialization: `22s`

结果：`-0.06%`，负收益，拒绝采纳。

## 实际命中测试（保留路径）

命令：

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --quit-count 3 --output .\runtime\perf_v062_revert_hits.txt
```

结果：
- `Validated hits: 3`
- `Saved hits: 3`

## private 独立推导校验

对 `runtime\perf_v062_revert_hits.txt` 的 3 条命中使用独立流程 `ecdsa + Keccak-256 + Base58Check` 复算：
- `verified=3`
- `mismatched=0`

## 结论

- `perf-v062` 在真实路径上无提升并轻微回退，已拒绝并回退。
- 当前保留实现维持 `perf-v058` 路线，真实命中与私钥独立推导校验通过。
- 失败原因：fixed-shape switch 展开未带来更优代码生成，实际吞吐略低于保留实现。
