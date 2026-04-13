#!/bin/bash
#
# Integration tests for frequent-cron
#

set -e

BINARY="${1:-./frequent-cron}"
TEST_TMPDIR=$(mktemp -d)
PASS=0
FAIL=0

cleanup() {
    # kill any leftover frequent-cron processes we started
    for pidfile in "$TEST_TMPDIR"/*.pid; do
        [ -f "$pidfile" ] && kill "$(cat "$pidfile")" 2>/dev/null || true
    done
    rm -rf "$TEST_TMPDIR"
}
trap cleanup EXIT

pass() {
    PASS=$((PASS + 1))
    echo "  PASS: $1"
}

fail() {
    FAIL=$((FAIL + 1))
    echo "  FAIL: $1"
}

assert_file_exists() {
    if [ -f "$1" ]; then pass "$2"; else fail "$2"; fi
}

assert_file_not_exists() {
    if [ ! -f "$1" ]; then pass "$2"; else fail "$2"; fi
}

assert_line_count_ge() {
    local file="$1" expected="$2" label="$3"
    local actual
    actual=$(wc -l < "$file" | tr -d ' ')
    if [ "$actual" -ge "$expected" ]; then
        pass "$label (got $actual lines, expected >= $expected)"
    else
        fail "$label (got $actual lines, expected >= $expected)"
    fi
}

assert_line_count_le() {
    local file="$1" expected="$2" label="$3"
    local actual
    actual=$(wc -l < "$file" | tr -d ' ')
    if [ "$actual" -le "$expected" ]; then
        pass "$label (got $actual lines, expected <= $expected)"
    else
        fail "$label (got $actual lines, expected <= $expected)"
    fi
}

# ============================================================
echo "=== Argument Parsing (pre-daemon) ==="
# ============================================================

# --help prints usage and exits (no daemonization happens)
$BINARY --help > "$TEST_TMPDIR/help_output.txt" 2>&1 || true
if grep -q "Commands" "$TEST_TMPDIR/help_output.txt"; then
    pass "--help prints usage"
else
    fail "--help prints usage"
fi

# Missing --frequency exits with error (no daemonization)
$BINARY --command="echo hi" > "$TEST_TMPDIR/no_freq.txt" 2>&1 || true
if grep -qi "frequency" "$TEST_TMPDIR/no_freq.txt"; then
    pass "missing --frequency reports error"
else
    fail "missing --frequency reports error"
fi

# Missing --command exits with error (no daemonization)
$BINARY --frequency=1000 > "$TEST_TMPDIR/no_cmd.txt" 2>&1 || true
if grep -qi "command" "$TEST_TMPDIR/no_cmd.txt"; then
    pass "missing --command reports error"
else
    fail "missing --command reports error"
fi

# ============================================================
echo "=== PID File ==="
# ============================================================

# PID file is created
PIDFILE="$TEST_TMPDIR/test_pid.pid"
$BINARY --frequency=500 --command="echo hi" --pid-file="$PIDFILE" > /dev/null 2>&1
sleep 0.5
assert_file_exists "$PIDFILE" "PID file is created"

# PID file contains valid integer
if [ -f "$PIDFILE" ]; then
    PID_VALUE=$(cat "$PIDFILE")
    if echo "$PID_VALUE" | grep -qE '^[0-9]+$'; then
        pass "PID file contains valid integer ($PID_VALUE)"
    else
        fail "PID file contains valid integer (got: $PID_VALUE)"
    fi
    kill "$PID_VALUE" 2>/dev/null || true
else
    fail "PID file contains valid integer (file missing)"
fi

# PID matches actual running process
$BINARY --frequency=500 --command="echo hi" --pid-file="$TEST_TMPDIR/pid_match.pid" > /dev/null 2>&1
sleep 0.5
PID_VALUE=$(cat "$TEST_TMPDIR/pid_match.pid")
if kill -0 "$PID_VALUE" 2>/dev/null; then
    pass "PID matches running process"
    kill "$PID_VALUE" 2>/dev/null || true
else
    fail "PID matches running process"
fi

# Invalid PID file path
$BINARY --frequency=500 --command="echo hi" --pid-file="/nonexistent/dir/test.pid" > /dev/null 2>&1
sleep 0.5
assert_file_not_exists "/nonexistent/dir/test.pid" "invalid PID path does not create file"


# ============================================================
echo "=== Synchronous Execution ==="
# ============================================================

# Command executes at roughly the configured frequency
OUTPUT="$TEST_TMPDIR/sync_freq.txt"
PIDFILE="$TEST_TMPDIR/sync_freq.pid"
$BINARY --frequency=200 --command="echo tick >> $OUTPUT" --pid-file="$PIDFILE" > /dev/null 2>&1
sleep 1.5
kill "$(cat "$PIDFILE")" 2>/dev/null || true
sleep 0.2
assert_line_count_ge "$OUTPUT" 5 "synchronous: ~correct execution frequency"

# Slow command blocks next execution
OUTPUT="$TEST_TMPDIR/slow_sync.txt"
PIDFILE="$TEST_TMPDIR/slow_sync.pid"
$BINARY --frequency=100 --command="sleep 1 && echo done >> $OUTPUT" --pid-file="$PIDFILE" > /dev/null 2>&1
sleep 2.5
kill "$(cat "$PIDFILE")" 2>/dev/null || true
sleep 0.2
assert_line_count_le "$OUTPUT" 3 "synchronous: slow command blocks next execution"


# ============================================================
echo "=== Asynchronous Execution ==="
# ============================================================

# Multiple commands run concurrently in async mode
OUTPUT="$TEST_TMPDIR/async_concurrent.txt"
PIDFILE="$TEST_TMPDIR/async_concurrent.pid"
$BINARY --frequency=200 --command="sleep 1 && echo done >> $OUTPUT" --synchronous=false --pid-file="$PIDFILE" > /dev/null 2>&1
sleep 2.5
kill "$(cat "$PIDFILE")" 2>/dev/null || true
sleep 1.5
assert_line_count_ge "$OUTPUT" 4 "asynchronous: concurrent execution"

# No zombie processes accumulate
PIDFILE="$TEST_TMPDIR/async_zombie.pid"
$BINARY --frequency=200 --command="echo hi > /dev/null" --synchronous=false --pid-file="$PIDFILE" > /dev/null 2>&1
sleep 2
DAEMON_PID=$(cat "$PIDFILE")
ZOMBIE_COUNT=0
CHILDREN=$(pgrep -P "$DAEMON_PID" 2>/dev/null || true)
if [ -n "$CHILDREN" ]; then
    ZOMBIE_COUNT=$(echo "$CHILDREN" | xargs -I{} ps -o stat= -p {} 2>/dev/null | grep -c "Z" || true)
fi
if [ "$ZOMBIE_COUNT" -le 2 ]; then
    pass "no zombie accumulation (found $ZOMBIE_COUNT)"
else
    fail "no zombie accumulation (found $ZOMBIE_COUNT)"
fi
kill "$DAEMON_PID" 2>/dev/null || true


# ============================================================
echo "=== Daemonization ==="
# ============================================================

# Process detaches (parent exits immediately)
PIDFILE="$TEST_TMPDIR/daemon_test.pid"
$BINARY --frequency=500 --command="echo hi" --pid-file="$PIDFILE" > /dev/null 2>&1
LAUNCH_EXIT=$?
if [ "$LAUNCH_EXIT" -eq 0 ]; then
    pass "daemon detaches from terminal (parent exits immediately)"
else
    fail "daemon detaches from terminal (exit code: $LAUNCH_EXIT)"
fi
sleep 0.5
kill "$(cat "$PIDFILE")" 2>/dev/null || true


# ============================================================
echo "=== Signal Handling ==="
# ============================================================

# SIGTERM cleanly stops the daemon
PIDFILE="$TEST_TMPDIR/sigterm.pid"
$BINARY --frequency=200 --command="echo hi" --pid-file="$PIDFILE" > /dev/null 2>&1
sleep 0.5
DAEMON_PID=$(cat "$PIDFILE")
kill "$DAEMON_PID" 2>/dev/null
sleep 0.5
if kill -0 "$DAEMON_PID" 2>/dev/null; then
    fail "SIGTERM stops daemon (still running)"
else
    pass "SIGTERM stops daemon"
fi


# ============================================================
echo "=== Edge Cases ==="
# ============================================================

# frequency=1 (1ms) doesn't crash
OUTPUT="$TEST_TMPDIR/fast_freq.txt"
PIDFILE="$TEST_TMPDIR/fast_freq.pid"
$BINARY --frequency=1 --command="echo tick >> $OUTPUT" --pid-file="$PIDFILE" > /dev/null 2>&1
sleep 1
kill "$(cat "$PIDFILE")" 2>/dev/null || true
sleep 0.2
assert_line_count_ge "$OUTPUT" 10 "frequency=1 doesn't crash"

# Nonexistent command doesn't crash daemon
PIDFILE="$TEST_TMPDIR/bad_cmd.pid"
$BINARY --frequency=500 --command="/nonexistent/script.sh" --pid-file="$PIDFILE" > /dev/null 2>&1
sleep 1.5
DAEMON_PID=$(cat "$PIDFILE")
if kill -0 "$DAEMON_PID" 2>/dev/null; then
    pass "nonexistent command doesn't crash daemon"
    kill "$DAEMON_PID" 2>/dev/null || true
else
    fail "nonexistent command doesn't crash daemon"
fi


# ============================================================
echo ""
echo "=== Results ==="
echo "  Passed: $PASS"
echo "  Failed: $FAIL"
echo "  Total:  $((PASS + FAIL))"

if [ "$FAIL" -gt 0 ]; then
    exit 1
fi
