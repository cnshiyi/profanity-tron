param(
    [string]$Version = ""
)

$ErrorActionPreference = "Stop"

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$distDir = Join-Path $repoRoot "dist"
$runtimeDir = Join-Path $distDir "runtime"
$kernelDistDir = Join-Path $distDir "kernels"
$buildDir = Join-Path $repoRoot "build\windows"
$objDir = Join-Path $buildDir "obj"
$packageRoot = Join-Path $buildDir "package"

function First-ExistingPath {
    param([string[]]$Paths)
    foreach ($path in $Paths) {
        if ($path -and (Test-Path -LiteralPath $path)) {
            return $path
        }
    }
    return $null
}

if (Test-Path -LiteralPath $distDir) {
    $resolvedDist = (Resolve-Path -LiteralPath $distDir).Path
    if (-not $resolvedDist.StartsWith($repoRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to clear dist outside repository: $resolvedDist"
    }
    Remove-Item -LiteralPath $distDir -Recurse -Force
}

New-Item -ItemType Directory -Force -Path $distDir, $runtimeDir, $kernelDistDir, $objDir | Out-Null

$vcvars = First-ExistingPath @(
    "C:\Program\VC\Auxiliary\Build\vcvars64.bat",
    "$env:ProgramFiles\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat",
    "$env:ProgramFiles\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat",
    "$env:ProgramFiles\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat",
    "$env:ProgramFiles\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
)

if (-not $vcvars) {
    throw "Could not find vcvars64.bat."
}

$csc = First-ExistingPath @(
    "C:\Program\MSBuild\Current\Bin\Roslyn\csc.exe",
    "$env:ProgramFiles\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\Roslyn\csc.exe",
    "$env:ProgramFiles\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\Roslyn\csc.exe",
    "$env:ProgramFiles\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\Roslyn\csc.exe",
    "$env:ProgramFiles\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\Roslyn\csc.exe"
)

if (-not $csc) {
    throw "Could not find csc.exe."
}

$nativeBuildCmd = Join-Path $buildDir "build-native.cmd"
@"
@echo off
call "$vcvars"
if errorlevel 1 exit /b %errorlevel%
cl.exe /c /utf-8 /I"$repoRoot\OpenCL\include" /nologo /W1 /WX- /diagnostics:column /O2 /EHsc /MD /GS /fp:precise /Zc:wchar_t /Zc:forScope /Zc:inline /Fo"$objDir\\" /external:W1 /Gd /TP /FC /errorReport:prompt "$repoRoot\Dispatcher.cpp" "$repoRoot\KernelSources.cpp" "$repoRoot\Mode.cpp" "$repoRoot\precomp.cpp" "$repoRoot\profanity.cpp" "$repoRoot\SpeedSample.cpp"
if errorlevel 1 exit /b %errorlevel%
link.exe /ERRORREPORT:PROMPT /OUT:"$distDir\shiyi.exe" /NOLOGO /INCREMENTAL:NO /LIBPATH:"$repoRoot\OpenCL\lib" OpenCL.lib bcrypt.lib /MANIFEST /MANIFESTUAC:"level='asInvoker' uiAccess='false'" /manifest:embed /TLBID:1 /DYNAMICBASE /NXCOMPAT /IMPLIB:"$buildDir\shiyi.lib" /MACHINE:X64 "$objDir\*.obj"
exit /b %errorlevel%
"@ | Set-Content -LiteralPath $nativeBuildCmd -Encoding ASCII

Push-Location $repoRoot
try {
    & cmd.exe /d /c "`"$nativeBuildCmd`""
    if ($LASTEXITCODE -ne 0) {
        throw "Native build failed with exit code $LASTEXITCODE."
    }

    & $csc /nologo /target:winexe /platform:x64 /codepage:65001 /utf8output /out:"$distDir\start.exe" /reference:System.dll /reference:System.Core.dll /reference:System.Drawing.dll /reference:System.Windows.Forms.dll "$repoRoot\launcher\TronStudio.cs"
    if ($LASTEXITCODE -ne 0) {
        throw "Launcher build failed with exit code $LASTEXITCODE."
    }
}
finally {
    Pop-Location
}

$utf8NoBom = [System.Text.UTF8Encoding]::new($false)
$defaultTargets = [System.IO.File]::ReadAllText((Join-Path $repoRoot "profanity.txt"), [System.Text.Encoding]::UTF8)
$targetLines = $defaultTargets -replace "`r`n", "`n" -replace "`r", "`n" -split "`n" |
    ForEach-Object { $_.Trim().TrimStart([char]0xFEFF) } |
    Where-Object { $_.Length -gt 0 }
$normalizedTargets = ($targetLines -join "`r`n") + "`r`n"
[System.IO.File]::WriteAllText((Join-Path $distDir "profanity.txt"), $normalizedTargets, $utf8NoBom)
[System.IO.File]::WriteAllText((Join-Path $runtimeDir "targets.txt"), $normalizedTargets, $utf8NoBom)
Copy-Item -Path (Join-Path $repoRoot "kernels\*.cl") -Destination $kernelDistDir -Force

@"
@echo off
setlocal
cd /d "%~dp0"
start "" "%~dp0start.exe"
"@ | Set-Content -LiteralPath (Join-Path $distDir "start.bat") -Encoding ASCII

$packageReadme = [System.IO.File]::ReadAllText((Join-Path $repoRoot "docs\package-README.md"), [System.Text.Encoding]::UTF8)
[System.IO.File]::WriteAllText((Join-Path $distDir "README.md"), $packageReadme, $utf8NoBom)

if ([string]::IsNullOrWhiteSpace($Version)) {
    $Version = (& git -C $repoRoot describe --tags --always --dirty 2>$null)
    if ([string]::IsNullOrWhiteSpace($Version)) {
        $Version = "local"
    }
}

@"
profanity-tron $Version
Built: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz")
Launcher: start.exe
Generator: shiyi.exe
"@ | Set-Content -LiteralPath (Join-Path $distDir "VERSION.txt") -Encoding UTF8

$zipPath = Join-Path $distDir "shiyi-$Version.zip"
if (Test-Path -LiteralPath $zipPath) {
    Remove-Item -LiteralPath $zipPath -Force
}

if (Test-Path -LiteralPath $packageRoot) {
    Remove-Item -LiteralPath $packageRoot -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $packageRoot | Out-Null
Copy-Item -LiteralPath (Join-Path $distDir "start.exe") -Destination $packageRoot -Force
Copy-Item -LiteralPath (Join-Path $distDir "shiyi.exe") -Destination $packageRoot -Force
Copy-Item -LiteralPath (Join-Path $distDir "start.bat") -Destination $packageRoot -Force
Copy-Item -LiteralPath (Join-Path $distDir "README.md") -Destination $packageRoot -Force
Copy-Item -LiteralPath (Join-Path $distDir "VERSION.txt") -Destination $packageRoot -Force
Copy-Item -LiteralPath (Join-Path $distDir "profanity.txt") -Destination $packageRoot -Force
Copy-Item -LiteralPath $runtimeDir -Destination $packageRoot -Recurse -Force
Copy-Item -LiteralPath $kernelDistDir -Destination $packageRoot -Recurse -Force

Compress-Archive -Path (Join-Path $packageRoot "*") -DestinationPath $zipPath -Force
Write-Host "Built $zipPath"
