param(
    [string]$Configuration = "Release",
    [string]$OutputName = "shiyi.exe",
    [switch]$Clean
)

$ErrorActionPreference = "Stop"

function Find-VsDevCmd {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        $installationPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
        if ($installationPath) {
            $candidate = Join-Path $installationPath "Common7\Tools\VsDevCmd.bat"
            if (Test-Path $candidate) {
                return $candidate
            }
        }
    }

    $fallbacks = @(
        "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat",
        "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat",
        "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat",
        "C:\Program Files\Microsoft Visual Studio\18\Professional\Common7\Tools\VsDevCmd.bat"
    )

    foreach ($candidate in $fallbacks) {
        if (Test-Path $candidate) {
            return $candidate
        }
    }

    throw "Unable to find VsDevCmd.bat. Install Visual Studio C++ Build Tools first."
}

function Find-MsvcTool([string]$toolName) {
    $vsDevCmd = Find-VsDevCmd
    $vsRoot = Split-Path (Split-Path (Split-Path $vsDevCmd -Parent) -Parent) -Parent
    $msvcRoot = Join-Path $vsRoot "VC\Tools\MSVC"
    if (!(Test-Path $msvcRoot)) {
        throw "Unable to find MSVC tools directory: $msvcRoot"
    }

    $tool = Get-ChildItem $msvcRoot -Directory |
        Sort-Object Name -Descending |
        ForEach-Object { Join-Path $_.FullName "bin\Hostx64\x64\$toolName" } |
        Where-Object { Test-Path $_ } |
        Select-Object -First 1

    if (!$tool) {
        throw "Unable to find tool: $toolName"
    }

    return $tool
}

function Ensure-OpenClImportLibrary {
    $repo = $PSScriptRoot
    $libPath = Join-Path $repo "OpenCL.lib"
    if (Test-Path $libPath) {
        return $libPath
    }

    $dumpbin = Find-MsvcTool "dumpbin.exe"
    $libexe = Find-MsvcTool "lib.exe"
    $defPath = Join-Path $repo "OpenCL.def"
    $dump = & $dumpbin /exports C:\Windows\System32\OpenCL.dll
    $names = @()
    foreach ($line in $dump) {
        if ($line -match '^\s+\d+\s+[0-9A-F]+\s+[0-9A-F]+\s+(.+)$') {
            $name = $Matches[1].Trim()
            if ($name -and $name -notmatch '^\[') {
                $names += $name
            }
        }
    }

    Set-Content -Path $defPath -Encoding ASCII -Value "LIBRARY OpenCL.dll"
    Add-Content -Path $defPath -Encoding ASCII -Value "EXPORTS"
    $names | Sort-Object -Unique | Add-Content -Path $defPath -Encoding ASCII
    & $libexe /def:$defPath /machine:x64 /out:$libPath | Out-Null
    return $libPath
}

function Invoke-BuildInVsEnv([string]$vsDevCmd, [string]$commandLine) {
    $tempBat = Join-Path $env:TEMP ("profanity-build-" + [guid]::NewGuid().ToString("N") + ".bat")
    @"
@echo off
call "$vsDevCmd" -arch=x64 >nul
if errorlevel 1 exit /b %errorlevel%
$commandLine
"@ | Set-Content -Path $tempBat -Encoding ASCII

    try {
        & cmd /c $tempBat
        if ($LASTEXITCODE -ne 0) {
            throw "Build command failed with exit code $LASTEXITCODE"
        }
    }
    finally {
        Remove-Item -LiteralPath $tempBat -ErrorAction SilentlyContinue
    }
}

$repo = $PSScriptRoot
Set-Location $repo

Get-Process profanity* -ErrorAction SilentlyContinue |
    Where-Object { $_.Path -eq (Join-Path $repo $OutputName) } |
    Stop-Process -Force -ErrorAction SilentlyContinue

if ($Clean) {
    Get-ChildItem -LiteralPath $repo -Filter *.obj -ErrorAction SilentlyContinue | Remove-Item -Force -ErrorAction SilentlyContinue
    Remove-Item -LiteralPath (Join-Path $repo $OutputName), (Join-Path $repo "OpenCL.def"), (Join-Path $repo "OpenCL.exp"), (Join-Path $repo "OpenCL.lib") -ErrorAction SilentlyContinue
}

$vsDevCmd = Find-VsDevCmd
$null = Ensure-OpenClImportLibrary

$optimizationFlags = if ($Configuration -eq "Debug") { "/Od /Zi" } else { "/O2" }
$compileCmd = "cl /nologo /EHsc /std:c++14 /utf-8 $optimizationFlags /I .\OpenCL\include /c Dispatcher.cpp KernelSources.cpp Mode.cpp precomp.cpp profanity.cpp SpeedSample.cpp"
$linkCmd = "link /nologo /OUT:$OutputName Dispatcher.obj KernelSources.obj Mode.obj precomp.obj profanity.obj SpeedSample.obj OpenCL.lib bcrypt.lib"

Invoke-BuildInVsEnv $vsDevCmd $compileCmd
Invoke-BuildInVsEnv $vsDevCmd $linkCmd

Write-Host "Build complete: $(Join-Path $repo $OutputName)"
