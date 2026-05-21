# perf-v051 - 多目标 suffix1 位图匹配预计算

时间：2026-05-21

## 目标

在“多目标 + 仅后缀 1 位”真实可用地址生成路径上减少 GPU 每线程匹配开销：
- 保持现有 `private -> address` 生成与主机验证流程不变。
- 不引入仅 benchmark 可用的路径。

## 优化方向

此前路径（perf-v049）在 `matchingCount > 1 && prefixCount <= 1 && suffixCount == 1` 时：
- 每个 work-item 先算出最后一位 Base58 索引；
- 再在 kernel 内循环所有目标逐个比对。

本轮改为 host 侧预计算 58 槽命中表（`suffix1Allowed[58]`），kernel 只做一次查表判断。

## 代码改动

涉及文件：
- `Dispatcher.hpp`
- `Dispatcher.cpp`
- `kernels/kernel_profanity.cl`

具体变更：
1. `Device` 新增 `m_memSuffix1Allowed`（58 字节常量缓冲区）。
2. `initBegin` 中在满足多目标 suffix1 条件时，按所有目标的 mask/value 预计算允许命中的 Base58 索引集合。
3. 把该缓冲区作为 `profanity_score_matching` 新参数传入。
4. kernel 中该分支从“逐目标循环比较”改为：
   - 计算 `suffixIndex`
   - 直接 `suffix1Allowed[suffixIndex]` 查表

## Benchmark（可复现）

基线（改动前）：
```powershell
.\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 1 --benchmark-seconds 5
```
- Final: `296.355 MH/s`
- Initialization: `20s`

本轮（改动后同命令）：
```powershell
.\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 1 --benchmark-seconds 5
```
- Final: `375.603 MH/s`
- Initialization: `20s`
- 提升：`+26.74%`

## 实际命中测试

```powershell
.\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 1 --quit-count 5 --output .\runtime\perf_v051_hits.txt
```

结果：
- `Validated hits: 5`
- `Saved hits: 5`

## 独立 private -> TRON address 校验

使用独立 Python 脚本（`ecdsa + Keccak-256 + Base58Check`）校验输出文件：
```powershell
python - <verify script>
```
结果：
- `verified=5`
- 全部命中均可由 `private` 独立推导到同一 `address`

## 结论

本轮优化只作用于真实可用的多目标 suffix1 生成路径，保持命中与私钥独立推导一致，且在目标场景 benchmark 有明显提升，采纳。
