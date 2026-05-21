# perf-v051 恢复与 410 MH/s 复现记录

时间：2026-05-22

## 结论

这次丢失的高价值代码路径已经恢复并保存：

- 恢复分支：`codex/recovered-perf-v051`
- 恢复提交：`1cbf514 perf: recover perf-v051 suffix1 bitmap path`
- 恢复标签：`recovered-perf-v051`
- 恢复补丁：`experiments/perf-v051-recovered.patch`
- 远端仓库：`https://github.com/cnshiyi/profanity-tron.git`

`perf-v054 = 410.025 MH/s` 不是 bitmask 候选实现本身。历史记录显示，410.025 MH/s 是当时保留的 `perf-v051` 真实生成路径作为 `perf-v054-baseline` 跑出的短测结果。

## 恢复到的真实代码方向

`perf-v051` 优化的是多目标 `suffix-count 1` 的真实钱包地址生成路径：

- Host 侧预计算 58 个 Base58 尾字符是否允许命中。
- GPU kernel 在多目标 suffix1 分支中只查 `suffix1Allowed[suffixIndex]`。
- 不跳过私钥、公钥、Keccak、Base58Check 或命中保存流程。
- 实际命中结果已经用独立 `private -> TRON address` 推导验证。

涉及源码：

- `Dispatcher.hpp`
- `Dispatcher.cpp`
- `kernels/kernel_profanity.cl`

## 历史 410 证据

原始输出仍保存在：

```text
runtime/perf_v054_baseline.full.txt
```

历史命令：

```powershell
.\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 1 --benchmark-seconds 5
```

历史输出关键结果：

```text
Final: 410.025 MH/s - GPU0: 410.025 MH/s
```

同一轮记录：

- `perf-v054-baseline`: `410.025 MH/s`
- `perf-v054` bitmask 候选：`408.636 MH/s`
- `perf-v054-revert`: `408.512 MH/s`

所以应回溯和保留的是 `perf-v051` 的 `suffix1Allowed[58]` 常量表路径，而不是 `perf-v054` 的 `ulong` bitmask 方向。

## 当前恢复验证

恢复代码已经在独立工作树中提交：

```text
C:\Users\399\Desktop\profanity-tron-perf051-exact
```

恢复分支：

```text
codex/recovered-perf-v051
```

当前已完成验证：

- 构建通过。
- 实际命中测试生成 3 条结果。
- 独立 `private -> TRON address` 验证：`verified=3 mismatched=0`。
- 当前环境短测复现速度约 `358-360 MH/s`，暂未稳定复现历史 `410.025 MH/s`。

## 当前未复现 410 的初步判断

源码路径已经找回；当前复现低于历史 410，更像是运行环境差异或 OpenCL/NVIDIA 编译状态差异，而不是源代码没有恢复。

已观察到的当前环境信息：

- GPU 负载期间利用率可到 `100%`。
- P-state 负载下多为 `P2`。
- SM clock 约 `1920-1950 MHz`。
- Memory clock 约 `6801 MHz`。
- 当前短测结果约 `358-360 MH/s`。

后续复现方向：

1. 用恢复分支连续运行 5s、10s、60s benchmark，确认波动范围。
2. 对比历史 OpenCL cache 文件和当前 cache 文件。
3. 测试历史 release/worktree 二进制是否仍能跑到 410。
4. 检查 NVIDIA 驱动、电源策略、P-state、应用时钟和后台 GPU 进程。
5. 只有实际生成、实际命中、独立验证通过的路径才可以作为恢复版本保留。

## 防再次丢失措施

从这次开始，高价值版本必须至少同时满足：

1. 有源码 commit 或可应用 patch。
2. 有 `experiments/versions/perf-vNNN.md` 记录。
3. 有 benchmark 原始输出路径。
4. 有实际命中输出路径。
5. 有独立 `private -> TRON address` 校验结果。
6. commit 失败时必须立刻换工作树、生成 patch、打 tag 或推送恢复分支，不能只停留在会话记录。
