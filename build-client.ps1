$ErrorActionPreference = "Stop"

$repo = $PSScriptRoot
$source = Join-Path $repo "TronStudio.cs"
$output = Join-Path $repo "TronStudio.exe"
$csc = "C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe"

if (!(Test-Path $csc)) {
    $csc = "C:\Windows\Microsoft.NET\Framework\v4.0.30319\csc.exe"
}

if (!(Test-Path $csc)) {
    throw "Unable to find csc.exe"
}

& $csc /nologo /target:winexe /platform:x64 /optimize+ /utf8output /out:"$output" /reference:System.dll /reference:System.Core.dll /reference:System.Drawing.dll /reference:System.Windows.Forms.dll "$source"

if ($LASTEXITCODE -ne 0) {
    throw "Client build failed with exit code $LASTEXITCODE"
}

Write-Host "Build complete: $output"
