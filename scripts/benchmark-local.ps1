param(
    [string]$ExePath = ".\dist\shiyi.exe",
    [string]$Name = "benchmark",
    [string[]]$Arguments = @(),
    [string]$ArgumentLine = "",
    [int]$TimeoutSeconds = 180,
    [switch]$StopExisting
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

$existingProcesses = @(Get-Process shiyi,profanity* -ErrorAction SilentlyContinue |
    Where-Object { $_.Path -and $_.Path.StartsWith($exeDir, [System.StringComparison]::OrdinalIgnoreCase) })
if ($existingProcesses.Count -gt 0) {
    if ($StopExisting) {
        $existingProcesses | Stop-Process -Force -ErrorAction SilentlyContinue
    } else {
        $existingList = ($existingProcesses | ForEach-Object { "$($_.ProcessName)($($_.Id))" }) -join ", "
        [pscustomobject]@{
            Name = $Name
            ExitCode = "RUNNING"
            Speed = ""
            Stdout = ""
            Stderr = ""
            Error = "Benchmark target already has running processes in $exeDir`: $existingList. Re-run with -StopExisting to clean them explicitly."
        }
        return
    }
}

$stdoutPath = Join-Path $tmpDir "$Name.stdout.txt"
$stderrPath = Join-Path $tmpDir "$Name.stderr.txt"
Remove-Item -LiteralPath $stdoutPath, $stderrPath -Force -ErrorAction SilentlyContinue

if (-not [string]::IsNullOrWhiteSpace($ArgumentLine)) {
    $Arguments = $ArgumentLine -split '\s+' | Where-Object { $_.Length -gt 0 }
}

$argumentText = ($Arguments | ForEach-Object {
    if ($_ -match '[\s"]') {
        '"' + ($_ -replace '"', '\"') + '"'
    } else {
        $_
    }
}) -join ' '

try {
    $startInfo = [System.Diagnostics.ProcessStartInfo]::new()
    $startInfo.FileName = $resolvedExe
    $startInfo.WorkingDirectory = $exeDir
    $startInfo.UseShellExecute = $false
    $startInfo.CreateNoWindow = $true
    $startInfo.RedirectStandardOutput = $true
    $startInfo.RedirectStandardError = $true
    $startInfo.Arguments = $argumentText

    $process = [System.Diagnostics.Process]::new()
    $process.StartInfo = $startInfo
    [void]$process.Start()
    $stdoutTask = $process.StandardOutput.ReadToEndAsync()
    $stderrTask = $process.StandardError.ReadToEndAsync()
} catch {
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
    [void]$process.WaitForExit(5000)
    Set-Content -LiteralPath $stdoutPath -Value $stdoutTask.Result -Encoding UTF8
    Set-Content -LiteralPath $stderrPath -Value $stderrTask.Result -Encoding UTF8
    [pscustomobject]@{
        Name = $Name
        ExitCode = "TIMEOUT"
        Speed = ""
        Stdout = $stdoutPath
        Stderr = $stderrPath
        Error = "Timed out after $TimeoutSeconds seconds."
    }
    return
}

$process.WaitForExit()
$stdout = $stdoutTask.Result
$stderr = $stderrTask.Result
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
