# frequent-cron

A lightweight daemon that executes shell commands at sub-second intervals. Standard cron only supports minute-level granularity -- frequent-cron supports millisecond precision. Runs on Linux and macOS.

By default, command execution is **blocking** (synchronous): if a command takes longer than the configured interval, the next execution waits until the previous one completes. For example, a 500ms interval with a command that takes 3 minutes will effectively run once every 3 minutes.

You can set `--synchronous=false` to run commands asynchronously. Warning: running asynchronously can cause unbounded resource growth if the script never exits or is called too frequently (e.g. consuming too many database connections or too much memory). Synchronous (the default) is the safer option.

Licensed under the MIT License.


## Dependencies

- [Boost](https://www.boost.org/) 1.37+
- [CMake](https://cmake.org/) 3.5+

**Linux (Debian/Ubuntu):**
```bash
sudo apt-get install libboost-system-dev libboost-program-options-dev cmake
```

**macOS (Homebrew):**
```bash
brew install boost cmake
```


## Installation

```bash
git clone https://github.com/homer6/frequent-cron.git
cd frequent-cron
cmake .
make
```


## Usage

### Command-Line Options

| Option | Description |
|---|---|
| `--frequency` | Interval in milliseconds between command executions |
| `--command` | Shell command to execute |
| `--pid-file` | Path to write the daemon's PID (optional) |
| `--synchronous` | Set to `false` for async execution (default: `true`) |
| `--help` | Display help |

### Running Directly

```bash
./frequent-cron --frequency=1000 --command="/path/to/your/script.sh"
```

To stop, find and kill the process:

```bash
ps aux | grep frequent-cron
kill <pid>
```

### Running as a Service

- **macOS**: See [docs/macos.md](docs/macos.md) for launchd setup.
- **Ubuntu/Debian**: See [docs/ubuntu.md](docs/ubuntu.md) for init.d and systemd setup.
- **Windows**: See [docs/windows.md](docs/windows.md) for NSSM service setup.
- **Linux (init.d)**:

1. Copy the template:
   ```bash
   sudo cp init_script.tpl /etc/init.d/frequent_service
   ```

2. Edit `/etc/init.d/frequent_service` and set `COMMAND`, `FREQUENCY`, and `PIDFILE` (use absolute paths).

3. Enable and start:
   ```bash
   sudo chmod +x /etc/init.d/frequent_service
   sudo /etc/init.d/frequent_service start
   ```

4. Optionally, enable auto-start on boot:
   ```bash
   sudo update-rc.d frequent_service defaults
   ```

5. To stop:
   ```bash
   sudo /etc/init.d/frequent_service stop
   ```
