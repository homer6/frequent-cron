# CLI Reference

## Commands

### `run`

Run a command at the specified frequency. Daemonizes on POSIX systems.

```bash
frequent-cron run --frequency=<ms> --command=<cmd> [options]
```

| Option | Required | Description |
|---|---|---|
| `--frequency=<ms>` | Yes | Interval in milliseconds |
| `--command=<cmd>` | Yes | Shell command to execute |
| `--pid-file=<path>` | No | Write PID to this file |
| `--synchronous=<bool>` | No | `true` (default) or `false` |
| `--data-dir=<path>` | No | Override data directory |

**Legacy mode:** Omitting the `run` keyword works identically:
```bash
frequent-cron --frequency=1000 --command="echo hi" --pid-file=/tmp/fc.pid
```

### `install`

Register a named service in the local registry and create a platform-native service definition.

```bash
frequent-cron install <name> --frequency=<ms> --command=<cmd> [options]
```

| Option | Required | Description |
|---|---|---|
| `<name>` | Yes | Service name (positional) |
| `--frequency=<ms>` | Yes | Interval in milliseconds |
| `--command=<cmd>` | Yes | Shell command to execute |
| `--synchronous=<bool>` | No | `true` (default) or `false` |
| `--data-dir=<path>` | No | Override data directory |

Creates:
- SQLite registry entry
- systemd unit file (Linux), launchd plist (macOS), rc.d script (FreeBSD), or SCM entry (Windows)

### `remove`

Unregister a service. Stops it if running, removes platform service definition, cleans up PID and log files.

```bash
frequent-cron remove <name> [--data-dir=<path>]
```

### `start`

Start a registered service.

```bash
frequent-cron start <name> [--data-dir=<path>]
```

### `stop`

Stop a running service by sending SIGTERM (POSIX) or TerminateProcess (Windows).

```bash
frequent-cron stop <name> [--data-dir=<path>]
```

### `status`

Show status of a specific service or all services.

```bash
frequent-cron status [name] [--data-dir=<path>]
```

Example output:
```
NAME                STATUS    PID     FREQ(ms)    COMMAND
--------------------------------------------------------------------------------
my-poller           running   12345   1000        /opt/scripts/poll.sh
my-checker          stopped   -       500         /opt/scripts/check.sh
```

### `list`

List all registered services with their status.

```bash
frequent-cron list [--data-dir=<path>]
```

### `logs`

Display log output for a service.

```bash
frequent-cron logs <name> [--data-dir=<path>]
```

## Global Options

| Option | Description |
|---|---|
| `--data-dir=<path>` | Override the default data directory |
| `--help` | Show help for the current command |

## Exit Codes

| Code | Meaning |
|---|---|
| 0 | Success |
| 1 | Error (missing args, service not found, operation failed) |
