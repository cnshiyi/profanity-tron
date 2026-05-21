# perf-v054 - 多目标 suffix1 位图改为 64-bit 参数掩码（未采纳）

时间：2026-05-21

## 目标

在真实可用生成路径 `matchingCount > 1 && prefixCount <= 1 && suffixCount == 1` 中，
把 `suffix1Allowed[58]` 常量缓冲区查表改为单个 `ulong` 位掩码参数，降低参数与常量内存访问开销。

约束：
- 不改变地址生成与校验流程。
- 不引入仅 benchmark 可跑的路径。

## 新方向说明

相对 perf-v051，本轮尝试：
- Host 侧按目标集合预计算 58 位允许集合，并打包到 `cl_ulong suffix1AllowedMask`。
- Kernel 在多目标 suffix1 分支改为位测试：`(suffix1AllowedMask >> suffixIndex) & 1`。

## Benchmark（可复现）

A/B 命令（保持一致）：

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 1 --benchmark-seconds 5
```

基线（perf-v051 保留实现）：
- Final: `410.025 MH/s`
- Initialization: `20s`

候选（perf-v054 掩码实现）：
- Final: `408.636 MH/s`
- Initialization: `20s`

结果：`-0.34%`，轻微回退。

回退后复测（恢复保留实现）：
- Final: `408.512 MH/s`
- Initialization: `20s`

## 实际命中测试（保留实现）

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 1 --quit-count 3 --output .\runtime\perf_v054_revert_hits.txt
```

结果：
- `Validated hits: 3`
- `Saved hits: 3`

## 私钥独立推导校验

对 `runtime\perf_v054_revert_hits.txt` 使用独立 Python（`ecdsa + keccak + base58check`）校验：
- `verified=3`
- `mismatched=0`

## 失败原因

64-bit 位掩码替代常量缓冲查表在当前 GPU 编译结果下没有形成优势，可能带来额外位运算/寄存器压力，最终吞吐略低于基线。

## 结论

- perf-v054 方向记录为失败，不采纳。
- 保留当前真实可用代码（perf-v051 路径）。
