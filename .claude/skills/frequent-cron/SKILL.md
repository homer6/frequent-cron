---
name: frequent-cron
description: Operate and manage frequent-cron, a daemon that runs shell commands at millisecond intervals (sub-second cron). Use when installing, starting, stopping, checking status, viewing logs, removing, or troubleshooting frequent-cron services. Triggers on mentions of frequent-cron, sub-second scheduling, millisecond cron, high-frequency task execution, or service management with frequent-cron.
---

# frequent-cron Operations

frequent-cron is a daemon and service manager that runs shell commands at millisecond-precision intervals. Production-stable for 15+ years. Runs on Linux, macOS, FreeBSD, and Windows.

## Quick Start

```bash
# Install and run a service
frequent-cron install myservice --frequency=1000 --command="/path/to/script.sh"
frequent-cron start myservice
frequent-cron status
frequent-cron logs myservice
frequent-cron stop myservice
frequent-cron remove myservice

# Direct execution (one-off, no service registration)
frequent-cron run --frequency=500 --command="echo tick" --pid-file=/tmp/fc.pid
```

## Command Reference

### run -- Direct Execution
```bash
frequent-cron run --frequency=<ms> --command=<cmd> [options]
```
Daemonizes on POSIX, runs foreground on Windows. Returns immediately after fork.

**Required:**
- `--frequency=<ms>` -- Interval in milliseconds (min 1ms, practical min ~100ms)
- `--command=<cmd>` -- Shell command (runs via `/bin/sh -c` on POSIX, `cmd /c` on Windows)

**Optional:**
- `--pid-file=<path>` -- Write daemon PID for lifecycle management
- `--synchronous=<bool>` -- Wait for command to finish before next tick (default: true)
- `--data-dir=<path>` -- Override platform data directory
- `--jitter=<ms>` -- Max timing variance around frequency
- `--jitter-distribution=<mode>` -- "uniform" (default) or "normal"
- `--fire-probability=<0.0-1.0>` -- Chance of firing each tick (default: 1.0)
- `--service-name=<name>` -- Name for log capture (set automatically by `start`)

**Legacy mode** (backward compat): omit `run` keyword:
```bash
frequent-cron --frequency=500 --command="echo hi"
```

### install -- Register a Named Service
```bash
frequent-cron install <name> --frequency=<ms> --command=<cmd> [options]
```
Creates database entry + platform-native service definition (systemd unit, launchd plist, rc.d script, or Windows SCM entry). All `run` options apply.

### start / stop
```bash
frequent-cron start <name> [--data-dir=<path>]
frequent-cron stop <name> [--data-dir=<path>]
```
Start launches `frequent-cron run` in background with stored config. Stop sends SIGTERM (POSIX) or TerminateProcess (Windows), waits up to 2s.

### status
```bash
frequent-cron status [name] [--data-dir=<path>]
```
Without name: table of all services (NAME, STATUS, PID, FREQ, COMMAND). With name: detailed view including jitter, fire probability, timestamps. Live-verifies PIDs and auto-corrects stale entries.

### list
```bash
frequent-cron list [--data-dir=<path>]
```
Table of all registered services: NAME, STATUS, FREQ(ms), COMMAND.

### logs
```bash
frequent-cron logs <name> [--data-dir=<path>]
```
Last 100 lines of `<data_dir>/logs/<name>.log`. Each line timestamped `[YYYY-MM-DD HH:MM:SS]`. For live tailing, use the log file path directly with `tail -f`.

### remove
```bash
frequent-cron remove <name> [--data-dir=<path>]
```
Stops service if running, removes platform service definition, deletes database record, PID file, and all log files (including rotated copies).

## Execution Modes

**Synchronous (default):** Timer waits for command to finish before rescheduling. If command takes 3s at 500ms frequency, effective interval is 3s. Use for sequential work (DB queries, health checks).

**Asynchronous (`--synchronous=false`):** Timer fires independently; commands can overlap. Risk of resource exhaustion if commands pile up. Use for non-blocking/distributed work. On POSIX, reaps zombies via `waitpid(-1, WNOHANG)`.

## Jitter and Probabilistic Firing

**Jitter** prevents thundering herd when many instances fire at the same frequency:
```bash
frequent-cron install db-check --frequency=5000 --command="curl localhost/health" \
  --jitter=1000 --jitter-distribution=uniform
```
Delay = frequency + random offset in [-jitter, +jitter]. Minimum delay enforced at 1ms.

