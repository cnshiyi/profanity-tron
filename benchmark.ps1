param(
    [string]$Version = "perf-v002",
    [string]$ExePath = ".\\profanity.x64.exe",
    [string]$Matching = "TUqEg3dzVEJNQSVW2HY98z5X8SBdhmao8D",
    [int]$PrefixCount = 1,
    [int]$SuffixCount = 1,
    [int]$QuitCount = 1,
    [int]$BenchmarkSeconds = 0,
    [string]$Notes = "",
    [switch]$BuildFirst
)

$ErrorActionPreference = "Stop"

if ($BuildFirst)
{
    & (Join-Path $PSScriptRoot "build.ps1")
}

$resolvedExePath = if ([System.IO.Path]::IsPathRooted($ExePath)) {
    $ExePath
} else {
    [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot $ExePath))
}
$command = "$resolvedExePath --matching $Matching --prefix-count $PrefixCount --suffix-count $SuffixCount"
if ($BenchmarkSeconds -gt 0)
{
    $command += " --benchmark-seconds $BenchmarkSeconds"
}
else
{
    $command += " --quit-count $QuitCount"
}
$stdoutPath = Join-Path $PSScriptRoot "experiments\\tmp\\benchmark_stdout.txt"
$stderrPath = Join-Path $PSScriptRoot "experiments\\tmp\\benchmark_stderr.txt"
$tmpDir = Split-Path $stdoutPath -Parent
if (!(Test-Path $tmpDir))
{
    New-Item -ItemType Directory -Path $tmpDir | Out-Null
}

$argumentList = @("--matching", $Matching, "--prefix-count", "$PrefixCount", "--suffix-count", "$SuffixCount")
if ($BenchmarkSeconds -gt 0)
{
    $argumentList += @("--benchmark-seconds", "$BenchmarkSeconds")
}
else
{
    $argumentList += @("--quit-count", "$QuitCount")
}

$process = Start-Process -FilePath $resolvedExePath `
    -ArgumentList $argumentList `
    -RedirectStandardOutput $stdoutPath `
    -RedirectStandardError $stderrPath `
    -NoNewWindow `
    -PassThru `
    -Wait

$output = ""
if (Test-Path $stdoutPath)
{
    $output += Get-Content $stdoutPath -Raw
}
if (Test-Path $stderrPath)
{
    $output += "`n" + (Get-Content $stderrPath -Raw)
}
$output = $output -replace "`e\\[[\d;]*[A-Za-z]", ""

$deviceName = ""
$vendor = ""
$work = ""
$inverseMultiple = ""
$workMax = ""
$speed = ""
$initSeconds = ""

if ($output -match "GPU-\d+:\s*(.+?),\s*\d+\s+bytes available")
{
    $deviceName = $Matches[1].Trim()
}
if ($deviceName -match "NVIDIA")
{
    $vendor = "NVIDIA"
}
elseif ($deviceName -match "Intel")
{
    $vendor = "Intel"
}
else
{
    $vendor = "Unknown"
}

if ($output -match "work = (\d+)")
{
    $work = $Matches[1]
}
if ($output -match "inverse-multiple = (\d+)")
{
    $inverseMultiple = $Matches[1]
}
if ($output -match "work-max = (\d+)")
{
    $workMax = $Matches[1]
}
if ($output -match "Initialization time: (\d+) seconds")
{
    $initSeconds = $Matches[1]
}
if ($output -match "Final:\s+([0-9.]+)\s+MH/s")
{
    $speed = $Matches[1]
}
elseif ($output -match "Total:\s+([0-9.]+)\s+MH/s")
{
    $speed = $Matches[1]
}

$summaryPath = Join-Path $PSScriptRoot "experiments\\benchmark_summary.csv"
$date = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
$escapedCommand = '"' + $command.Replace('"', '""') + '"'
$escapedNotes = '"' + $Notes.Replace('"', '""') + '"'
$buildOk = if ($process.ExitCode -eq 0) { "True" } else { "False" }
$line = "$Version,$date,$deviceName,$vendor,$escapedCommand,$work,$inverseMultiple,$workMax,$speed,$initSeconds,$buildOk,$escapedNotes"
Add-Content -Path $summaryPath -Value $line -Encoding UTF8

Write-Output $output
