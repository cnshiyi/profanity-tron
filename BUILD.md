# Build Guide

This project contains two build targets:

- `shiyi.exe`: the C++ / OpenCL command-line generator.
- `start.exe`: the Windows desktop launcher that starts `shiyi.exe`.

Build the command-line binary first. The desktop app expects `shiyi.exe` to exist in the repository root.

## Requirements

### Windows

- Windows 10 or later.
- Visual Studio 2022 or Visual Studio Build Tools with the C++ workload.
- A GPU driver with OpenCL runtime support.
- PowerShell.
- Node.js and npm, only if you want to run the desktop app.

The Windows build script automatically creates `OpenCL.lib` from `C:\Windows\System32\OpenCL.dll` when needed.

### Linux

- `g++` or another C++ compiler.
- OpenCL headers and runtime library.
- GPU driver with OpenCL support.

Example packages:

```bash
sudo apt update
sudo apt install build-essential ocl-icd-opencl-dev
```

### macOS

- Xcode Command Line Tools.
- Apple OpenCL framework.

Install the command-line tools if they are missing:

```bash
xcode-select --install
```

## Build The Command-Line Tool

### Windows

From the repository root:

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1 -Clean
```

Expected output:

```text
Build complete: <repo>\shiyi.exe
```

Useful options:

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1
powershell -ExecutionPolicy Bypass -File .\build.ps1 -Configuration Debug
powershell -ExecutionPolicy Bypass -File .\build.ps1 -OutputName shiyi-debug.exe
```

### Linux

From the repository root:

```bash
make clean
make
```

If your OpenCL headers are not in the default include path, build manually and pass the bundled header path:

```bash
g++ Dispatcher.cpp KernelSources.cpp Mode.cpp precomp.cpp profanity.cpp SpeedSample.cpp \
  -std=c++11 -Wall -O2 -mcmodel=large -I./OpenCL/include -lOpenCL -o shiyi
```

### macOS

From the repository root:

```bash
make clean
make
```

The Makefile links against the system OpenCL framework on macOS.

## Smoke Test

After building, run a short test with a low match difficulty:

### Windows

```powershell
.\shiyi.exe --matching .\profanity.txt --suffix-count 3 --quit-count 1
```

### Linux / macOS

```bash
./shiyi --matching profanity.txt --suffix-count 3 --quit-count 1
```

If the machine has both integrated and dedicated GPUs, try `--skip 1` to select the next OpenCL device:

```powershell
.\shiyi.exe --matching .\profanity.txt --suffix-count 3 --quit-count 1 --skip 1
```

## Run The Desktop App

Build `shiyi.exe` first, then run:

```powershell
powershell -ExecutionPolicy Bypass -File .\build-client.ps1
.\start.exe
```

The desktop app launches `shiyi.exe` from the repository root. If the binary is missing, rebuild the command-line tool before starting the desktop app.

## Common Issues

### `Unable to find VsDevCmd.bat`

Install Visual Studio Build Tools and include the C++ build tools workload.

### `OpenCL.dll` or OpenCL device not found

Install or update the GPU driver. The project needs an OpenCL runtime from the GPU vendor.

### `OpenCL.lib` cannot be resolved

On Windows, use `build.ps1` instead of invoking `cl` manually. The script generates the import library from the system OpenCL DLL when needed.

### `vcruntime140_1.dll` is missing

Install the Microsoft Visual C++ Redistributable or Visual Studio Build Tools.

### Desktop app starts but generation fails

Confirm that `shiyi.exe` exists in the repository root and that the command-line smoke test works outside Electron.

## Clean Generated Files

Windows:

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1 -Clean
```

Linux / macOS:

```bash
make clean
```

The repository should not commit generated binaries, object files, OpenCL cache files, logs, screenshots, or hit/result files.
