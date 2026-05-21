# perf-v060 - 多目标 suffix2-12 exact-tail 打包比较（未采纳）

时间：2026-05-21

## 目标

继续优化真实可用路径 `matchingCount > 1 && prefixCount <= 1 && suffixCount in [2,12]`，
不引入 benchmark-only 逻辑，保持实际命中与私钥校验链路不变。

## 新方向

在保留实现 `perf-v058` 的 exact-tail 索引比较基础上，尝试减少逐字节比较开销：
- Host 侧预打包每个目标的 exact tail（最多 12 字节）为三个 `uint`。
- Kernel 侧把候选 `tailIndices` 同样打包后做 3 次整数比较。
- 仅作用于真实多目标后缀路径，不改变命中结果格式。

## Benchmark（可复现）

统一命令：

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 5
```

基线（保留实现 perf-v058）：
- Final: `351.808 MH/s`
- Initialization: `22s`

候选（perf-v060 打包比较）：
- Final: `348.281 MH/s`
- Initialization: `22s`

结果：`-1.00%`，拒绝采纳。

## 回退与实际命中测试（保留路径）

回退到保留实现后执行：

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --quit-count 3 --output .\runtime\perf_v060_revert_hits.txt
```

结果：
- `Validated hits: 3`
- `Saved hits: 3`

## private 独立推导校验

使用独立 Python（`ecdsa + Keccak-256 + Base58Check`）对 `runtime\perf_v060_revert_hits.txt` 校验：
- `verified=3`
- `mismatched=0`

## 结论

- `perf-v060` 在真实路径上性能下降，已拒绝并回退，不保留失败实现。
- 当前保留实现仍为 `perf-v058`，真实命中与 private 独立推导校验通过。
- 失败原因：每候选额外打包计算开销超过逐字节比较节省。
