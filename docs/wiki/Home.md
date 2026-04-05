# frequent-cron

A lightweight daemon and service manager for running shell commands at millisecond intervals. Sub-second cron for Linux, macOS, and Windows.

## Quick Links

- [Getting Started](Getting-Started)
- [Installation](Installation)
- [CLI Reference](CLI-Reference)
- [Service Management](Service-Management)
- [Configuration](Configuration)
- [Architecture](Architecture)
- [Logging](Logging)
- [Platform Guides](Platform-Guides)
- [FAQ](FAQ)
- [Troubleshooting](Troubleshooting)
- [Contributing](Contributing)
- [Release Notes](Release-Notes)

## What is frequent-cron?

Standard Unix cron only supports minute-level scheduling. frequent-cron fills the gap by running commands at millisecond precision -- ideal for:

- Polling queues or APIs multiple times per second
- Health checks at sub-second intervals
- High-frequency data collection
- Any task that needs to run more often than once per minute

frequent-cron has been production-stable for 15+ years. Starting with v0.3.0, it also works as a **service manager** -- register named services, start/stop them, view logs, and let the OS supervise them natively via systemd, launchd, or the Windows Service Control Manager.
