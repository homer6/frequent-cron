# frequent-cron Platform Reference

Detailed platform-specific service configurations, native commands, and advanced operational details.

## Linux (systemd)

### Generated Unit File
Location:
- Root: `/etc/systemd/system/frequent-cron-<name>.service`
- User: `~/.config/systemd/user/frequent-cron-<name>.service`

```ini
[Unit]
Description=frequent-cron: <name>
After=network.target

[Service]
Type=forking
ExecStart=/usr/local/bin/frequent-cron run --frequency=<ms> --command="<cmd>" --pid-file=<data_dir>/pids/<name>.pid --service-name=<name>
PIDFile=<data_dir>/pids/<name>.pid
Restart=on-failure
RestartSec=5

[Install]
WantedBy=multi-user.target
```

### Native Commands
```bash
systemctl start frequent-cron-<name>
systemctl stop frequent-cron-<name>
systemctl status frequent-cron-<name>
systemctl enable frequent-cron-<name>
systemctl disable frequent-cron-<name>
systemctl daemon-reload                  # After install/remove
journalctl -u frequent-cron-<name> -f    # systemd journal logs
```

### Data Directory
- Root: `/var/lib/frequent-cron/`
- User: `~/.local/share/frequent-cron/` (or `$XDG_DATA_HOME/frequent-cron/`)

---

## macOS (launchd)

### Generated Plist
Location:
- Root: `/Library/LaunchDaemons/com.frequent-cron.<name>.plist`
- User: `~/Library/LaunchAgents/com.frequent-cron.<name>.plist`

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>Label</key>
    <string>com.frequent-cron.<name></string>
    <key>ProgramArguments</key>
    <array>
        <string>/usr/local/bin/frequent-cron</string>
        <string>run</string>
        <string>--frequency=<ms></string>
        <string>--command=<cmd></string>
        <string>--pid-file=<data_dir>/pids/<name>.pid</string>
        <string>--service-name=<name></string>
    </array>
    <key>RunAtLoad</key>
    <true/>
    <key>KeepAlive</key>
    <true/>
</dict>
</plist>
```

### Native Commands
```bash
launchctl load ~/Library/LaunchAgents/com.frequent-cron.<name>.plist
launchctl unload ~/Library/LaunchAgents/com.frequent-cron.<name>.plist
launchctl list | grep frequent-cron
launchctl kickstart -k gui/$(id -u)/com.frequent-cron.<name>
```

### Data Directory
- User: `~/Library/Application Support/frequent-cron/`
- Root: `/var/lib/frequent-cron/`

### Known Issue: kqueue and fork
On macOS, the Boost ASIO `io_context` (backed by kqueue) must be constructed AFTER `daemonize()` calls `fork()`. kqueue FDs are not inherited across fork. If constructed before fork, the child gets a stale kqueue FD causing `kevent()` EBADF and 100% CPU busy-spin. This was fixed in v0.3.x.

---

## FreeBSD (rc.d)

### Generated Script
Location: `/usr/local/etc/rc.d/frequent_cron_<name>`

Uses standard `rc.subr` framework.

### Native Commands
```bash
sysrc frequent_cron_<name>_enable=YES    # Enable at boot
sysrc frequent_cron_<name>_enable=NO     # Disable
service frequent_cron_<name> start
service frequent_cron_<name> stop
service frequent_cron_<name> status
```

### Notes
- No built-in auto-restart on crash (use daemon(8) wrapper or cron health checks)
- Data directory: Root `/var/lib/frequent-cron/`, User `~/.local/share/frequent-cron/`

---

## Windows (Service Control Manager)

### Service Registration
- Service name: `frequent-cron-<name>`
- Display name: `frequent-cron: <name>`
- Start type: `SERVICE_AUTO_START`
- Service type: `SERVICE_WIN32_OWN_PROCESS`

### Native Commands
```powershell
sc query frequent-cron-<name>
sc start frequent-cron-<name>
sc stop frequent-cron-<name>
sc delete frequent-cron-<name>
```

### Auto-Restart Configuration
```powershell
sc failure frequent-cron-<name> reset=86400 actions=restart/5000/restart/10000/restart/30000
```
Restarts at 5s (1st failure), 10s (2nd), 30s (3rd+). Counter resets after 24h.

### GUI
`services.msc` -- Windows Services MMC snap-in.

### Data Directory
- User: `%LOCALAPPDATA%\frequent-cron\`
- System: `C:\ProgramData\frequent-cron\`

### Notes
- `run` mode runs in foreground (no daemonization)
- Requires Administrator for SCM operations
- Async mode uses `cmd /c start /b` for detached child processes

---

## Daemonization Details (POSIX)

1. `fork()` -- parent exits immediately
2. `setsid()` -- create new session (detach from terminal)
3. `chdir("/")` -- prevent holding mount points
4. Redirect FDs 0-2 to `/dev/null` (not closed, preserving the invariant for child processes via `system()`)

---

## Timer Engine (Boost ASIO)

- Uses `boost::asio::steady_timer` (monotonic clock, immune to NTP adjustments)
- Precision: OS-dependent, typically millisecond-level
- Drift-free: timer rescheduled after each execution, not accumulated
- Jitter calculation: `delay = frequency_ms + offset` where offset comes from uniform or normal distribution clamped to `[-jitter, +jitter]`
- Minimum enforced delay: 1ms

---

## Signal Handling

- **SIGTERM:** Stops `io_context`, clean exit from event loop
- Async mode: calls `waitpid(-1, &status, WNOHANG)` twice per tick to reap zombie children
- Stop command: sends SIGTERM, polls for exit up to 2s at 200ms intervals

---

## Database Details

```sql
CREATE TABLE services (
    name TEXT PRIMARY KEY,
    command TEXT NOT NULL,
    frequency_ms INTEGER NOT NULL,
    synchronous INTEGER NOT NULL DEFAULT 1,
    jitter_ms INTEGER NOT NULL DEFAULT 0,
    jitter_distribution TEXT NOT NULL DEFAULT 'uniform',
    fire_probability REAL NOT NULL DEFAULT 1.0,
    created_at TEXT NOT NULL DEFAULT (datetime('now')),
    updated_at TEXT NOT NULL DEFAULT (datetime('now'))
);

CREATE TABLE service_state (
    name TEXT PRIMARY KEY REFERENCES services(name) ON DELETE CASCADE,
    status TEXT NOT NULL DEFAULT 'stopped',
    pid INTEGER,
    last_started_at TEXT,
    last_stopped_at TEXT,
    execution_count INTEGER NOT NULL DEFAULT 0
);
```

- WAL mode (`PRAGMA journal_mode=WAL`)
- Foreign keys enforced (`PRAGMA foreign_keys=ON`)
- Auto-migration: adds jitter/fire_probability columns if missing (v0.3.0+ compat)
