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

You can use [NSSM](https://nssm.cc/) (Non-Sucking Service Manager) to run frequent-cron as a Windows service:

1. Download and install NSSM.

2. Install the service:
   ```powershell
   nssm install frequent-cron "C:\path\to\frequent-cron.exe" "--frequency=1000 --command=C:\path\to\your\script.bat"
   ```

3. Start the service:
   ```powershell
   nssm start frequent-cron
   ```

4. To stop:
   ```powershell
   nssm stop frequent-cron
   ```

## Notes

- The `daemon()` call used on Linux/macOS is not available on Windows. The daemon functionality is handled by running as a Windows service via NSSM.
- The `fork()` call used for async mode is not available on Windows. Async mode (`--synchronous=false`) is not currently supported on Windows.
