param(
    [string]$Version = ""
)

$ErrorActionPreference = "Stop"

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$distDir = Join-Path $repoRoot "dist"
$runtimeDir = Join-Path $distDir "runtime"
$buildDir = Join-Path $repoRoot "build\windows"
$objDir = Join-Path $buildDir "obj"

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

New-Item -ItemType Directory -Force -Path $distDir, $runtimeDir, $objDir | Out-Null

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
cl.exe /c /I"$repoRoot\Curl\include" /I"$repoRoot\OpenCL\include" /Zi /nologo /W1 /WX- /diagnostics:column /O2 /D BUILDING_LIBCURL /D HTTP_ONLY /Gm- /EHsc /MD /GS /fp:precise /Zc:wchar_t /Zc:forScope /Zc:inline /Fo"$objDir\\" /external:W1 /Gd /TP /FC /errorReport:prompt "$repoRoot\Dispatcher.cpp" "$repoRoot\Mode.cpp" "$repoRoot\precomp.cpp" "$repoRoot\profanity.cpp" "$repoRoot\SpeedSample.cpp"
if errorlevel 1 exit /b %errorlevel%
link.exe /ERRORREPORT:PROMPT /OUT:"$distDir\profanity.x64.exe" /NOLOGO /INCREMENTAL:NO /LIBPATH:"$repoRoot\Curl\lib" /LIBPATH:"$repoRoot\OpenCL\lib" OpenCL.lib libcurl_a.lib ws2_32.lib winmm.lib wldap32.lib crypt32.lib normaliz.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /MANIFEST /MANIFESTUAC:"level='asInvoker' uiAccess='false'" /manifest:embed /DEBUG:FULL /PDB:"$buildDir\profanity.pdb" /TLBID:1 /DYNAMICBASE /NXCOMPAT /IMPLIB:"$buildDir\profanity.lib" /MACHINE:X64 "$objDir\*.obj"
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

Copy-Item -LiteralPath (Join-Path $repoRoot "profanity.txt") -Destination (Join-Path $distDir "profanity.txt") -Force
Copy-Item -LiteralPath (Join-Path $repoRoot "profanity.txt") -Destination (Join-Path $runtimeDir "targets.txt") -Force

@"
@echo off
setlocal
cd /d "%~dp0"
start "" "%~dp0start.exe"
"@ | Set-Content -LiteralPath (Join-Path $distDir "start.bat") -Encoding ASCII

@"
# profanity-tron Windows package

Run ``start.exe`` to open the Windows launcher.

Files:

- ``start.exe``: graphical launcher
- ``profanity.x64.exe``: OpenCL generator used by the launcher
- ``runtime\targets.txt``: default target list
- ``profanity.txt``: copy of the default target list
"@ | Set-Content -LiteralPath (Join-Path $distDir "README.md") -Encoding UTF8

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
Generator: profanity.x64.exe
"@ | Set-Content -LiteralPath (Join-Path $distDir "VERSION.txt") -Encoding UTF8

$zipPath = Join-Path $distDir "profanity-tron-windows-$Version.zip"
if (Test-Path -LiteralPath $zipPath) {
    Remove-Item -LiteralPath $zipPath -Force
}

$packageItems = @(
    (Join-Path $distDir "start.exe"),
    (Join-Path $distDir "profanity.x64.exe"),
    (Join-Path $distDir "start.bat"),
    (Join-Path $distDir "README.md"),
    (Join-Path $distDir "VERSION.txt"),
    (Join-Path $distDir "profanity.txt"),
    (Join-Path $runtimeDir "targets.txt")
)

Compress-Archive -LiteralPath $packageItems -DestinationPath $zipPath -Force
Write-Host "Built $zipPath"
