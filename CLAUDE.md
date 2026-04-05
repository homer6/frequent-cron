# frequent-cron

A daemon and service manager that runs shell commands at millisecond intervals (sub-second cron). Production-stable for 15+ years. Runs on Linux, macOS, and Windows.

## Build

```bash
cmake .
make
make test
sudo make install
```

## Dependencies

- Boost 1.37+ (components: `asio`, `program_options`)
- SQLite3
- CMake 3.14+
- C++23

## Architecture

Modular C++ with static library (`frequent-cron_lib`) and thin `main.cc`:

### Core Modules (`include/` + `src/`)
- **`config.h/cc`** -- Subcommand enum + `parse_args()`. Supports: `run`, `install`, `remove`, `start`, `stop`, `status`, `list`, `logs`, and legacy mode (backward compat)
- **`executor.h/cc`** -- `Executor` class: Boost ASIO `io_context` + `steady_timer` event loop. Optional `OutputCallback` for log capture (uses `run_process()` when set, `system()` when not)
- **`process.h/cc`** -- `run_process()`: cross-platform command execution with stdout/stderr capture via `popen`
- **`database.h/cc`** -- SQLite wrapper: `ServiceRecord` + `ServiceState` CRUD with WAL mode
- **`service_registry.h/cc`** -- High-level subcommand handlers: install/remove/start/stop/status/list/logs
- **`platform_service.h/cc`** -- Abstract `PlatformService` interface with factory. Implementations: `SystemdService` (Linux), `LaunchdService` (macOS), `ScmService` (Windows)
- **`log_manager.h/cc`** -- Per-service log files with timestamps and size-based rotation
- **`data_dir.h/cc`** -- Platform-specific default data directory resolution
- **`daemonize.h/cc`** -- `fork()`+`setsid()` on POSIX, no-op on Windows
- **`pid_file.h/cc`** -- Write PID to file

### Data Directory
- Linux: `~/.local/share/frequent-cron` (user) or `/var/lib/frequent-cron` (root)
- macOS: `~/Library/Application Support/frequent-cron`
- Windows: `%LOCALAPPDATA%\frequent-cron`

Contents: `frequent-cron.db`, `logs/<name>.log`, `pids/<name>.pid`

## Testing

```bash
make test    # runs all tests via CTest
```

- **70 GTest unit tests**: config (32), pid_file (3), executor (7), data_dir (4), database (12), process (5), log_manager (7)
- **Integration tests**: `tests/test_frequent_cron.sh` (Linux/macOS), `tests/test_frequent_cron.ps1` (Windows)
- **CI**: GitHub Actions on Linux, macOS, Windows (with vcpkg binary caching)

## Usage

```bash
# Direct execution (legacy mode)
frequent-cron --frequency=1000 --command="/path/to/script.sh" --pid-file=/var/run/fc.pid

# Service management
frequent-cron install myservice --frequency=1000 --command="/path/to/script.sh"
frequent-cron start myservice
frequent-cron status
frequent-cron logs myservice
frequent-cron stop myservice
frequent-cron remove myservice
```
