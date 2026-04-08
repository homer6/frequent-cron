# Migrating from v0.1.x to v0.2.x

This guide covers upgrading from v0.1.x (the original Linux-only daemon) to v0.2.x (cross-platform, modern C++).

## What's New

- **macOS and Windows support**
- **Asynchronous execution mode** (`--synchronous=false`)
- **Modern C++23** and modern Boost ASIO (`io_context`/`steady_timer`)
- **GTest unit tests** and integration test suite
- **GitHub Actions CI** on Linux, macOS, and Windows
- **`make install` and `make test`** support

## Breaking Changes

### Build Requirements

| | v0.1.x | v0.2.x |
|---|---|---|
| CMake | 2.8+ | 3.5+ |
| C++ standard | C++03 | C++23 |
| Platforms | Linux only | Linux, macOS, Windows |

If your build system pins CMake or compiler versions, update them before upgrading.

### Source Layout

v0.1.x was a single file (`src/frequent.cc`). v0.2.x splits into modules:

```
include/
  config.h
  executor.h
  daemonize.h
  pid_file.h
src/
  main.cc
  config.cc
  executor.cc
  daemonize.cc
  pid_file.cc
```

If you were patching `frequent.cc` directly, you'll need to re-apply changes to the appropriate module.

### Daemonization

v0.1.x used the BSD `daemon(0, 0)` call. v0.2.x uses a manual `fork()`/`setsid()` implementation for portability (macOS deprecated `daemon()`).

**Known issue (fixed in v0.2.1):** The v0.2.0 daemonize implementation closed FDs 0-2 instead of redirecting them to `/dev/null`. This caused commands containing backgrounded subprocesses (`command &`) to silently fail on some platforms. **Upgrade to v0.2.1+ to get the fix.** See [#17](https://github.com/homer6/frequent-cron/issues/17).

## No Behavioral Changes

- The CLI is fully backward-compatible. `frequent-cron --frequency=1000 --command="echo hi" --pid-file=/tmp/fc.pid` works identically.
- The timer/execution engine behavior is unchanged.
- The `init_script.tpl` service template is still at the project root.

## Migration Steps

### 1. Update build dependencies

```bash
# Ubuntu/Debian
sudo apt-get install cmake build-essential libboost-system-dev libboost-program-options-dev

# macOS
brew install boost cmake
```

Ensure your compiler supports C++23 (GCC 13+, Clang 16+, MSVC 2022+).

### 2. Build

```bash
git clone https://github.com/homer6/frequent-cron.git
cd frequent-cron
git checkout 0.2
cmake .
make
sudo make install
```

### 3. Verify

```bash
make test
frequent-cron --frequency=1000 --command="echo hello" --pid-file=/tmp/fc_test.pid
sleep 2
kill $(cat /tmp/fc_test.pid)
```

### 4. Update init scripts (optional)

Your existing `init_script.tpl`-based scripts will continue to work without changes. The CLI interface is identical.

## New Capability: Async Mode

v0.2.x adds `--synchronous=false` for non-blocking execution. In this mode, the timer fires regardless of whether the previous command has finished:

```bash
frequent-cron --frequency=500 --command="/opt/scripts/fast-poll.sh" --synchronous=false
```

Use with caution -- overlapping commands can exhaust system resources.
