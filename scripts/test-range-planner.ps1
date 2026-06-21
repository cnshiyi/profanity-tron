param()

$ErrorActionPreference = "Stop"

function Assert-Equal {
    param(
        [string]$Name,
        [string]$Expected,
        [string]$Actual
    )
    if ($Expected -ne $Actual) {
        throw "$Name expected $Expected but got $Actual"
    }
}

function Normalize-Key {
    param([string]$Value)
    $text = ($Value -replace "\s+", "")
    if ($text.StartsWith("0x", [System.StringComparison]::OrdinalIgnoreCase)) {
        $text = $text.Substring(2)
    }
    if ($text.Length -eq 0 -or $text.Length -gt 64 -or $text -notmatch "^[0-9a-fA-F]+$") {
        throw "invalid private key"
    }
    return $text.PadLeft(64, '0').ToLowerInvariant()
}

function Align-Range-Start {
    param(
        [string]$StartKey,
        [int]$Digits,
        [bool]$DirectionUp
    )
    $chars = (Normalize-Key $StartKey).ToCharArray()
    $first = 64 - $Digits
    $fill = if ($DirectionUp) { '0' } else { 'f' }
    for ($i = $first; $i -lt 64; $i++) {
        $chars[$i] = $fill
    }
    return -join $chars
}

function Get-Range-End {
    param(
        [string]$StartKey,
        [int]$Digits,
        [bool]$DirectionUp
    )
    $chars = (Normalize-Key $StartKey).ToCharArray()
    $first = 64 - $Digits
    $limit = if ($Digits -eq 16) { [UInt64]::MaxValue } else { ([UInt64]1 -shl ($Digits * 4)) - 1 }
    $value = [Convert]::ToUInt64((Normalize-Key $StartKey).Substring($first, $Digits), 16)
    $endValue = if ($DirectionUp) { $limit } else { [UInt64]0 }
    if ($DirectionUp -and $value -eq $limit) { $endValue = $value }
    if ((-not $DirectionUp) -and $value -eq 0) { $endValue = $value }
    $replacement = $endValue.ToString("x").PadLeft($Digits, '0')
    for ($i = 0; $i -lt $Digits; $i++) {
        $chars[$first + $i] = $replacement[$i]
    }
    return -join $chars
}

function Get-Next-Range-Start {
    param(
        [string]$StartKey,
        [int]$Digits,
        [bool]$DirectionUp
    )
    $chars = (Normalize-Key $StartKey).ToCharArray()
    $first = 64 - $Digits
    if ($DirectionUp) {
        for ($i = $first; $i -lt 64; $i++) { $chars[$i] = '0' }
        for ($i = $first - 1; $i -ge 0; $i--) {
            $value = [Convert]::ToInt32([string]$chars[$i], 16)
            if ($value -lt 15) {
                $chars[$i] = "0123456789abcdef"[$value + 1]
                return -join $chars
            }
            $chars[$i] = '0'
        }
        return $null
    }

    for ($i = $first; $i -lt 64; $i++) { $chars[$i] = 'f' }
    for ($i = $first - 1; $i -ge 0; $i--) {
        $value = [Convert]::ToInt32([string]$chars[$i], 16)
        if ($value -gt 0) {
            $chars[$i] = "0123456789abcdef"[$value - 1]
            return -join $chars
        }
        $chars[$i] = 'f'
    }
    return $null
}

$zero = Normalize-Key "0"
Assert-Equal "digits16-up-end" ("0" * 48 + "f" * 16) (Get-Range-End $zero 16 $true)
Assert-Equal "digits16-up-next" ("0" * 47 + "1" + "0" * 16) (Get-Next-Range-Start $zero 16 $true)

$randomLike = Normalize-Key "1123456789abcdef0"
Assert-Equal "random-digits16-up-align" ("0" * 47 + "1" + "0" * 16) (Align-Range-Start $randomLike 16 $true)
Assert-Equal "random-digits16-down-align" ("0" * 47 + "1" + "f" * 16) (Align-Range-Start $randomLike 16 $false)

$near = Normalize-Key "12ab"
Assert-Equal "digits2-up-end" (Normalize-Key "12ff") (Get-Range-End $near 2 $true)
Assert-Equal "digits2-up-next-block" (Normalize-Key "1300") (Get-Next-Range-Start $near 2 $true)

$downStart = Normalize-Key "1300"
Assert-Equal "digits2-down-end" (Normalize-Key "1300") (Get-Range-End $downStart 2 $false)
Assert-Equal "digits2-down-next-block" (Normalize-Key "12ff") (Get-Next-Range-Start $downStart 2 $false)

$source = Get-Content -LiteralPath (Join-Path $PSScriptRoot "..\launcher\TronStudio.cs") -Raw
if ($source -notmatch "AlignRangeStart\(CreateRandomPrivateKey\(\), digits, directionUp\)") {
    throw "random fixed-digit mode must align the random start to a full digit window"
}
if ($source -notmatch "if \(overrideStartKey != null\)\s*\{\s*directionUp = rangeDirectionUp;") {
    throw "auto-continued fixed-digit runs must keep the previously selected direction"
}
if ($source -notmatch "rangeAutoContinue = false;\s*rangeNextStartKey = null;\s*var proc = runningProcess;") {
    throw "StopGeneration must disable auto-continue before killing the current process"
}
if ($source -notmatch "digits < 8" -or $source -notmatch "digits > 8") {
    throw "launcher must keep explicit fixed-digit advance branches for <8, =8, and >8 digits"
}

Write-Host "range planner expectations passed"
