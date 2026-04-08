# Platform Guides

## Linux (systemd)

When you run `frequent-cron install`, a systemd unit file is generated:

- **Root**: `/etc/systemd/system/frequent-cron-<name>.service`
- **User**: `~/.config/systemd/user/frequent-cron-<name>.service`

The unit file uses `Type=forking` with a PID file and `Restart=on-failure` with a 5-second delay.

You can also manage the service directly with systemctl:
```bash
systemctl status frequent-cron-myservice
journalctl -u frequent-cron-myservice
```

See [docs/ubuntu.md](https://github.com/homer6/frequent-cron/blob/main/docs/ubuntu.md) for full setup instructions.

## macOS (launchd)

When you run `frequent-cron install`, a launchd plist is generated:

- **User**: `~/Library/LaunchAgents/com.frequent-cron.<name>.plist`
- **Root**: `/Library/LaunchDaemons/com.frequent-cron.<name>.plist`

The plist uses `RunAtLoad` and `KeepAlive` for automatic startup and restart.

You can also manage the service directly with launchctl:
```bash
launchctl list | grep frequent-cron
```

See [docs/macos.md](https://github.com/homer6/frequent-cron/blob/main/docs/macos.md) for full setup instructions.

## Windows (Service Control Manager)

When you run `frequent-cron install` as Administrator, a Windows Service is registered via the Win32 `CreateService` API.

The service can be managed with:
```powershell
sc query frequent-cron-myservice
sc start frequent-cron-myservice
sc stop frequent-cron-myservice
```

Or through the Services MMC snap-in (`services.msc`).

See [docs/windows.md](https://github.com/homer6/frequent-cron/blob/main/docs/windows.md) for full setup instructions.

## FreeBSD (rc.d)

When you run `frequent-cron install`, an rc.d script is generated at `/usr/local/etc/rc.d/frequent_cron_<name>` and enabled via `sysrc`.

The script uses the standard `rc.subr` framework:
```bash
service frequent_cron_myservice status
service frequent_cron_myservice restart
```

See [docs/freebsd.md](https://github.com/homer6/frequent-cron/blob/main/docs/freebsd.md) for full setup instructions.

## Platform Detection

frequent-cron automatically detects the platform at compile time and selects the appropriate service manager:

| Platform | Compile Flag | Service Manager |
|---|---|---|
| Linux | `__linux__` | SystemdService |
| macOS | `__APPLE__` | LaunchdService |
| FreeBSD | `__FreeBSD__` | RcdService |
| Windows | `_WIN32` | ScmService |
