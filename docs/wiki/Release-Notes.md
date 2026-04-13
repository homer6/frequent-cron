# Release Notes

## [v0.3.2](../releases/v0.3.2.md) - High CPU Fix + Jitter & Probabilistic Firing (April 2026)

Fix 100% CPU in daemon mode (kqueue fd not inherited across fork). Fix service logs showing no output. Add `--jitter` and `--fire-probability` flags for timing variance and probabilistic tick skipping. Add Claude Code skill for AI-assisted operation.

## [v0.3.1](../releases/v0.3.1.md) - Background Command Fix + Migration Guides (April 2026)

Fix backgrounded commands (`&`) silently failing in legacy mode. Add migration guides for v0.1→v0.2 and v0.2→v0.3.

## [v0.3.0](../releases/v0.3.0.md) - Service Manager (April 2026)

Transforms frequent-cron into a service manager. Register, start, stop, and monitor multiple named services with SQLite, platform-native service integration, and log rotation.

## [v0.2.1](../releases/v0.2.1.md) - Background Command Fix (April 2026)

Fix backgrounded commands (`&`) silently failing in legacy mode.

## [v0.2.0](../releases/v0.2.0.md) - Modernize and Expand Platform Support (April 2026)

Cross-platform support (macOS, Windows), modern C++23, GTest unit tests, GitHub Actions CI.

## [v0.1.0](../releases/v0.1.0.md) - Legacy Build (October 2011)

Original release. Single-file Linux daemon with millisecond-precision scheduling.
