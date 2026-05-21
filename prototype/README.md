# Standalone Prototype

这个目录放的是脱离主工程结构的性能原型。

当前原型目标：

- 验证把结果缓冲区初始化从主初始化 kernel 中拆出来，是否更利于整体吞吐
- 不依赖主工程的调度代码
- 单独构建、单独 benchmark、单独记录结果

当前原型：

- `standalone_clear_benchmark.cpp`

说明：

- 这是一个 OpenCL 独立基准程序
- 它比较两种结果区初始化方式：
  1. 每个 work-item 都循环清结果区
  2. 单独用一个 kernel 清结果区，主 kernel 不再重复清理

这个原型不是完整钱包生成器，而是用来验证“流水线拆分”是否值得继续深挖。
