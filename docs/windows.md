# Windows Installation and Setup

## Dependencies

- [Visual Studio](https://visualstudio.microsoft.com/) with C++ desktop development workload (or Build Tools for Visual Studio)
- [vcpkg](https://github.com/microsoft/vcpkg)
- [CMake](https://cmake.org/download/) 3.14+

### Installing dependencies with vcpkg

```powershell
vcpkg install boost-asio:x64-windows boost-program-options:x64-windows sqlite3:x64-windows
```

## Build

```powershell
git clone https://github.com/homer6/frequent-cron.git
cd frequent-cron
cmake . -DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
ctest --output-on-failure -C Release
```

## Running Directly

```powershell
.\frequent-cron.exe run --frequency=1000 --command="C:\path\to\your\script.bat" --pid-file=C:\path\to\frequent-cron.pid
```

## Managing Services

```powershell
# Register a service (also creates a Windows Service via SCM)
# Run as Administrator for SCM access
frequent-cron.exe install myservice --frequency=1000 --command="C:\path\to\script.bat"

# Start/stop/status
frequent-cron.exe start myservice
frequent-cron.exe status
frequent-cron.exe stop myservice

# View logs
frequent-cron.exe logs myservice

# Remove (also removes the Windows Service)
frequent-cron.exe remove myservice
```

## Manual Windows Service Setup

If you prefer to manage the service yourself using Task Scheduler:

1. Open Task Scheduler and create a new task.
2. Under **General**, select "Run whether user is logged on or not".
3. Under **Triggers**, add a trigger for "At startup".
4. Under **Actions**, add an action:
   - Program: `C:\path\to\frequent-cron.exe`
   - Arguments: `run --frequency=1000 --command=C:\path\to\your\script.bat --pid-file=C:\path\to\frequent-cron.pid`
5. Under **Settings**, uncheck "Stop the task if it runs longer than".

## Notes

- On Windows, frequent-cron runs in the foreground when started from the command line. Use the `install` subcommand for background service execution via SCM.
- Async mode (`--synchronous=false`) uses `cmd /c start /b` on Windows instead of `fork()`.
