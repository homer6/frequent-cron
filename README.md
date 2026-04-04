# frequent-cron

A lightweight Linux daemon that executes shell commands at sub-second intervals. Standard cron only supports minute-level granularity -- frequent-cron supports millisecond precision.

Command execution is **blocking**: if a command takes longer than the configured interval, the next execution waits until the previous one completes. For example, a 500ms interval with a command that takes 3 minutes will effectively run once every 3 minutes.

Licensed under the MIT License.


## Dependencies

- [Boost](https://www.boost.org/) 1.37+ (`sudo apt-get install libboost-all-dev`)
- [CMake](https://cmake.org/) 2.8.2+ (`sudo apt-get install cmake`)


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

### Running as an init.d Service

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
