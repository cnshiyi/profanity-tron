# perf-v050 - 私钥范围控制参数

时间：2026-05-21

## 目标

给生成器增加可选的私钥范围控制参数：

- `--range-start`
- `--range-end`
- `--range-direction up|down`

三个参数都可以留空，留空时继续使用默认安全随机种子。

## 实现

- 默认随机模式不变。
- 范围模式控制私钥前 64 位中的连续变化窗口。
- 当前版本要求变化窗口结束在第 16 个 hex 位。
- `up` 使用原有 `seed.w + id` 路径。
- `down` 增加 `seed.w - id` 初始化方向。
- 输出文件中的 `private` 按实际 GPU seed / round / foundId 重建，不覆盖低位，确保能独立推导出同一 TRON 地址。

## 验证

构建：

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1 -Clean
```

向上测试：

```powershell
.\profanity.x64.exe --no-cache --matching TUqEg3dzVEJNQSVW2HY98z5X8SBdhmao8D --prefix-count 0 --suffix-count 1 --quit-count 3 --work 128 --inverse-multiple 1024 --range-start 58958afa84300000e8ce89f9a5b808e8ee611ad00deaf3745a5305e0a3e1d688 --range-end 58958afa844fffffe8ce89f9a5b808e8ee611ad00deaf3745a5305e0a3e1d688 --range-direction up --output .\runtime\range_test_hits.txt
```

结果：

- `Validated hits: 3`
- 独立 Python 校验 `private -> TRON address` 全部 OK

向下测试：

```powershell
.\profanity.x64.exe --no-cache --matching TUqEg3dzVEJNQSVW2HY98z5X8SBdhmao8D --prefix-count 0 --suffix-count 1 --quit-count 2 --work 128 --inverse-multiple 1024 --range-start 58958afa844fffffe8ce89f9a5b808e8ee611ad00deaf3745a5305e0a3e1d688 --range-end 58958afa84300000e8ce89f9a5b808e8ee611ad00deaf3745a5305e0a3e1d688 --range-direction down --output .\runtime\range_down_hits.txt
```

结果：

- `Validated hits: 2`
- 独立 Python 校验 `private -> TRON address` 全部 OK

## 后续

- 扩展到更低位窗口需要改 GPU 迭代模型，不能牺牲地址/私钥一致性。
- 性能自动迭代继续围绕真实可用生成路径，不记录不可实际生成的钱包地址版本。
