# perf-v061 - exact-tail 比较前置拒绝（未采纳）

时间：2026-05-21

## 目标

继续优化真实可用路径 `matchingCount > 1 && prefixCount <= 1 && suffixCount in [2,12]`，
不引入 benchmark-only 逻辑，保持实际命中与私钥校验链路不变。

## 新方向

在保留实现（perf-v058）基础上，尝试缩短 exact-tail 比较热路径：
- 函数 `base58_tail_match_indexed` 先比较 `tailIndices[0]` 与 `tailIndices[1]`，
- 只有前两位都匹配时才进入后续循环比较，期望降低平均比较开销。

## Benchmark（可复现）

统一命令：

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 5
```

候选（perf-v061）：
- Final: `975.097 KH/s`（`0.975 MH/s`）
- Initialization: `47s`

回退后基线（保留实现）：
- Final: `349.288 MH/s`
- Initialization: `22s`

结果：候选较基线严重回退，拒绝采纳。

## 实际命中测试（保留路径）

命令：

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --quit-count 3 --output .\runtime\perf_v061_revert_hits.txt
```

结果：
- `Validated hits: 3`
- `Saved hits: 3`

## private 独立推导校验

对 `runtime\perf_v061_revert_hits.txt` 最后 3 条命中，使用独立流程 `ecdsa + Keccak-256 + Base58Check` 复算地址：
- `verified=3`
- `mismatched=0`

## 结论

- `perf-v061` 会导致真实路径速度大幅下降，已拒绝并回退。
- 当前保留实现不变（延续已采纳路径），真实命中与私钥独立推导校验通过。
- 失败原因：前置两字节拒绝改写触发了不利代码生成/执行路径，整体吞吐明显下滑。
