# Service Lifecycle

## Boot Behavior

When you install a service with `frequent-cron install`, a platform-native service definition is created that starts the service automatically at boot.

| Platform | Service Manager | Auto-start at Boot | Auto-restart on Crash |
|---|---|---|---|
| Linux | systemd | Yes | Yes (on-failure, 5s delay) |
| macOS | launchd | Yes | Yes (KeepAlive) |
| FreeBSD | rc.d | Yes | No |
| Windows | SCM | Yes | No |

## What Happens on Each Platform

### Linux (systemd)

`frequent-cron install myservice` creates a systemd unit file:

- **Root**: `/etc/systemd/system/frequent-cron-myservice.service`
- **User**: `~/.config/systemd/user/frequent-cron-myservice.service`

The unit uses:
- `Type=forking` -- frequent-cron daemonizes after startup
- `Restart=on-failure` -- systemd restarts the service if it exits with a non-zero code
- `RestartSec=5` -- waits 5 seconds before restarting
- `WantedBy=multi-user.target` -- starts at boot (after network)

You can also manage it directly:
```bash
systemctl status frequent-cron-myservice
systemctl restart frequent-cron-myservice
journalctl -u frequent-cron-myservice
```

### macOS (launchd)

`frequent-cron install myservice` creates a launchd plist:

- **User**: `~/Library/LaunchAgents/com.frequent-cron.myservice.plist`
- **Root**: `/Library/LaunchDaemons/com.frequent-cron.myservice.plist`

The plist uses:
- `RunAtLoad=true` -- starts when the plist is loaded (at login for LaunchAgents, at boot for LaunchDaemons)
- `KeepAlive=true` -- launchd restarts the process if it exits for any reason

You can also manage it directly:
```bash
launchctl list | grep frequent-cron
launchctl kickstart -k gui/$(id -u)/com.frequent-cron.myservice
```

### FreeBSD (rc.d)

`frequent-cron install myservice` creates an rc.d script:

- Path: `/usr/local/etc/rc.d/frequent_cron_myservice`
- Enabled via: `sysrc frequent_cron_myservice_enable=YES`

The script uses the standard `rc.subr` framework:
- Starts at boot when enabled in `rc.conf`
- Managed via the `service` command

```bash
service frequent_cron_myservice status
service frequent_cron_myservice restart
```

FreeBSD's rc.d does not auto-restart on crash by default. For auto-restart, consider using `daemon(8)` with the `-r` flag or a cron-based health check.

### Windows (SCM)

`frequent-cron install myservice` registers a Windows Service via the Service Control Manager:

- Service name: `frequent-cron-myservice`
- Start type: `SERVICE_AUTO_START` -- starts at boot
- Requires Administrator privileges to install

Managed via `sc.exe` or `services.msc`:
```powershell
sc query frequent-cron-myservice
sc start frequent-cron-myservice
sc stop frequent-cron-myservice
```

For auto-restart on failure, configure recovery options:
```powershell
sc failure frequent-cron-myservice reset=86400 actions=restart/5000/restart/10000/restart/30000
```
This restarts after 5s on first failure, 10s on second, 30s on subsequent, resetting the counter after 24 hours.

## Service States

frequent-cron tracks service state in its SQLite database:

| State | Meaning |
|---|---|
| `stopped` | Service is not running |
| `running` | Service is running (PID is tracked) |
| `failed` | Service exited unexpectedly |

The `status` command verifies that the tracked PID is actually alive. If a process died without updating the database, the status is auto-corrected to `stopped`.

## Lifecycle Commands

```bash
# Register and create platform service definition
frequent-cron install myservice --frequency=1000 --command="/path/to/script.sh"

# Start (also updates database state)
frequent-cron start myservice

# Check if running (verifies PID is alive)
frequent-cron status myservice

# Stop (sends SIGTERM on POSIX, TerminateProcess on Windows)
frequent-cron stop myservice

# Unregister and clean up everything
frequent-cron remove myservice
```

## What `remove` Cleans Up

When you run `frequent-cron remove myservice`, it:

1. Stops the service if running
2. Removes the platform service definition (systemd unit / launchd plist / rc.d script / SCM entry)
3. Deletes the database record and state
4. Removes the PID file
5. Removes all log files (including rotated ones)
