# Configuration

frequent-cron is configured entirely through command-line arguments. There are no configuration files.

## Command Options

### Frequency

The interval between command executions, in milliseconds.

```bash
--frequency=1000    # 1 second
--frequency=500     # 500 milliseconds
--frequency=100     # 100 milliseconds (10 times/second)
--frequency=1       # 1 millisecond (use with caution)
```

The actual interval depends on execution mode:
- **Synchronous**: frequency is the delay *after* the previous command finishes
- **Asynchronous**: frequency is the delay between command *starts*, regardless of completion

### Command

The shell command to execute. Passed to the system shell (`/bin/sh -c` on POSIX, `cmd /c` on Windows).

```bash
--command="echo hello"
--command="/opt/scripts/poll.sh"
--command="curl -s https://api.example.com/health"
```

### Synchronous Mode

Controls whether commands block subsequent executions.

```bash
--synchronous=true    # Default. Next execution waits for current to finish.
--synchronous=false   # Commands can overlap. Use with caution.
--synchronous         # Implicit true (no value needed)
```

Accepted true values: `true`, `True`, `TRUE`, `1`
Everything else is treated as false.

### PID File

Write the daemon's process ID to a file for lifecycle management.

```bash
--pid-file=/var/run/frequent-cron.pid
```

### Data Directory

Override the default platform data directory.

```bash
--data-dir=/opt/frequent-cron/data
```

Affects where the SQLite database, logs, and PID files are stored for service management commands.

## Environment Variables

| Variable | Platform | Effect |
|---|---|---|
| `XDG_DATA_HOME` | Linux | Overrides default data directory base (default: `~/.local/share`) |
| `HOME` | POSIX | Used to resolve data directory when XDG_DATA_HOME is not set |
| `LOCALAPPDATA` | Windows | Used for data directory (default: `C:\Users\<user>\AppData\Local`) |