**Fire probability** for chaos engineering or load testing:
```bash
frequent-cron install chaos --frequency=100 --command="/opt/chaos.sh" \
  --synchronous=false --fire-probability=0.8
```

## Data Directory

| Platform | User | Root |
|---|---|---|
| Linux/FreeBSD | `~/.local/share/frequent-cron/` | `/var/lib/frequent-cron/` |
| macOS | `~/Library/Application Support/frequent-cron/` | `/var/lib/frequent-cron/` |
| Windows | `%LOCALAPPDATA%\frequent-cron\` | `C:\ProgramData\frequent-cron\` |

Override with `--data-dir=<path>` on every command. `XDG_DATA_HOME` overrides base on Linux/FreeBSD.

**Contents:**
```
<data_dir>/
  frequent-cron.db              # SQLite registry (WAL mode)
  logs/<name>.log               # Current log
  logs/<name>.log.{1..5}        # Rotated logs (10MB each, 5 max)
  pids/<name>.pid               # PID file
```

**Log rotation:** 10 MB max per file, 5 rotated copies kept (~60 MB total per service).

## Database

SQLite with WAL mode and foreign keys. Two tables:

- **services**: name (PK), command, frequency_ms, synchronous, jitter_ms, jitter_distribution, fire_probability, created_at, updated_at
- **service_state**: name (PK, FK), status, pid, last_started_at, last_stopped_at, execution_count

Direct inspection:
```bash
sqlite3 ~/.local/share/frequent-cron/frequent-cron.db "SELECT * FROM services;"
sqlite3 ~/.local/share/frequent-cron/frequent-cron.db "SELECT * FROM service_state;"
```

## Boot-Start Behavior

All platforms auto-start installed services at boot:
- **Linux:** systemd `WantedBy=multi-user.target` + `Restart=on-failure` (5s delay)
- **macOS:** launchd `RunAtLoad` + `KeepAlive`
- **FreeBSD:** rc.d with `sysrc enable=YES`
- **Windows:** SCM `SERVICE_AUTO_START`

## Platform-Specific Operations

For detailed platform service files, paths, and native commands, see [reference.md](reference.md).

## Troubleshooting

### Service won't start
1. Verify it exists: `frequent-cron list`
2. Test command directly: `frequent-cron run --frequency=1000 --command="your command"`
3. Check logs: `frequent-cron logs <name>`
4. Check platform service: `systemctl status frequent-cron-<name>` (Linux) / `launchctl list | grep frequent-cron` (macOS)
5. Check permissions on binary, paths, and data directory

### Stale PID
`frequent-cron stop <name>` detects and handles stale PIDs automatically. Then `frequent-cron start <name>`.

### High CPU (async mode)
Commands piling up without completing. Reduce frequency, add concurrency guards in script, or switch to synchronous mode.

### Database issues
```bash
# Backup
cp <data_dir>/frequent-cron.db <data_dir>/frequent-cron.db.bak
# Remove and re-install problematic service
frequent-cron remove <name>
frequent-cron install <name> --frequency=... --command="..."
```

### Permission errors
- Root services: `/etc/systemd/system/` (Linux), `/Library/LaunchDaemons/` (macOS)
- User services: `~/.config/systemd/user/` (Linux), `~/Library/LaunchAgents/` (macOS)
- Use `--data-dir` pointing to a writable path if needed

## Building from Source

```bash
cmake .
make
make test       # 95 tests (70 GTest unit + integration)
sudo make install
```

Requires: CMake 3.14+, C++23 compiler, Boost 1.37+ (asio, program_options), SQLite3.

## Common Patterns

```bash
# Health check poller
frequent-cron install health --frequency=500 --command="curl -sf http://localhost/health || alert.sh"

# Jittered multi-instance (prevent thundering herd)
frequent-cron install poller --frequency=5000 --command="/opt/poll.sh" --jitter=2000

# Chaos engineering
frequent-cron install chaos --frequency=100 --command="/opt/inject-fault.sh" \
  --synchronous=false --fire-probability=0.3

# Isolated test environment
frequent-cron install test-svc --frequency=1000 --command="echo test" --data-dir=/tmp/fc-test
frequent-cron start test-svc --data-dir=/tmp/fc-test
```
