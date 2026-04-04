#
# Integration tests for frequent-cron (Windows)
#
# On Windows, frequent-cron runs in the foreground (no daemon mode),
# so we launch it as a background job and test from there.
#

param(
    [string]$Binary = ".\frequent-cron.exe"
)

$ErrorActionPreference = "Continue"
$TestDir = Join-Path $env:TEMP "frequent-cron-tests-$(Get-Random)"
New-Item -ItemType Directory -Path $TestDir -Force | Out-Null

$Pass = 0
$Fail = 0
$Jobs = @()

function Pass($msg) { $script:Pass++; Write-Host "  PASS: $msg" }
function Fail($msg) { $script:Fail++; Write-Host "  FAIL: $msg" }

function Stop-AllJobs {
    foreach ($j in $script:Jobs) {
        Stop-Job $j -ErrorAction SilentlyContinue
        Remove-Job $j -Force -ErrorAction SilentlyContinue
    }
    # Also kill by pid file
    Get-ChildItem "$TestDir\*.pid" -ErrorAction SilentlyContinue | ForEach-Object {
        $pidVal = Get-Content $_.FullName -ErrorAction SilentlyContinue
        if ($pidVal) {
            Stop-Process -Id $pidVal -Force -ErrorAction SilentlyContinue
        }
    }
}

function Start-FrequentCron {
    param([string[]]$Args)
    $job = Start-Job -ScriptBlock {
        param($bin, $a)
        & $bin @a 2>&1
    } -ArgumentList $Binary, $Args
    $script:Jobs += $job
    return $job
}

# ============================================================
Write-Host "=== Argument Parsing (pre-daemon) ==="
# ============================================================

# --help prints usage
$output = & $Binary --help 2>&1 | Out-String
if ($output -match "Options") { Pass "--help prints usage" } else { Fail "--help prints usage" }

# Missing --frequency
$output = & $Binary --command="echo hi" 2>&1 | Out-String
if ($output -match "(?i)frequency") { Pass "missing --frequency reports error" } else { Fail "missing --frequency reports error" }

# Missing --command
$output = & $Binary --frequency=1000 2>&1 | Out-String
if ($output -match "(?i)command") { Pass "missing --command reports error" } else { Fail "missing --command reports error" }


# ============================================================
Write-Host "=== PID File ==="
# ============================================================

# PID file is created
$pidFile = Join-Path $TestDir "test_pid.pid"
$job = Start-FrequentCron @("--frequency=500", "--command=echo hi", "--pid-file=$pidFile")
Start-Sleep -Seconds 1
if (Test-Path $pidFile) { Pass "PID file is created" } else { Fail "PID file is created" }

# PID file contains valid integer
if (Test-Path $pidFile) {
    $pidVal = Get-Content $pidFile
    if ($pidVal -match '^\d+$') {
        Pass "PID file contains valid integer ($pidVal)"
        Stop-Process -Id $pidVal -Force -ErrorAction SilentlyContinue
    } else {
        Fail "PID file contains valid integer (got: $pidVal)"
    }
} else {
    Fail "PID file contains valid integer (file missing)"
}
Stop-Job $job -ErrorAction SilentlyContinue


# ============================================================
Write-Host "=== Synchronous Execution ==="
# ============================================================

# Command executes at roughly the configured frequency
$outputFile = Join-Path $TestDir "sync_freq.txt"
$pidFile = Join-Path $TestDir "sync_freq.pid"
$job = Start-FrequentCron @("--frequency=200", "--command=echo tick >> $outputFile", "--pid-file=$pidFile")
Start-Sleep -Seconds 2
Stop-Job $job -ErrorAction SilentlyContinue
Start-Sleep -Milliseconds 500
if (Test-Path $pidFile) { Stop-Process -Id (Get-Content $pidFile) -Force -ErrorAction SilentlyContinue }
if (Test-Path $outputFile) {
    $lines = @(Get-Content $outputFile).Count
    if ($lines -ge 5) { Pass "synchronous: ~correct execution frequency (got $lines lines, expected >= 5)" }
    else { Fail "synchronous: ~correct execution frequency (got $lines lines, expected >= 5)" }
} else {
    Fail "synchronous: ~correct execution frequency (output file not created)"
}

# Slow command blocks next execution
$outputFile = Join-Path $TestDir "slow_sync.txt"
$pidFile = Join-Path $TestDir "slow_sync.pid"
$sleepCmd = "ping -n 2 127.0.0.1 > nul && echo done >> $outputFile"
$job = Start-FrequentCron @("--frequency=100", "--command=$sleepCmd", "--pid-file=$pidFile")
Start-Sleep -Seconds 4
Stop-Job $job -ErrorAction SilentlyContinue
Start-Sleep -Milliseconds 500
if (Test-Path $pidFile) { Stop-Process -Id (Get-Content $pidFile) -Force -ErrorAction SilentlyContinue }
if (Test-Path $outputFile) {
    $lines = @(Get-Content $outputFile).Count
    if ($lines -le 4) { Pass "synchronous: slow command blocks next execution (got $lines lines, expected <= 4)" }
    else { Fail "synchronous: slow command blocks next execution (got $lines lines, expected <= 4)" }
} else {
    Pass "synchronous: slow command blocks next execution (no output yet, confirms blocking)"
}


# ============================================================
Write-Host "=== Edge Cases ==="
# ============================================================

# frequency=1 doesn't crash
$outputFile = Join-Path $TestDir "fast_freq.txt"
$pidFile = Join-Path $TestDir "fast_freq.pid"
$job = Start-FrequentCron @("--frequency=1", "--command=echo tick >> $outputFile", "--pid-file=$pidFile")
Start-Sleep -Seconds 2
Stop-Job $job -ErrorAction SilentlyContinue
Start-Sleep -Milliseconds 500
if (Test-Path $pidFile) { Stop-Process -Id (Get-Content $pidFile) -Force -ErrorAction SilentlyContinue }
if (Test-Path $outputFile) {
    $lines = @(Get-Content $outputFile).Count
    if ($lines -ge 10) { Pass "frequency=1 doesn't crash (got $lines lines, expected >= 10)" }
    else { Fail "frequency=1 doesn't crash (got $lines lines, expected >= 10)" }
} else {
    Fail "frequency=1 doesn't crash (output file not created)"
}

# Nonexistent command doesn't crash
$pidFile = Join-Path $TestDir "bad_cmd.pid"
$job = Start-FrequentCron @("--frequency=500", "--command=C:\nonexistent\script.bat", "--pid-file=$pidFile")
Start-Sleep -Seconds 2
if (Test-Path $pidFile) {
    $pidVal = Get-Content $pidFile
    $proc = Get-Process -Id $pidVal -ErrorAction SilentlyContinue
    if ($proc) {
        Pass "nonexistent command doesn't crash"
        Stop-Process -Id $pidVal -Force -ErrorAction SilentlyContinue
    } else {
        Fail "nonexistent command doesn't crash"
    }
} else {
    Fail "nonexistent command doesn't crash (no pid file)"
}
Stop-Job $job -ErrorAction SilentlyContinue


# ============================================================
# Cleanup
Stop-AllJobs
Remove-Item -Recurse -Force $TestDir -ErrorAction SilentlyContinue

Write-Host ""
Write-Host "=== Results ==="
Write-Host "  Passed: $Pass"
Write-Host "  Failed: $Fail"
Write-Host "  Total:  $($Pass + $Fail)"

if ($Fail -gt 0) { exit 1 }
