# Contributing

## Development Setup

```bash
git clone https://github.com/homer6/frequent-cron.git
cd frequent-cron
cmake .
make -j$(nproc)
make test ARGS="--output-on-failure"
```

GTest is fetched automatically via CMake FetchContent.

## Branches and Releases

Release branches hold the latest patch for each minor version:

| Branch | Version Line | Description |
|---|---|---|
| [`0.1`](https://github.com/homer6/frequent-cron/tree/0.1) | v0.1.x | Original Linux daemon |
| [`0.2`](https://github.com/homer6/frequent-cron/tree/0.2) | v0.2.x | Cross-platform, modern C++ |
| [`0.3`](https://github.com/homer6/frequent-cron/tree/0.3) | v0.3.x | Service manager |
| `main` | latest stable | Mirrors the latest release branch after a release is cut |

**Workflow:**
1. Create a feature/fix branch off the appropriate release branch (e.g. `fix/my-bug` off `0.3`).
2. Open a PR targeting the release branch.
3. After merge, tag the release (e.g. `v0.3.1`) on the release branch.
4. Merge the release branch into `main`.

For fixes that apply to multiple versions, create separate branches and PRs for each release branch.

## Project Structure

```
include/          # Public headers
src/              # Implementation + main.cc
tests/            # GTest unit tests + integration test scripts
docs/             # Platform guides, release notes, wiki content
docs/wiki/        # Wiki pages (synced to GitHub Wiki)
docs/releases/    # Release notes
.github/workflows # CI configuration
```

## Running Tests

```bash
# All tests (GTest + integration)
make test ARGS="--output-on-failure"

# GTest only (faster, skips integration)
make test ARGS="--output-on-failure -E integration"

# Specific test suite
make test ARGS="--output-on-failure -R Config"
make test ARGS="--output-on-failure -R Database"
```

## Adding a New Module

1. Create header in `include/mymodule.h`
2. Create implementation in `src/mymodule.cc`
3. Add `mymodule.cc` to `src/CMakeLists.txt` in the library sources
4. Create tests in `tests/test_mymodule.cc`
5. Add test target in `tests/CMakeLists.txt`:
   ```cmake
   add_executable( test_mymodule test_mymodule.cc )
   target_link_libraries( test_mymodule ${CMAKE_PROJECT_NAME}_lib GTest::gtest_main )
   gtest_discover_tests( test_mymodule )
   ```
6. Add test binary to `.gitignore`

## Platform Notes

- Use `#ifdef _WIN32` / `#elif defined(__APPLE__)` / `#elif defined(__FreeBSD__)` / `#else` for platform-specific code
- Test on all three platforms via CI before submitting a PR
- Windows requires `WIN32_LEAN_AND_MEAN` before `<windows.h>` and `NOMINMAX` to avoid macro conflicts
- macOS uses `_NSGetExecutablePath()` for binary path detection; Linux uses `/proc/self/exe`

## Code Style

- 4-space indentation
- Spaces inside parentheses: `if( condition )`
- Braces on same line: `}else{`
- `#pragma once` for header guards
- No `using namespace std` -- qualify with `std::`
