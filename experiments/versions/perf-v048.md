# perf-v048

## 状态

已完成。

## 目的

对 `perf-v047` 做连续二次确认，判断重启后的 300+ MH/s 是否只是偶发冲高。

## 测试命令

```powershell
.\profanity.x64.exe --matching TUqEg3dzVEJNQSVW2HY98z5X8SBdhmao8D --prefix-count 1 --suffix-count 1 --benchmark-seconds 10
```

## 结果

- 初始化时间: 21 秒
- 最终速度: 359.758 MH/s
- GPU0: 359.758 MH/s
- 运行后未发现 `profanity*` 或 `TronStudio` 残留进程

## 结论

连续第二次运行仍保持在 300+ MH/s，说明 `perf-v044` 的高速度可以复现。之前同样命令掉到约 208 MH/s，更可能由系统/GPU运行状态、后台占用、驱动状态或未清理进程导致，而不是 native benchmark 路径本身退化。
