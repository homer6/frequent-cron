# Windows Installation and Setup

## Dependencies

- [Visual Studio](https://visualstudio.microsoft.com/) with C++ desktop development workload (or Build Tools for Visual Studio)
- [vcpkg](https://github.com/microsoft/vcpkg)
- [CMake](https://cmake.org/download/) 3.14+

### Installing Boost with vcpkg

```powershell
vcpkg install boost-asio:x64-windows boost-program-options:x64-windows
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
.\frequent-cron.exe --frequency=1000 --command="C:\path\to\your\script.bat" --pid-file=C:\path\to\frequent-cron.pid
```

## Running as a Windows Service

You can use Windows Task Scheduler to run frequent-cron at startup:

1. Open Task Scheduler and create a new task.
2. Under **General**, select "Run whether user is logged on or not".
3. Under **Triggers**, add a trigger for "At startup".
4. Under **Actions**, add an action:
   - Program: `C:\path\to\frequent-cron.exe`
   - Arguments: `--frequency=1000 --command=C:\path\to\your\script.bat --pid-file=C:\path\to\frequent-cron.pid`
5. Under **Settings**, uncheck "Stop the task if it runs longer than".

Alternatively, use `sc.exe` to register a native Windows service if you wrap frequent-cron with a service host.

## Notes

- On Windows, frequent-cron runs in the foreground (no daemonization). Use Task Scheduler or a service wrapper for background execution.
- Async mode (`--synchronous=false`) uses `cmd /c start /b` on Windows instead of `fork()`.
