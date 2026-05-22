# perf-v090 - 单账号 suffix2 直接取模快路径

时间：2026-05-22

## 目标

继续深挖单账号实际使用路径，这轮专门处理：

```powershell
.\profanity.x64.exe --matching TUqEg3dzVEJNQSVW2HY98z5X8SBdhmao8D --prefix-count 1 --suffix-count 2
```

`perf-v089` 已经让单账号 `prefix-count 1 + suffix-count >= 3` 走后缀快路径，但 `suffix-count 2` 仍会落入完整 Base58 编码通用路径。

## 改动

新增单目标 `suffix-count 2` 快路径：

- 条件：`matchingCount == 1 && prefixCount <= 1 && suffixCount == 2`
- 仅针对最后两位都是精确匹配的常用目标地址场景。
- 使用已有 `tronhash_mod_3364_from_ethhash()` 直接计算 TRON Base58Check payload 对 `58^2` 的余数。
- 从余数得到最后两位 Base58 索引并与目标最后两位比较。
- 不改变命中保存、私钥生成或主机侧验证流程。

文件：

- `kernels/kernel_profanity.cl`

## Benchmark

基线（改动前）：

```powershell
.\profanity.x64.exe --no-cache --matching TUqEg3dzVEJNQSVW2HY98z5X8SBdhmao8D --prefix-count 1 --suffix-count 2 --benchmark-seconds 60
```

- `173.535 MH/s`
- initialization: `23s`

候选预筛：

```powershell
.\profanity.x64.exe --no-cache --matching TUqEg3dzVEJNQSVW2HY98z5X8SBdhmao8D --prefix-count 1 --suffix-count 2 --benchmark-seconds 5
```

- `357.730 MH/s`

正式 60 秒测试：

```powershell
.\profanity.x64.exe --no-cache --matching TUqEg3dzVEJNQSVW2HY98z5X8SBdhmao8D --prefix-count 1 --suffix-count 2 --benchmark-seconds 60
```

- `354.693 MH/s`
- initialization: `23s`
- work: `128`
- inverse-multiple: `237568`
- work-max: `60579840`

## 实际命中测试

```powershell
.\profanity.x64.exe --no-cache --matching TUqEg3dzVEJNQSVW2HY98z5X8SBdhmao8D --prefix-count 1 --suffix-count 2 --quit-count 3 --output .\runtime\single_p1s2_candidate_hits_v090.txt
```

结果：

- `Validated hits: 3`
- `Saved hits: 3`
- run speed: `289.963 MH/s`

## 独立校验

使用独立 Python `ecdsa + Keccak-256 + Base58Check` 从私钥推导 TRON 地址：

- `verified=3`
- `mismatched=0`

## 结论

采纳。

这轮修复了单账号 `prefix-count 1 + suffix-count 2` 仍走完整 Base58 通用路径的问题。新路径仍然是真实地址生成路径，实际命中和独立私钥推导均通过。
