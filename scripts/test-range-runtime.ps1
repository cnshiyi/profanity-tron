param(
    [string]$ExePath = ".\dist\shiyi.exe"
)

$ErrorActionPreference = "Stop"

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$resolvedExe = if ([System.IO.Path]::IsPathRooted($ExePath)) {
    (Resolve-Path -LiteralPath $ExePath).Path
} else {
    (Resolve-Path -LiteralPath (Join-Path $repoRoot $ExePath)).Path
}

$exeDir = Split-Path -Parent $resolvedExe
$runDir = Join-Path $repoRoot "experiments\tmp\range-runtime"
New-Item -ItemType Directory -Force -Path $runDir | Out-Null

function Invoke-Benchmark {
    param(
        [string]$Name,
        [string]$ArgumentLine,
        [int]$TimeoutSeconds = 180
    )

    $result = & (Join-Path $PSScriptRoot "benchmark-local.ps1") `
        -ExePath $resolvedExe `
        -Name $Name `
        -ArgumentLine $ArgumentLine `
        -TimeoutSeconds $TimeoutSeconds `
        -StopExisting
    if ($result.ExitCode -ne 0) {
        throw "$Name failed with exit code $($result.ExitCode). stdout=$($result.Stdout) stderr=$($result.Stderr)"
    }
    return $result
}

function Read-OutputFromExeDir {
    param([string]$RelativePath)
    $path = Join-Path $exeDir $RelativePath
    if (-not (Test-Path -LiteralPath $path)) {
        throw "expected output file was not created: $path"
    }
    return Get-Content -LiteralPath $path
}

$enumRel = "experiments\tmp\range-runtime\enum_0_f.txt"
$enumAbs = Join-Path $exeDir $enumRel
Remove-Item -LiteralPath $enumAbs -Force -ErrorAction SilentlyContinue
Invoke-Benchmark `
    -Name "range_runtime_enum_0_f" `
    -ArgumentLine "--matching TTTTTTTTTTTTTTTTT11111111111111111 --prefix-count 0 --suffix-count 0 --quit-count 999 --output $enumRel --range-start 0000000000000000000000000000000000000000000000000000000000000000 --range-end 000000000000000000000000000000000000000000000000000000000000000f --range-direction up --work 32 --inverse-multiple 1024 --no-cache" | Out-Null

$enumLines = @(Read-OutputFromExeDir $enumRel)
if ($enumLines.Count -eq 0) {
    throw "range enum test produced no hits"
}

$privateValues = @()
foreach ($line in $enumLines) {
    if ($line -notmatch "private=([0-9a-fA-F]{64})") {
        throw "range enum line did not contain a private key: $line"
    }
    $privateValues += [Convert]::ToUInt64($Matches[1].Substring(48, 16), 16)
}

$min = ($privateValues | Measure-Object -Minimum).Minimum
$max = ($privateValues | Measure-Object -Maximum).Maximum
if ($min -lt 1 -or $max -gt 15) {
    throw "range enum escaped the 1..f window: min=$min max=$max"
}
if (($privateValues | Select-Object -Unique).Count -ne $privateValues.Count) {
    throw "range enum produced duplicate private keys inside the first bounded window"
}

$enumText = $enumLines -join "`n"
if ($enumText -match "\bscore=") {
    throw "result output must not include score="
}
foreach ($line in $enumLines) {
    if ($line -notmatch "^prefix=\d+\s+suffix=\d+\s+address=T[1-9A-HJ-NP-Za-km-z]{33}\s+private=[0-9a-f]{64}\s+time=\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}$") {
        throw "result output format must keep time at the end and omit score=: $line"
    }
}

$enumStderr = Get-Content -LiteralPath (Join-Path $repoRoot "experiments\tmp\range_runtime_enum_0_f.stderr.txt") -Raw
$enumClean = $enumStderr -replace "`e\[[\d;]*[A-Za-z]", ""
if ($enumClean -notmatch "Final:\s+[0-9.]+\s+H/s" -or $enumClean -match "Final:\s+[0-9.]+\s+[KMG]H/s") {
    throw "finite range speed accounting should report only newly covered candidates, not duplicate raw GPU work"
}

$knownRel = "experiments\tmp\range-runtime\known_suffix8.txt"
$knownAbs = Join-Path $exeDir $knownRel
Remove-Item -LiteralPath $knownAbs -Force -ErrorAction SilentlyContinue
Invoke-Benchmark `
    -Name "range_runtime_known_suffix8" `
    -ArgumentLine "--matching TJSWuhAyiDQyb7E8ne1AitDV62H8HXRnvU --prefix-count 0 --suffix-count 8 --quit-count 1 --output $knownRel --range-start 0000000000000000000000000000000000000000000000000000000000000000 --range-end 000000000000000000000000000000000000000000000000ffffffffffffffff --range-direction up --work 32 --inverse-multiple 196608 --no-cache" `
    -TimeoutSeconds 240 | Out-Null

$knownText = (Read-OutputFromExeDir $knownRel) -join "`n"
if ($knownText -notmatch "address=TJSWuhAyiDQyb7E8ne1AitDV62H8HXRnvU") {
    throw "known range hit did not report the expected address"
}
if ($knownText -notmatch "private=0000000000000000000000000000000000000000000000000000000002fcf6a3") {
    throw "known range hit did not report the expected private key"
}

Write-Host "range runtime expectations passed"
