# Getting Started

This guide walks you through installing frequent-cron and running your first sub-second cron job.

## 1. Install

**macOS:**
```bash
brew install boost cmake sqlite
git clone https://github.com/homer6/frequent-cron.git
cd frequent-cron && cmake . && make && sudo make install
```

**Ubuntu/Debian:**
```bash
sudo apt-get install libboost-system-dev libboost-program-options-dev libsqlite3-dev cmake build-essential
git clone https://github.com/homer6/frequent-cron.git
cd frequent-cron && cmake . && make && sudo make install
```

**Windows:** See [Installation](Installation) for vcpkg setup.

## 2. Run a Command

Run a command every 500 milliseconds:

```bash
frequent-cron run --frequency=500 --command="echo tick" --pid-file=/tmp/fc.pid
```

The process daemonizes and runs in the background. To stop it:

```bash
kill $(cat /tmp/fc.pid)
```

## 3. Register a Named Service

For production use, register services by name so they're tracked and easy to manage:

```bash
# Register
frequent-cron install my-poller --frequency=1000 --command="/opt/scripts/poll.sh"

# Start
frequent-cron start my-poller

# Check status
frequent-cron status

# View output
frequent-cron logs my-poller

# Stop
frequent-cron stop my-poller
```

The `install` command also creates a platform-native service definition (systemd unit, launchd plist, rc.d script, or Windows Service) so the OS can supervise and auto-restart your service.

## 4. Understand Execution Modes

**Synchronous (default):** The command must complete before the next execution starts. If your command takes 3 seconds and the frequency is 500ms, it will effectively run once every 3 seconds.

```bash
frequent-cron run --frequency=500 --command="/slow/script.sh"
```

**Asynchronous:** The timer keeps firing regardless of whether previous executions have finished. Commands can overlap.

```bash
frequent-cron run --frequency=500 --command="/fast/script.sh" --synchronous=false
```

Use async mode with caution -- if your script never exits or is called too frequently, you may exhaust system resources (file descriptors, database connections, memory).

## Next Steps

- [CLI Reference](CLI-Reference) -- full list of commands and options
- [Service Management](Service-Management) -- managing multiple services
- [Logging](Logging) -- log capture and rotation
- [Platform Guides](Platform-Guides) -- systemd, launchd, rc.d, Windows Service setup
