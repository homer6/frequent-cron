# frequent-cron

A daemon that runs shell commands at millisecond intervals (sub-second cron). Production-stable for 15+ years. Runs on Linux, macOS, and Windows.

## Build

```bash
cmake .
make
make test
make install
```

Produces the `frequent-cron` executable in the project root.

## Dependencies

- Boost 1.37+ (components: `asio`, `program_options`)
- CMake 3.14+
- C++23

## Architecture

Modular C++ daemon split into a static library and thin main:

- **`include/config.h`** -- `Config` struct and `parse_args()` for CLI parsing (wraps `boost::program_options`)
- **`include/executor.h`** -- `Executor` class owning `io_context`, `steady_timer`, sync/async dispatch
- **`include/pid_file.h`** -- `write_pid_file()` utility
- **`include/daemonize.h`** -- `daemonize()` with `#ifdef _WIN32` guards (no-op on Windows)
- **`src/main.cc`** -- thin wiring: parse args, daemonize, write PID, run executor

**Execution flow:** `main()` -> `parse_args()` -> `Executor()` -> `daemonize()` -> `write_pid_file()` -> `executor.run()` (enters Boost ASIO event loop)

**Sync mode** (default): `system()` blocks, next timer fires after command completes.
**Async mode** (`--synchronous=false`): `fork()` + `system()` in child, timer keeps firing. On Windows, uses `cmd /c start /b`.

## Testing

```bash
make test    # runs all 25 tests via CTest
```

- **GTest unit tests**: `tests/test_config.cc`, `tests/test_pid_file.cc`, `tests/test_executor.cc`
- **Integration tests**: `tests/test_frequent_cron.sh` (Linux/macOS), `tests/test_frequent_cron.ps1` (Windows)
- **CI**: GitHub Actions on Linux, macOS, and Windows

## Key Files

- `include/` -- public headers (config, executor, pid_file, daemonize)
- `src/` -- implementation files and main
- `tests/` -- GTest unit tests and integration test scripts
- `docs/` -- platform-specific installation guides (macOS, Ubuntu, Windows)
- `init_script.tpl` -- template for Linux `/etc/init.d` service script

## Usage

```bash
./frequent-cron --frequency=1000 --command="/path/to/script.sh" --pid-file=/var/run/myservice.pid
./frequent-cron --frequency=500 --command="/path/to/script.sh" --synchronous=false
```
