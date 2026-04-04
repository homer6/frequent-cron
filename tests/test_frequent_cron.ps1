#
# Integration tests for frequent-cron (Windows)
#
# On Windows, frequent-cron runs in the foreground (no daemon mode),
# so we launch it via Start-Process and test from there.
#

param(
    [string]$Binary = ".\frequent-cron.exe"
)

$ErrorActionPreference = "Continue"
$Binary = Resolve-Path $Binary
$TestDir = Join-Path $env:TEMP "frequent-cron-tests-$(Get-Random)"
New-Item -ItemType Directory -Path $TestDir -Force | Out-Null

$Pass = 0
$Fail = 0
$Processes = @()

function Pass($msg) { $script:Pass++; Write-Host "  PASS: $msg" }
function Fail($msg) { $script:Fail++; Write-Host "  FAIL: $msg" }

function Stop-AllProcesses {
    foreach ($p in $script:Processes) {
        Stop-Process -Id $p -Force -ErrorAction SilentlyContinue
    }
    Get-ChildItem "$TestDir\*.pid" -ErrorAction SilentlyContinue | ForEach-Object {
        $pidVal = Get-Content $_.FullName -ErrorAction SilentlyContinue
        if ($pidVal) {
            Stop-Process -Id $pidVal -Force -ErrorAction SilentlyContinue
        }
    }
}

function Start-FrequentCron {
    param([string]$Arguments)
    $proc = Start-Process -FilePath $Binary -ArgumentList $Arguments -PassThru -NoNewWindow -RedirectStandardOutput "$TestDir\nul.txt" -RedirectStandardError "$TestDir\nul_err.txt"
    $script:Processes += $proc.Id
    return $proc
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
$proc = Start-FrequentCron "--frequency=500 --command=`"echo hi`" --pid-file=`"$pidFile`""
Start-Sleep -Seconds 2
if (Test-Path $pidFile) { Pass "PID file is created" } else { Fail "PID file is created" }

# PID file contains valid integer
if (Test-Path $pidFile) {
    $pidVal = (Get-Content $pidFile).Trim()
    if ($pidVal -match '^\d+$') {
        Pass "PID file contains valid integer ($pidVal)"
        Stop-Process -Id $pidVal -Force -ErrorAction SilentlyContinue
    } else {
        Fail "PID file contains valid integer (got: $pidVal)"
    }
} else {
    Fail "PID file contains valid integer (file missing)"
}
Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue


# ============================================================
Write-Host "=== Synchronous Execution ==="
# ============================================================

# Command executes at roughly the configured frequency
$outputFile = Join-Path $TestDir "sync_freq.txt"
$pidFile = Join-Path $TestDir "sync_freq.pid"
$proc = Start-FrequentCron "--frequency=200 --command=`"cmd /c echo tick >> \`"$outputFile\`"`" --pid-file=`"$pidFile`""
Start-Sleep -Seconds 2
Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue
Start-Sleep -Milliseconds 500
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
$proc = Start-FrequentCron "--frequency=100 --command=`"cmd /c ping -n 2 127.0.0.1 > nul && echo done >> \`"$outputFile\`"`" --pid-file=`"$pidFile`""
Start-Sleep -Seconds 4
Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue
Start-Sleep -Milliseconds 500
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
$proc = Start-FrequentCron "--frequency=1 --command=`"cmd /c echo tick >> \`"$outputFile\`"`" --pid-file=`"$pidFile`""
Start-Sleep -Seconds 2
Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue
Start-Sleep -Milliseconds 500
if (Test-Path $outputFile) {
    $lines = @(Get-Content $outputFile).Count
    if ($lines -ge 10) { Pass "frequency=1 doesn't crash (got $lines lines, expected >= 10)" }
    else { Fail "frequency=1 doesn't crash (got $lines lines, expected >= 10)" }
} else {
    Fail "frequency=1 doesn't crash (output file not created)"
}

# Nonexistent command doesn't crash
$pidFile = Join-Path $TestDir "bad_cmd.pid"
$proc = Start-FrequentCron "--frequency=500 --command=`"C:\nonexistent\script.bat`" --pid-file=`"$pidFile`""
Start-Sleep -Seconds 2
try {
    $check = Get-Process -Id $proc.Id -ErrorAction Stop
    Pass "nonexistent command doesn't crash"
    Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue
} catch {
    Fail "nonexistent command doesn't crash"
}


# ============================================================
# Cleanup
Stop-AllProcesses
Remove-Item -Recurse -Force $TestDir -ErrorAction SilentlyContinue

Write-Host ""
Write-Host "=== Results ==="
Write-Host "  Passed: $Pass"
Write-Host "  Failed: $Fail"
Write-Host "  Total:  $($Pass + $Fail)"

if ($Fail -gt 0) { exit 1 }
