# Logging

## Log Capture

When running services through the service manager (`install`/`start`), frequent-cron captures command output (stdout and stderr) and writes it to per-service log files.

Log files are stored at:
```
<data_dir>/logs/<service_name>.log
```

Each entry is timestamped:
```
[2026-04-05 12:00:01] Processing batch 42...
[2026-04-05 12:00:01] Batch 42 complete: 150 records
[2026-04-05 12:00:02] Processing batch 43...
```

## Viewing Logs

```bash
frequent-cron logs myservice
```

This outputs the contents of the service's log file to stdout.

## Log Rotation

Logs are automatically rotated when they exceed the maximum file size (default: 10MB).

Rotation scheme:
```
myservice.log      → myservice.log.1
myservice.log.1    → myservice.log.2
myservice.log.2    → myservice.log.3
...
myservice.log.4    → myservice.log.5 (oldest, deleted on next rotation)
```

Default settings:
- **Max file size**: 10 MB
- **Max rotated files**: 5

## Direct Run Mode

When using `frequent-cron run` directly (not through the service manager), command output goes to the system's stdout/stderr as usual. The log manager is only active for managed services.

## Log Cleanup

When a service is removed with `frequent-cron remove`, all of its log files (including rotated ones) are deleted automatically.
