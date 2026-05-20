# perf-v024

## 状态

已完成

## 优化目标

移除 AMD GPU 作为支持目标，收缩运行与调优范围，避免继续为 AMD 路径保留自动调优和设备接入逻辑。

## 涉及文件

- `profanity.cpp`
- `benchmark.ps1`
- `experiments/versions/perf-v024.md`

## 具体改动

1. 删除 AMD 设备拓扑识别辅助逻辑
2. 删除自动调优中的 AMD 分支
3. 在设备扫描阶段显式跳过 AMD GPU，并输出 `AMD support removed`
4. benchmark 供应商识别不再标记 AMD

## 适用设备范围

- 保留: NVIDIA、Intel
- 移除: AMD

## 结论

这是支持范围收缩，不是提速优化。它的作用是明确产品边界，让后续优化只围绕保留设备推进，减少跨厂商兼容包袱。
