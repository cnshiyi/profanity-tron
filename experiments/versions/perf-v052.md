# perf-v052 - 多目标 suffix2-12 最后一位预过滤（未采纳）

时间：2026-05-21

## 目标

在真实可用生成路径 `matchingCount > 1 && prefixCount <= 1 && suffixCount in [2,12]` 中，
利用已有最后一位 Base58 位图（`suffix1Allowed`）做早期过滤，减少不必要的尾部完整匹配开销。

## 新方向说明

相对 perf-v051（只加速了 suffix1 分支），本轮尝试：
- Host 侧将 `suffix1Allowed` 的预计算范围扩展到 `suffixCount >= 1`。
- Kernel 在多目标 suffix2-12 分支先计算最后一位索引：
  - 若 `suffix1Allowed[suffixIndex] == 0` 则直接 `return`；
  - 否则再执行 `base58_tail_indices_from_ethhash` 与逐目标尾部匹配。

该方向不改变地址生成算法，也不绕过命中校验，属于真实路径优化尝试。

## 代码触点（尝试版）

- `Dispatcher.cpp`
- `kernels/kernel_profanity.cl`

## Benchmark（可复现）

命令（A/B 完全一致）：

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 5
```

基线（v051）：
- Final: `349.284 MH/s`
- Initialization: `20s`

候选（v052 尝试版）：
- Final: `312.516 MH/s`
- Initialization: `20s`

结果：`-10.53%`（回退）

## 实际命中测试（候选版执行）

```powershell
.\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --quit-count 5 --output .\runtime\perf_v052_hits.txt
```

结果：
- `Validated hits: 5`
- `Saved hits: 5`

## 私钥独立推导校验

使用独立 Python（`ecdsa + keccak + base58check`）对 `runtime/perf_v052_hits.txt` 中 5 条记录做 `private -> address` 验证：
- `verified=5`
- `mismatched=0`

## 结论

- 功能有效（命中与私钥独立推导正确），但性能显著回退。
- `perf-v052` 方向标记为失败，不采纳。
- 代码已回退到 `perf-v051` 状态，避免把失败方向留在真实生成路径中。
