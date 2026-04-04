# frequent-cron

A Linux daemon that runs shell commands at millisecond intervals (sub-second cron). Production-stable for 15+ years.

## Build

```bash
cmake .
make
```

Produces the `frequent-cron` executable in the project root.

## Dependencies

- Boost 1.37+ (components: `system`, `program_options`)
- CMake 2.8+
- Linux (uses POSIX `daemon()`)

## Architecture

Single-file C++ daemon (`src/frequent.cc`, ~168 lines). The design is intentionally simple:

- **Boost ASIO event loop** with a `deadline_timer` for millisecond-precision scheduling
- **Blocking execution**: commands run via `system()` -- the next invocation waits for the previous one to finish
- **Daemonization** via POSIX `daemon(0,0)` -- detaches from terminal after argument validation
- **PID file** for service lifecycle management (optional `--pid-file` flag)

Flow: `main()` -> parse args -> create timer -> daemonize -> `register_callback()` -> `io_service->run()` (event loop). `timer_callback()` runs the command, then calls `register_callback()` to schedule the next execution.

## Key Files

- `src/frequent.cc` -- entire daemon implementation
- `src/CMakeLists.txt` -- build rules, Boost linking
- `init_script.tpl` -- template for `/etc/init.d` service script

## Usage

```bash
./frequent-cron --frequency=1000 --command="/path/to/script.sh" --pid-file=/var/run/myservice.pid
```
