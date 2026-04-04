# Windows Installation and Setup

## Dependencies

- [Visual Studio](https://visualstudio.microsoft.com/) with C++ desktop development workload (or Build Tools for Visual Studio)
- [Boost](https://www.boost.org/users/download/) 1.37+
- [CMake](https://cmake.org/download/) 3.5+

### Installing Boost with vcpkg

```powershell
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg install boost-asio boost-program-options
```

## Build

```powershell
git clone https://github.com/homer6/frequent-cron.git
cd frequent-cron
cmake . -DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

## Running Directly

```powershell
.\frequent-cron.exe --frequency=1000 --command="C:\path\to\your\script.bat"
```

## Running as a Windows Service

You can use Windows Task Scheduler to run frequent-cron at startup:

1. Open Task Scheduler and create a new task.
2. Under **General**, select "Run whether user is logged on or not".
3. Under **Triggers**, add a trigger for "At startup".
4. Under **Actions**, add an action:
   - Program: `C:\path\to\frequent-cron.exe`
   - Arguments: `--frequency=1000 --command=C:\path\to\your\script.bat`
5. Under **Settings**, uncheck "Stop the task if it runs longer than".

Alternatively, use `sc.exe` to register a native Windows service if you wrap frequent-cron with a service host.

## Notes

- The `daemon()` call used on Linux/macOS is not available on Windows. Use Task Scheduler or a service wrapper instead.
- The `fork()` call used for async mode is not available on Windows. Async mode (`--synchronous=false`) is not currently supported on Windows.
