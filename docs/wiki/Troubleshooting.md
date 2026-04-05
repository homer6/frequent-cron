# Troubleshooting

## Build Issues

### `CMake Error: Compatibility with CMake < 3.5 has been removed`

Your CMake is too new for the old minimum version. frequent-cron v0.3.0+ requires CMake 3.14+. Update your CMake or use the version from your package manager.

### `Could NOT find SQLite3`

Install the SQLite3 development package:
- Ubuntu: `sudo apt-get install libsqlite3-dev`
- macOS: `brew install sqlite`
- Windows: `vcpkg install sqlite3:x64-windows`

### `Could NOT find Boost`

Install the Boost development packages:
- Ubuntu: `sudo apt-get install libboost-system-dev libboost-program-options-dev`
- macOS: `brew install boost`
- Windows: `vcpkg install boost-asio:x64-windows boost-program-options:x64-windows`

### `error C2589: illegal token` (Windows)

This is a conflict between Windows `<windows.h>` macros and C++ code. frequent-cron defines `NOMINMAX` and `_WIN32_WINNT=0x0601` to prevent this. If you see this error, ensure you're building with the project's CMakeLists.txt.

## Runtime Issues

### `frequent-cron: command not found` after `make install`

The binary installs to `/usr/local/bin` by default. Ensure this is in your `PATH`:
```bash
export PATH="/usr/local/bin:$PATH"
```

### Service shows "running" but the command isn't executing

Check the PID is actually alive:
```bash
frequent-cron status myservice
ps -p <pid>
```

If the PID doesn't exist, the status will auto-correct to "stopped" on the next `status` check.

### `Failed to open Service Control Manager` (Windows)

SCM operations require Administrator privileges. Run the command prompt as Administrator:
```powershell
# Right-click Command Prompt → Run as Administrator
frequent-cron install myservice --frequency=1000 --command="echo hi"
```

### Service won't start after install

1. Check that the binary path is correct: `frequent-cron status myservice`
2. Try running the command manually: `frequent-cron run --frequency=1000 --command="your command"`
3. Check logs: `frequent-cron logs myservice`

### Stale PID file prevents start

If a previous instance crashed without cleaning up:
```bash
frequent-cron stop myservice    # will detect stale PID and clean up
frequent-cron start myservice
```

### Permission denied on PID file

On Linux, PID files in `/var/run/` require root. Either:
- Run as root: `sudo frequent-cron start myservice`
- Use a user-writable path: `--pid-file=/tmp/myservice.pid`
- Use the service manager (it handles paths automatically)

## Logging Issues

### No logs appearing

Logs are only captured for managed services (`install`/`start`). Direct `run` mode sends output to the terminal's stdout/stderr.

### Log files growing too large

Log rotation happens automatically at 10MB by default. If files are growing fast, consider:
- Reducing the verbosity of your command's output
- Using a shorter frequency only when needed
