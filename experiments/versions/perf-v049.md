# perf-v049

## 状态

已完成。

## 目标

优化多地址匹配场景，重点是客户端常见的多目标、只匹配后缀位数场景。

## 问题

旧逻辑中，只要 `matchingCount > 1`，GPU 评分 kernel 就会退回完整 TRON Base58 地址编码，再逐个目标逐位匹配。这会让多地址场景明显慢于单目标后缀快路径。

## 改动

- 新增多目标 suffix-only 快路径。
- 对 `matchingCount > 1 && prefixCount <= 1 && suffixCount == 1`：
  - 只计算 Base58 最后一位索引。
  - 在 GPU 上扫描多个目标最后一位。
- 对 `matchingCount > 1 && prefixCount <= 1 && suffixCount >= 2 && suffixCount <= 12`：
  - 只计算所需尾部 Base58 索引。
  - 避免完整 34 字符地址编码。
  - 对多个目标复用同一组尾部索引。
- 保留单目标原快路径，避免影响已有单目标速度。

## 基线

```powershell
.\profanity.x64.exe --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 5
```

- 目标数量: 10
- 后缀位数: 5
- 基线速度: 166.386 MH/s

## 结果

```powershell
.\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 5
```

- 初始化时间: 20 秒
- 最终速度: 370.666 MH/s
- 提升: 约 2.23x

## 正确性验证

实际命中验证：

```powershell
.\profanity.x64.exe --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 1 --quit-count 5 --output .\runtime\multi_fast_hits.txt
.\profanity.x64.exe --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 2 --quit-count 5 --output .\runtime\multi_fast_hits_suffix2.txt
```

结果：

- suffix1: `Validated hits: 5`, `Saved hits: 5`
- suffix2: `Validated hits: 5`, `Saved hits: 5`

## 结论

多地址 suffix-only 场景已经从完整 Base58 编码路径切到尾部索引快路径。10 目标后 5 位 benchmark 从 166.386 MH/s 提升到 370.666 MH/s，且实际命中保存和主机校验均通过。
