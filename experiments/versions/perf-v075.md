# perf-v075 - 60-second baseline rule

Date: 2026-05-22

## Goal

Establish the current retained path baseline with a benchmark duration long enough to avoid adopting noise-level changes.

From this version forward:
- performance adoption requires `--benchmark-seconds >= 60`
- shorter runs are allowed only as a prefilter
- a version still needs a real-hit test and independent `private -> TRON address` verification before adoption

## Command

```powershell
C:\Users\399\Desktop\profanity-tron\profanity.x64.exe --no-cache --matching .\runtime\multi10_targets.txt --prefix-count 0 --suffix-count 5 --benchmark-seconds 60
```

## Result

- Device: NVIDIA GeForce RTX 3070
- Work: `128`
- inverse-multiple: `237568`
- work-max: `60579840`
- Initialization: `23s`
- Final: `350.996 MH/s`

## Conclusion

This is a measurement-rule version, not a new algorithm candidate. The retained `perf-v065` path remains valid, and future optimization decisions must be compared against 60-second-or-longer measurements.
