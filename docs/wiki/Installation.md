# Installation

## Dependencies

| Dependency | Minimum Version | Purpose |
|---|---|---|
| Boost | 1.37+ | asio (timer), program_options (CLI) |
| SQLite3 | 3.x | Service registry database |
| CMake | 3.14+ | Build system |
| C++ compiler | C++23 | GCC 13+, Clang 16+, MSVC 19.35+ |

## macOS (Homebrew)

```bash
brew install boost cmake sqlite
```

```bash
git clone https://github.com/homer6/frequent-cron.git
cd frequent-cron
cmake .
make
make test
sudo make install
```

## Ubuntu / Debian

```bash
sudo apt-get update
sudo apt-get install libboost-system-dev libboost-program-options-dev libsqlite3-dev cmake build-essential
```

```bash
git clone https://github.com/homer6/frequent-cron.git
cd frequent-cron
cmake .
make
make test
sudo make install
```

## Windows (vcpkg + Visual Studio)

Prerequisites:
- Visual Studio with C++ desktop development workload
- [vcpkg](https://github.com/microsoft/vcpkg)
- CMake 3.14+

```powershell
vcpkg install boost-asio:x64-windows boost-program-options:x64-windows sqlite3:x64-windows
```

```powershell
git clone https://github.com/homer6/frequent-cron.git
cd frequent-cron
cmake . -DCMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build . --config Release
ctest --output-on-failure -C Release
```

## Verifying the Installation

```bash
frequent-cron --help
```

Should display the usage message with available commands.

## Building from Source (Development)

GTest is fetched automatically via CMake FetchContent during the build. No manual installation needed.

```bash
cmake .
make -j$(nproc)    # or -j$(sysctl -n hw.ncpu) on macOS
make test ARGS="--output-on-failure"
```
