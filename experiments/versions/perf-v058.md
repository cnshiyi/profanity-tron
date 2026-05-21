# perf-v058 - 多目标 suffix2-12 全精确后缀无分支索引匹配（已采纳）

时间：2026-05-21

## 目标

继续优化真实可用路径 `matchingCount > 1 && prefixCount <= 1 && suffixCount in [2,12]`，
不引入仅 benchmark 可用的旁路逻辑，保持实际命中和地址校验链路不变。

## 新方向

在 `perf-v057` 的“精确后缀索引匹配”基础上进一步减少分支开销：

- Host 侧新增 `suffixTailAllExact` 标记（1 字节常量缓冲）。
- 当多目标都满足“精确后缀”（每位 `mask == 0xff` 且字符可映射到 base58 index）时，
  kernel 走专用循环：仅执行 `base58_tail_match_indexed`，不再逐目标判断 `exact/generic` 分支。
- 若存在任意非精确目标，仍回退到 `v057` 的混合路径，保证功能覆盖不变。

## 代码变更

- `Dispatcher.hpp`
  - `Device` 新增 `m_memSuffixTailAllExact`。

- `Dispatcher.cpp`
  - 初始化阶段计算并写入 `suffixTailAllExact`。
  - `profanity_score_matching` 新增该缓冲参数并同步后移 kernel arg 索引。
  - `scoreMax` 动态更新参数索引同步调整。

- `kernels/kernel_profanity.cl`
  - 多目标 `suffix2-12` 分支增加 `suffixTailAllExact` 快路径：
    - 全精确：纯索引比较循环；
    - 非全精确：沿用 v057 的精确/通用混合匹配。

## Benchmark（可复现）

统一命令：

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 5
```

基线（v057 路径，临时强制 `suffixTailAllExact=0`）：
- Final: `351.194 MH/s`
- Initialization: `22s`

候选（perf-v058）：
- Final: `351.710 MH/s`
- Initialization: `22s`

结果：`+0.15%`，采纳。

## 实际命中测试

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --quit-count 3 --output .\runtime\perf_v058_hits.txt
```

结果：
- `Validated hits: 3`
- `Saved hits: 3`

## 私钥独立推导校验

使用独立 Python（`ecdsa + Crypto.Hash.keccak + base58check`）对
`runtime\perf_v058_hits.txt` 做 `private -> TRON address` 校验：

- `verified=3`
- `mismatched=0`

## 结论

- 本轮改动仅优化真实可用多目标后缀路径，未引入 benchmark-only 路径。
- 功能有效（真实命中 + 独立推导校验通过），性能小幅提升，已采纳。
- 失败原因：无（本轮候选通过）。
