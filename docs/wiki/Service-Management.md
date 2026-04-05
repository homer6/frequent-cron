# Service Management

frequent-cron can manage multiple named services through a SQLite registry. Each service has a name, command, frequency, and execution mode.

## Service Lifecycle

```
install → start → [running] → stop → remove
```

### Register a Service

```bash
frequent-cron install api-poller --frequency=500 --command="/opt/scripts/poll-api.sh"
frequent-cron install db-checker --frequency=5000 --command="/opt/scripts/check-db.sh" --synchronous=false
```

This does two things:
1. Creates a record in the SQLite database
2. Generates a platform-native service definition (systemd unit, launchd plist, or Windows Service)

### Start and Stop

```bash
frequent-cron start api-poller
frequent-cron stop api-poller
```

`start` launches a `frequent-cron run` process in the background and tracks its PID. `stop` sends SIGTERM (or TerminateProcess on Windows) and waits for the process to exit.

### Monitor

```bash
# All services
frequent-cron status

# Specific service
frequent-cron status api-poller
```

Status output verifies that the tracked PID is actually alive. If a process died unexpectedly, the status is updated to `stopped` automatically.

### View Logs

```bash
frequent-cron logs api-poller
```

Shows the captured output from command executions. See [Logging](Logging) for details on log capture and rotation.

### Remove

```bash
frequent-cron remove api-poller
```

This stops the service (if running), removes the platform service definition, deletes the database record, and cleans up PID and log files.

## Data Directory

Services are tracked in a SQLite database stored in the platform data directory:

| Platform | Default Path |
|---|---|
| Linux (user) | `~/.local/share/frequent-cron/` |
| Linux (root) | `/var/lib/frequent-cron/` |
| macOS | `~/Library/Application Support/frequent-cron/` |
| Windows | `%LOCALAPPDATA%\frequent-cron\` |

Override with `--data-dir`:

```bash
frequent-cron install myservice --frequency=1000 --command="echo hi" --data-dir=/opt/frequent-cron
frequent-cron start myservice --data-dir=/opt/frequent-cron
```

Directory structure:
```
<data_dir>/
  frequent-cron.db     # SQLite database
  logs/                # Per-service log files
    myservice.log
    myservice.log.1    # Rotated logs
  pids/                # PID files
    myservice.pid
```

## Database Schema

```sql
-- Service definitions
CREATE TABLE services (
    name TEXT PRIMARY KEY,
    command TEXT NOT NULL,
    frequency_ms INTEGER NOT NULL,
    synchronous INTEGER NOT NULL DEFAULT 1,
    created_at TEXT DEFAULT (datetime('now')),
    updated_at TEXT DEFAULT (datetime('now'))
);

-- Runtime state
CREATE TABLE service_state (
    name TEXT PRIMARY KEY REFERENCES services(name) ON DELETE CASCADE,
    status TEXT NOT NULL DEFAULT 'stopped',
    pid INTEGER,
    last_started_at TEXT,
    last_stopped_at TEXT,
    execution_count INTEGER DEFAULT 0
);
```
