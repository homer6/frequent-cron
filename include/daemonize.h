#pragma once

// Daemonizes the process on POSIX systems.
// On Windows, this is a no-op (run as foreground process).
// Returns true on success, false on failure.
bool daemonize();
