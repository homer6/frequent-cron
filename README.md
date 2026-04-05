# frequent-cron

[![CI](https://github.com/homer6/frequent-cron/actions/workflows/ci.yml/badge.svg)](https://github.com/homer6/frequent-cron/actions/workflows/ci.yml)

A lightweight daemon and service manager that executes shell commands at sub-second intervals. Standard cron only supports minute-level granularity -- frequent-cron supports millisecond precision. Runs on Linux, macOS, and Windows.

By default, command execution is **blocking** (synchronous): if a command takes longer than the configured interval, the next execution waits until the previous one completes. For example, a 500ms interval with a command that takes 3 minutes will effectively run once every 3 minutes.

You can set `--synchronous=false` to run commands asynchronously. Warning: running asynchronously can cause unbounded resource growth if the script never exits or is called too frequently (e.g. consuming too many database connections or too much memory). Synchronous (the default) is the safer option.

Licensed under the MIT License.


## Dependencies

- [Boost](https://www.boost.org/) 1.37+ (components: `asio`, `program_options`)
- [SQLite3](https://www.sqlite.org/)
- [CMake](https://cmake.org/) 3.14+
- C++23 compiler

See platform-specific install instructions in [docs/](docs/).


## Installation

```bash
git clone https://github.com/homer6/frequent-cron.git
cd frequent-cron
cmake .
make
make test
sudo make install
```


## Usage

### Running a Command Directly

```bash
frequent-cron run --frequency=1000 --command="/path/to/your/script.sh" --pid-file=/var/run/frequent-cron.pid
```

Or using legacy mode (backward compatible):

```bash
frequent-cron --frequency=1000 --command="/path/to/your/script.sh" --pid-file=/var/run/frequent-cron.pid
```

To stop:

```bash
kill $(cat /var/run/frequent-cron.pid)
```

### Managing Named Services

Register, start, monitor, and stop services by name:

```bash
# Install a service
frequent-cron install myservice --frequency=1000 --command="/path/to/script.sh"

# Start it
frequent-cron start myservice

# Check status
frequent-cron status           # all services
frequent-cron status myservice  # specific service

# View logs
frequent-cron logs myservice

# List all services
frequent-cron list

# Stop and remove
frequent-cron stop myservice
frequent-cron remove myservice
```

The `install` command also creates platform-native service definitions:
- **Linux**: systemd unit files
- **macOS**: launchd plists
- **Windows**: Windows Service (SCM) entries

### Command-Line Options

| Option | Description |
|---|---|
| `--frequency` | Interval in milliseconds between command executions |
| `--command` | Shell command to execute |
| `--pid-file` | Path to write the daemon's PID (optional) |
| `--synchronous` | Set to `false` for async execution (default: `true`) |
| `--data-dir` | Override the data directory path |
| `--help` | Display help |

### Platform Setup

- **macOS**: See [docs/macos.md](docs/macos.md)
- **Ubuntu/Debian**: See [docs/ubuntu.md](docs/ubuntu.md)
- **Windows**: See [docs/windows.md](docs/windows.md)


## Documentation

Full documentation is available in [docs/wiki/](docs/wiki/):

- [Getting Started](docs/wiki/Getting-Started.md)
- [CLI Reference](docs/wiki/CLI-Reference.md)
- [Service Management](docs/wiki/Service-Management.md)
- [Architecture](docs/wiki/Architecture.md)
- [Logging](docs/wiki/Logging.md)
- [FAQ](docs/wiki/FAQ.md)
- [Troubleshooting](docs/wiki/Troubleshooting.md)
- [Contributing](docs/wiki/Contributing.md)
