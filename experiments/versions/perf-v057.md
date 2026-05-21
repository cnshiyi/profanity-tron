# perf-v057 - 多目标 suffix2-12 精确后缀索引直比较快路径（已采纳）

时间：2026-05-21

## 目标

在真实可用路径 `matchingCount > 1 && prefixCount <= 1 && suffixCount in [2,12]` 中优化吞吐，
且必须保持实际可生成地址与校验链路不变。

约束：
- 不能引入仅 benchmark 可用路径。
- 仍通过现有结果验证与输出链路产出真实地址。

## 新方向

新增“精确后缀”快路径（仅在目标后缀位全为精确字符时启用）：
- Host 侧预计算每个目标是否满足 `mask == 0xff` 的精确后缀，并把对应 Base58 索引写入 `suffixTailIndex`。
- Kernel 在多目标 `suffix2-12` 分支中：
  - 若目标被标记为精确后缀，则直接比较 `tailIndices[step] == suffixTailIndex[...]`；
  - 否则回落到原有 `alphabet+mask` 的通用匹配。

该方案仅优化真实生成路径的匹配分支，不改变私钥生成、命中验证、结果落盘流程。

## 代码变更

- `Dispatcher.hpp`
  - `Device` 新增常量缓冲：`m_memSuffixTailExact`、`m_memSuffixTailIndex`。

- `Dispatcher.cpp`
  - `initBegin` 中预计算并写入精确后缀标记与索引。
  - `profanity_score_matching` 绑定新增两个参数缓冲并调整后续参数索引。
  - `handleResult` 对 `scoreMax` 的动态 kernel arg 更新索引同步后移。

- `kernels/kernel_profanity.cl`
  - 新增 `base58_tail_match_indexed`。
  - 在多目标 `suffix2-12` 分支按目标选择“索引直比较”或“通用 mask 匹配”。

## Benchmark（可复现）

A/B 命令（完全一致）：

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 5
```

基线（保留实现 / perf-v051 路径）：
- Final: `349.779 MH/s`
- Initialization: `22s`

候选（perf-v057）：
- Final: `351.407 MH/s`
- Initialization: `22s`

结果：`+0.47%`，采纳。

## 实际命中测试

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --quit-count 3 --output .\runtime\perf_v057_hits.txt
```

结果：
- `Validated hits: 3`
- `Saved hits: 3`

## 私钥独立推导校验

对 `runtime\perf_v057_hits.txt` 使用独立 Python（`ecdsa + Keccak-256 + base58check`）校验：
- `verified=3`
- `mismatched=0`

## 结论

- `perf-v057` 在真实多目标 suffix5 路径实现小幅稳定提升并通过真实命中与独立私钥推导校验。
- 本轮采纳该改动。
