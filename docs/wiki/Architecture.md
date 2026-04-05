# Architecture

## Overview

frequent-cron is a modular C++23 application built as a static library (`frequent-cron_lib`) with a thin `main.cc` entry point. This separation enables comprehensive unit testing.

```
main.cc
  ├── parse_args()          → Config struct
  ├── cmd_run()             → Executor + daemonize + run
  └── ServiceRegistry       → install/remove/start/stop/status/list/logs
        ├── Database         → SQLite CRUD
        ├── PlatformService  → systemd/launchd/SCM
        └── LogManager       → per-service log files
```

## Module Map

| Module | Header | Implementation | Purpose |
|---|---|---|---|
| Config | `config.h` | `config.cc` | CLI parsing, subcommand dispatch |
| Executor | `executor.h` | `executor.cc` | Boost ASIO timer loop, sync/async execution |
| Process | `process.h` | `process.cc` | Command execution with output capture |
| Database | `database.h` | `database.cc` | SQLite wrapper, ServiceRecord/ServiceState CRUD |
| Service Registry | `service_registry.h` | `service_registry.cc` | High-level subcommand handlers |
| Platform Service | `platform_service.h` | `platform_service.cc` | Abstract interface + systemd/launchd/SCM |
| Log Manager | `log_manager.h` | `log_manager.cc` | Per-service log files with rotation |
| Data Dir | `data_dir.h` | `data_dir.cc` | Platform-specific path resolution |
| Daemonize | `daemonize.h` | `daemonize.cc` | fork()/setsid() on POSIX, no-op on Windows |
| PID File | `pid_file.h` | `pid_file.cc` | Write process ID to file |

## Execution Flow

### Direct Run (`frequent-cron run`)

```
main() → parse_args() → Config{subcommand=RUN}
  → Executor(command, frequency, synchronous)
  → daemonize()          // fork + setsid + close fds
  → write_pid_file()
  → executor.run()       // enters Boost ASIO event loop
    → register_callback()
    → io_context->run()  // blocks
      → timer fires every frequency_ms
        → timer_callback()
          → execute_sync() or execute_async()
          → register_callback()  // reschedule
```

### Service Install (`frequent-cron install`)

```
main() → parse_args() → Config{subcommand=INSTALL, service_name="foo"}
  → ServiceRegistry(data_dir)
    → Database::insert_service()     // SQLite insert
    → PlatformService::install()     // write systemd unit / launchd plist / SCM entry
```

### Service Start (`frequent-cron start`)

```
main() → parse_args() → Config{subcommand=START, service_name="foo"}
  → ServiceRegistry::cmd_start()
    → Database::get_service()        // look up config
    → launch "frequent-cron run ..." as background process
    → wait for PID file
    → Database::update_state()       // record PID + running status
```

## Timer Engine

The core timer uses Boost ASIO's `steady_timer`:

1. `register_callback()` sets the timer to expire after `frequency_ms`
2. When the timer fires, `timer_callback()` runs the command
3. After execution completes (sync) or is launched (async), `register_callback()` reschedules

**Synchronous mode**: `system()` or `run_process()` blocks in `timer_callback()`, so the timer naturally delays.

**Asynchronous mode (POSIX)**: `register_callback()` is called before `fork()`, so the timer keeps firing independently of child processes. `waitpid()` with `WNOHANG` reaps zombies.

**Asynchronous mode (Windows)**: Uses `cmd /c start /b` to launch a detached process.

## Platform Abstraction

`PlatformService` is an abstract interface with a `create()` factory:

```cpp
class PlatformService {
    virtual bool install(name, record, binary_path, data_dir) = 0;
    virtual bool uninstall(name) = 0;
    virtual bool start(name) = 0;
    virtual bool stop(name) = 0;
    static std::unique_ptr<PlatformService> create();
};
```

Compile-time `#ifdef` selects:
- `SystemdService` on `__linux__`
- `LaunchdService` on `__APPLE__`
- `ScmService` on `_WIN32`

## Build System

CMake with FetchContent for GTest:

```
CMakeLists.txt          # root: project, C++23, GTest fetch, enable_testing
src/CMakeLists.txt      # library + executable, find Boost + SQLite3
tests/CMakeLists.txt    # test executables, gtest_discover_tests
```

The library is linked by both the main executable and all test executables, enabling unit testing of internal modules.
