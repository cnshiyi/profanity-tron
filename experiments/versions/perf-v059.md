# perf-v059 - 多目标 suffix2-12 exact/generic 分组循环（未采纳）

时间：2026-05-21

## 目标

继续优化真实可用路径 `matchingCount > 1 && prefixCount <= 1 && suffixCount in [2,12]`，
不引入 benchmark-only 逻辑，保持实际生成地址与校验链路不变。

## 新方向

在 `perf-v058` 保留实现基础上，尝试把多目标 `suffix2-12` 的混合匹配改为“预分组双循环”：
- Host 侧预计算 exact 目标顺序和 generic 目标顺序；
- Kernel 先遍历 exact 列表（索引直比较），再遍历 generic 列表（mask 比较）；
- 目的：去掉每目标 `exact/generic` 条件分支。

该方向仅修改真实多目标后缀匹配路径。

## 候选实现结果（perf-v059）

复现实验命令：

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 5
```

对照基线（已采纳 `perf-v058`）：
- Final: `351.710 MH/s`
- Initialization: `22s`

候选（perf-v059）：
- Final: `351.403 MH/s`
- Initialization: `22s`

结果：`-0.09%`，未达到基线，拒绝采纳。

## 回退与保留实现复测

回退后同命令复测：
- Final: `351.205 MH/s`
- Initialization: `22s`

说明：已回到保留实现（`perf-v058` 路径），保留真实可用生成链路。

## 实际命中测试（保留实现）

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --quit-count 3 --output .\runtime\perf_v059_revert_hits.txt
```

结果：
- `Validated hits: 3`
- `Saved hits: 3`

## private 独立推导校验

使用独立 Python（`ecdsa + Keccak-256 + Base58Check`）对 `runtime\perf_v059_revert_hits.txt` 校验：
- `verified=3`
- `mismatched=0`

## 结论

- `perf-v059` 方向在真实路径无性能收益，已明确记录为失败并拒绝采纳。
- 当前保留实现仍为 `perf-v058`，真实命中与私钥独立推导校验通过。
- 失败原因：分组顺序表额外内存访问/计数读取抵消了去分支收益。
