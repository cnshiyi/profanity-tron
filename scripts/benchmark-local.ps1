param(
    [string]$ExePath = ".\dist\shiyi.exe",
    [string]$Name = "benchmark",
    [string[]]$Arguments = @(),
    [string]$ArgumentLine = "",
    [int]$TimeoutSeconds = 180
)

$ErrorActionPreference = "Stop"

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$resolvedExe = if ([System.IO.Path]::IsPathRooted($ExePath)) {
    (Resolve-Path -LiteralPath $ExePath).Path
} else {
    (Resolve-Path -LiteralPath (Join-Path $repoRoot $ExePath)).Path
}

$exeDir = Split-Path -Parent $resolvedExe
$tmpDir = Join-Path $repoRoot "experiments\tmp"
New-Item -ItemType Directory -Force -Path $tmpDir | Out-Null

Get-Process shiyi,profanity* -ErrorAction SilentlyContinue |
    Where-Object { $_.Path -and $_.Path.StartsWith($exeDir, [System.StringComparison]::OrdinalIgnoreCase) } |
    Stop-Process -Force -ErrorAction SilentlyContinue

$stdoutPath = Join-Path $tmpDir "$Name.stdout.txt"
$stderrPath = Join-Path $tmpDir "$Name.stderr.txt"
Remove-Item -LiteralPath $stdoutPath, $stderrPath -Force -ErrorAction SilentlyContinue

if (-not [string]::IsNullOrWhiteSpace($ArgumentLine)) {
    $Arguments = $ArgumentLine -split '\s+' | Where-Object { $_.Length -gt 0 }
}

$startInfo = [System.Diagnostics.ProcessStartInfo]::new()
$startInfo.FileName = $resolvedExe
$startInfo.WorkingDirectory = $exeDir
$startInfo.UseShellExecute = $false
$startInfo.RedirectStandardOutput = $true
$startInfo.RedirectStandardError = $true
$startInfo.Arguments = ($Arguments | ForEach-Object {
    if ($_ -match '[\s"]') {
        '"' + ($_ -replace '"', '\"') + '"'
    } else {
        $_
    }
}) -join ' '

$process = [System.Diagnostics.Process]::new()
$process.StartInfo = $startInfo
try {
    [void]$process.Start()
} catch [System.ComponentModel.Win32Exception] {
    $message = $_.Exception.Message
    Set-Content -LiteralPath $stdoutPath -Value "" -Encoding UTF8
    Set-Content -LiteralPath $stderrPath -Value $message -Encoding UTF8
    [pscustomobject]@{
        Name = $Name
        ExitCode = "BLOCKED"
        Speed = ""
        Stdout = $stdoutPath
        Stderr = $stderrPath
        Error = $message
    }
    return
}

if (-not $process.WaitForExit($TimeoutSeconds * 1000)) {
    Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue
    throw "Benchmark '$Name' timed out after $TimeoutSeconds seconds."
}

$stdout = $process.StandardOutput.ReadToEnd()
$stderr = $process.StandardError.ReadToEnd()
Set-Content -LiteralPath $stdoutPath -Value $stdout -Encoding UTF8
Set-Content -LiteralPath $stderrPath -Value $stderr -Encoding UTF8
$exitCode = $process.ExitCode

$output = ""
if (Test-Path -LiteralPath $stdoutPath) {
    $output += Get-Content -LiteralPath $stdoutPath -Raw
}
if (Test-Path -LiteralPath $stderrPath) {
    $output += "`n" + (Get-Content -LiteralPath $stderrPath -Raw)
}
$clean = $output -replace "`e\[[\d;]*[A-Za-z]", ""

$speed = ""
if ($clean -match "Final:\s+([0-9.]+)\s+([KMGT]?H/s)") {
    $speed = "$($Matches[1]) $($Matches[2])"
}

[pscustomobject]@{
    Name = $Name
    ExitCode = $exitCode
    Speed = $speed
    Stdout = $stdoutPath
    Stderr = $stderrPath
}
