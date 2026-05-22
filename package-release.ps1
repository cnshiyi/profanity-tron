param(
    [Parameter(Mandatory = $true)]
    [string]$Version,

    [string]$OutputDirectory = ""
)

$ErrorActionPreference = "Stop"

$repo = $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($OutputDirectory)) {
    $OutputDirectory = $repo
}

$dist = Join-Path $OutputDirectory "profanity-tron-windows-$Version"
$zip = Join-Path $OutputDirectory "profanity-tron-windows-$Version.zip"

if (Test-Path $dist) {
    Remove-Item -LiteralPath $dist -Recurse -Force
}
if (Test-Path $zip) {
    Remove-Item -LiteralPath $zip -Force
}

New-Item -ItemType Directory -Force -Path $dist | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $dist "kernels") | Out-Null

Copy-Item -LiteralPath (Join-Path $repo "start.exe") -Destination (Join-Path $dist "start.exe") -Force
Copy-Item -LiteralPath (Join-Path $repo "shiyi.exe") -Destination (Join-Path $dist "shiyi.exe") -Force
Copy-Item -LiteralPath (Join-Path $repo "BUILD.md") -Destination (Join-Path $dist "BUILD.md") -Force
Copy-Item -LiteralPath (Join-Path $repo "README.md") -Destination (Join-Path $dist "README.md") -Force
Copy-Item -LiteralPath (Join-Path $repo "kernels\kernel_keccak.cl") -Destination (Join-Path $dist "kernels\kernel_keccak.cl") -Force
Copy-Item -LiteralPath (Join-Path $repo "kernels\kernel_sha256.cl") -Destination (Join-Path $dist "kernels\kernel_sha256.cl") -Force
Copy-Item -LiteralPath (Join-Path $repo "kernels\kernel_profanity.cl") -Destination (Join-Path $dist "kernels\kernel_profanity.cl") -Force

Compress-Archive -Path (Join-Path $dist "*") -DestinationPath $zip
Write-Host "Release package complete: $zip"
