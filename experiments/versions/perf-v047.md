# perf-v047

## 状态

已完成。

## 目的

在电脑重启后，重新复现 `perf-v044` 的 native benchmark，用来排查客户端退出后是否存在残留进程、显卡状态异常或后台占用导致的速度下降。

## 测试命令

```powershell
.\profanity.x64.exe --matching TUqEg3dzVEJNQSVW2HY98z5X8SBdhmao8D --prefix-count 1 --suffix-count 1 --benchmark-seconds 10
```

## 测试前检查

未发现残留进程：

- `profanity*`
- `TronStudio`

## 测试环境

- GPU: NVIDIA GeForce RTX 3070
- 显存: 8589410304 bytes available
- Compute units: 46
- work: 128
- inverse-multiple: 237568
- work-max: 60579840
- OpenCL program: cache 命中

## 结果

- 初始化时间: 22 秒
- 最终速度: 383.733 MH/s
- GPU0: 383.733 MH/s

## 结论

这次重启后，同一条 `perf-v044` benchmark 命令不仅复现了 300+ MH/s，还跑到了 383.733 MH/s。说明之前掉到约 208 MH/s 的情况更像是运行环境状态问题，而不是客户端 UI 日志本身直接导致。

下一步排查重点：

1. 客户端关闭时是否彻底结束 `profanity.x64.exe` 子进程。
2. 停止任务后是否仍有 OpenCL 上下文或子进程残留。
3. 多次运行后 GPU 是否进入低功耗、温控或驱动异常状态。
4. 客户端启动生成时是否应默认使用静默日志，并在退出时强制清理同目录下的生成器进程。
