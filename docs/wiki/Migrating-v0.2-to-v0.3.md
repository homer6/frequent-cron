# Migrating from v0.2.x to v0.3.x

This guide covers upgrading from v0.2.x to v0.3.x, which adds a full service manager on top of the existing daemon.

## What's New

- **Service management CLI**: `install`, `remove`, `start`, `stop`, `status`, `list`, `logs`
- **SQLite service registry** for tracking multiple named services
- **Platform-native service integration**: systemd (Linux), launchd (macOS), rc.d (FreeBSD), SCM (Windows)
- **Boot-start and auto-restart** on all platforms
- **Log capture and rotation** (10MB default, 5 rotated files)
- **FreeBSD support**
- **70 unit tests** (up from 25)

## Breaking Changes

### Build Requirements

| | v0.2.x | v0.3.x |
|---|---|---|
| CMake | 3.5+ | 3.14+ |
| New dependency | -- | SQLite3 |
| Platforms | Linux, macOS, Windows | Linux, macOS, FreeBSD, Windows |

### New Dependency: SQLite3

v0.3.x requires SQLite3 for the service registry:

```bash
# Ubuntu/Debian
sudo apt-get install libsqlite3-dev

# macOS
brew install sqlite

# FreeBSD
pkg install sqlite3

# Windows (vcpkg)
vcpkg install sqlite3:x64-windows
```

### init_script.tpl Moved

The init.d service template moved from the project root to `docs/init_script.tpl`. If your deployment scripts reference the old path, update them.

The `install` subcommand is the recommended replacement for init.d scripts -- it generates platform-native service definitions automatically.

### CI Workflow Split

The single `ci.yml` was replaced with per-platform workflow files:

```
.github/workflows/
  linux.yml
  macos.yml
  freebsd.yml
  windows.yml
```

If you forked and modified `ci.yml`, you'll need to migrate your changes.

## No Behavioral Changes to Legacy Mode

- `frequent-cron --frequency=1000 --command="echo hi" --pid-file=/tmp/fc.pid` works identically.
- The `run` subcommand is functionally equivalent to legacy mode.
- The timer/execution engine is unchanged.
- Async mode (`--synchronous=false`) works as before.

## Migration Steps

### 1. Install SQLite3

See the dependency table above for your platform.

### 2. Build

```bash
git clone https://github.com/homer6/frequent-cron.git
cd frequent-cron
git checkout 0.3
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

### 4. Migrate from init.d to service manager (recommended)

If you're using `init_script.tpl` to manage services, consider migrating to the built-in service manager. It handles PID files, log capture, boot-start, and auto-restart automatically.

**Before (init.d):**
```bash
# /etc/init.d/frequent_service
EXEC=/usr/bin/frequent-cron
PIDFILE=/var/run/frequent_service.pid
COMMAND=/tmp/myshell.sh
FREQUENCY=1000

$EXEC --frequency $FREQUENCY --command $COMMAND --pid-file $PIDFILE
```

**After (service manager):**
```bash
frequent-cron install frequent_service --frequency=1000 --command="/tmp/myshell.sh"
frequent-cron start frequent_service
```

This generates a systemd unit file (or launchd plist / rc.d script / SCM entry) automatically. The OS handles boot-start and restart-on-failure.

To see what you get:
```bash
frequent-cron status              # all services
frequent-cron logs frequent_service   # captured output
```

### 5. Update init_script.tpl path (if keeping init.d)

If you prefer to keep init.d scripts, update any references:

```bash
# Old path
init_script.tpl

# New path
docs/init_script.tpl
```

Legacy mode is fully supported and will continue to work. The service manager is additive -- you can migrate services one at a time.

## Data Directory

The service manager stores its database, logs, and PID files in a platform-specific directory:

| Platform | User | Root |
|---|---|---|
| Linux | `~/.local/share/frequent-cron/` | `/var/lib/frequent-cron/` |
| macOS | `~/Library/Application Support/frequent-cron/` | -- |
| FreeBSD | `~/.local/share/frequent-cron/` | `/var/lib/frequent-cron/` |
| Windows | `%LOCALAPPDATA%\frequent-cron\` | -- |

Override with `--data-dir=<path>` on any command.
