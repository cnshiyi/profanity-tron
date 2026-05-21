# perf-v089 - 单账号 prefix1 后缀快路径

时间：2026-05-22

## 目标

深挖单账号实际使用路径：

```powershell
.\profanity.x64.exe --matching TUqEg3dzVEJNQSVW2HY98z5X8SBdhmao8D --prefix-count 1 --suffix-count 5
```

TRON 地址首字符天然固定为 `T`。因此单目标 `prefix-count 1` 不需要完整 Base58 编码检查前缀，应该可以和 `prefix-count 0` 一样走已有的后缀快路径。

## 问题

改动前，单目标后缀快路径条件是：

```c
matchingCount == 1 && prefixCount == 0 && suffixCount >= 3 && suffixCount <= 12
```

这导致 `prefix-count 1 + suffix-count 5` 被迫落入通用路径，执行完整 Base58 编码和逐字符评分。

## 改动

把单目标后缀快路径放宽为：

```c
matchingCount == 1 && prefixCount <= 1 && suffixCount >= 3 && suffixCount <= 12
```

文件：

- `kernels/kernel_profanity.cl`

## Benchmark

预筛对照：

```powershell
.\profanity.x64.exe --no-cache --matching TUqEg3dzVEJNQSVW2HY98z5X8SBdhmao8D --prefix-count 1 --suffix-count 5 --benchmark-seconds 5
```

改动前：

- `163.300 MH/s`

改动后：

- `355.116 MH/s`

正式 60 秒测试：

```powershell
.\profanity.x64.exe --no-cache --matching TUqEg3dzVEJNQSVW2HY98z5X8SBdhmao8D --prefix-count 1 --suffix-count 5 --benchmark-seconds 60
```

结果：

- `344.505 MH/s`
- initialization: `22s`
- work: `128`
- inverse-multiple: `237568`
- work-max: `60579840`

## 实际命中测试

```powershell
.\profanity.x64.exe --no-cache --matching TUqEg3dzVEJNQSVW2HY98z5X8SBdhmao8D --prefix-count 1 --suffix-count 5 --quit-count 3 --output .\runtime\single_p1s5_candidate_hits_v089.txt
```

结果：

- `Validated hits: 3`
- `Saved hits: 3`
- run speed: `353.984 MH/s`

## 独立校验

使用独立 Python `ecdsa + Keccak-256 + Base58Check` 从私钥推导 TRON 地址：

- `verified=3`
- `mismatched=0`

## 结论

采纳。

这个优化修正了单账号常用参数 `prefix-count 1 + suffix-count >= 3` 未走快路径的问题。它不跳过真实地址生成，不改变私钥或命中保存逻辑，实际命中和独立私钥推导均通过。
