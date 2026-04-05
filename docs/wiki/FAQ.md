# FAQ

## General

### Why not just use cron with `sleep` in a loop?

A bash loop with `sleep` has several problems:
- Drift: the loop timing isn't precise (command execution time adds up)
- No PID management: hard to stop cleanly
- No service integration: no systemd/launchd supervision
- Resource leaks: bash loops are fragile over long periods

frequent-cron uses Boost ASIO's steady timer for precise, drift-free scheduling and integrates with OS service managers for production reliability.

### What's the minimum frequency I can use?

Technically, 1 millisecond (`--frequency=1`). In practice, your command's execution time and system overhead will be the bottleneck. For most use cases, 100-1000ms is a sweet spot.

### Does frequent-cron accumulate zombie processes?

No. In asynchronous mode, the parent process calls `waitpid()` with `WNOHANG` after each fork to reap completed child processes. This is tested in the unit test suite.

## Execution

### What happens if my command takes longer than the frequency?

**Synchronous mode (default):** The next execution waits. A 500ms frequency with a 3-second command effectively runs once every 3 seconds.

**Asynchronous mode:** The timer keeps firing. Multiple instances of your command can run simultaneously. Be careful with resource consumption.

### Can I run multiple commands?

Use shell syntax in the command string:
```bash
frequent-cron run --frequency=1000 --command="cmd1 && cmd2"
```

Or register multiple named services:
```bash
frequent-cron install svc1 --frequency=500 --command="cmd1"
frequent-cron install svc2 --frequency=1000 --command="cmd2"
```

### Does the command run in a shell?

Yes. Commands are executed via the system shell (`/bin/sh -c` on POSIX, `cmd.exe` on Windows), so shell features like pipes, redirects, and `&&` work.

## Service Management

### Where is the database stored?

In the platform data directory:
- Linux: `~/.local/share/frequent-cron/frequent-cron.db`
- macOS: `~/Library/Application Support/frequent-cron/frequent-cron.db`
- Windows: `%LOCALAPPDATA%\frequent-cron\frequent-cron.db`

### Can I move the database to a shared location?

Yes. Use `--data-dir` on every command to point to a shared path:
```bash
frequent-cron install svc --frequency=1000 --command="echo hi" --data-dir=/shared/fc
frequent-cron start svc --data-dir=/shared/fc
```

### What happens if the process dies unexpectedly?

The `status` command checks whether the tracked PID is actually alive. If the process died, it updates the database to `stopped`. If you installed with a platform service (systemd/launchd), the OS will auto-restart it based on the service configuration.

## Platform

### Does it work on ARM?

Yes. frequent-cron has no architecture-specific code. It builds and runs on ARM (Apple Silicon, Raspberry Pi, etc.) as long as the dependencies are available.

### Why does the Windows build install so many Boost packages?

vcpkg resolves all transitive dependencies individually. `boost-asio` and `boost-program-options` have ~55 header-only dependencies. The CI uses vcpkg binary caching so subsequent builds are fast.
